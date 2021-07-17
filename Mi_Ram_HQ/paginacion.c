#include "paginacion.h"
#define ESQUEMA_MEMORIA config_get_string_value(config, "ESQUEMA_MEMORIA")
#define TAMANIO_PAGINA config_get_int_value(config, "TAMANIO_PAGINA")
#define ALGORITMO_REEMPLAZO config_get_string_value(config, "ALGORITMO_REEMPLAZO")
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

int sobreescribir_paginas(tabla_paginas* tabla, void* data, int dl, int tam, int pid){
	pagina* pagina;
	int progreso = 0;
	int numero_pagina = dl / TAMANIO_PAGINA;
	int offset = dl % TAMANIO_PAGINA;
	if(offset > 0){
		pagina = list_get(tabla->paginas, numero_pagina);
		if(!pagina->en_memoria){
			page_fault(pagina, pid, numero_pagina + 1);
		}else{
			pagina->ultimo_uso = unix_epoch();
			pagina->usado = true;
		}
		pagina->modificada = true;
		progreso += escribir_en_marco(pagina->puntero_marco, data + progreso, offset, tam - progreso);
		numero_pagina++;
		//log_info(logger, "Progreso: %d / %d bytes", progreso, tam);
	}
	while(progreso < tam){
		pagina = list_get(tabla->paginas, numero_pagina);
		if(!pagina->en_memoria){
			page_fault(pagina, pid, numero_pagina + 1);
		}else{
			pagina->ultimo_uso = unix_epoch();
			pagina->usado = true;
		}
		pagina->modificada = true;
		progreso += escribir_en_marco(pagina->puntero_marco, data + progreso, 0, tam - progreso);
		numero_pagina++;
		//log_info(logger, "Progreso: %d / %d bytes", progreso, tam);
	}
}

int escribir_en_marco(marco* marco, void* data, int offset, int tam){
	// retorna lo que se logró escribir
	if(tam + offset <= TAMANIO_PAGINA){
		memcpy(memoria_principal + marco->base + offset, data, tam);
		return tam;
	}else{
		memcpy(memoria_principal + marco->base + offset, data, TAMANIO_PAGINA - offset);
		return TAMANIO_PAGINA - offset;
	}
	memcpy(memoria_principal + marco->base + offset, data, TAMANIO_PAGINA);
	return TAMANIO_PAGINA;
}

void* rescatar_de_paginas(tabla_paginas* tabla, int dl, int tam, int pid){
	void* data = malloc(tam);
	pagina* pagina; 
	int faltante = tam;
	int numero_pagina = dl / TAMANIO_PAGINA;
	int offset = dl % TAMANIO_PAGINA;

	if(offset > 0){
		pagina = list_get(tabla->paginas, numero_pagina);
		if(!pagina->en_memoria){
			page_fault(pagina, pid, numero_pagina + 1);
		}else{
			pagina->ultimo_uso =  unix_epoch();
			pagina->usado = true;
		}
		faltante -= rescatar_de_marco(pagina->puntero_marco, data + tam - faltante, offset, faltante);
		numero_pagina++;
		//log_info(logger, "Progreso: %d / %d bytes", tam-faltante, tam);
	}

	while(faltante > 0){
		pagina = list_get(tabla->paginas, numero_pagina);
		if(!pagina->en_memoria){
			page_fault(pagina, pid, numero_pagina + 1);
		}else{
			pagina->ultimo_uso =  unix_epoch();
			pagina->usado = true;
		}
		faltante -= rescatar_de_marco(pagina->puntero_marco, data + tam - faltante, 0, faltante);
		numero_pagina++;
		//log_info(logger, "Progreso: %d / %d bytes", tam-faltante, tam);
	}
	return data;
}

int rescatar_de_marco(marco* marco, void* data, int offset, int tam){
	//esta funcion retorna lo que de rescató de memoria
	int tam_rescate = MIN(tam, TAMANIO_PAGINA - offset);
	//log_info(logger, "Se rescatan %d bytes del marco con base %d, con offset %d, tam %d", tam_rescate, marco->base,offset,tam);
	memcpy(data, memoria_principal + marco->base + offset, tam_rescate);

	return tam_rescate;
}

int completar_pagina(pagina* pagina, void* data, int tam){
	//esta función devuelve lo que logró guardar
	int ocupado = pagina->tamano_ocupado;
	int disponible = TAMANIO_PAGINA - ocupado;
	marco* marco = pagina->puntero_marco;

	// me alcanza con esa pagina?
	if(disponible >= tam){
		pagina->tamano_ocupado += tam;
		memcpy(memoria_principal + marco->base + ocupado, data, tam);
		return tam; // pude guardar todo
	}

	pagina->tamano_ocupado = TAMANIO_PAGINA;
	memcpy(memoria_principal + marco->base + ocupado, data, disponible);
	return disponible; // pude guardar solo el tamaño disponible 
}

int agregar_paginas_segun_tamano(tabla_paginas* tabla, void* data, int tam, int pid){
	// esta función devuelve la dirección lógica de lo que se guardó
	int dl;
	int progreso = 0;
	// para calcular la dl
	int numero_pagina;
	int offset;

	pagina* ultima_pagina = pagina_incompleta(tabla);
	// TODO: PAGINA UPDATE LRU/CLOCK
	if(ultima_pagina != NULL){ // si hay una pagina incompleta
		if(!ultima_pagina->en_memoria){
			page_fault(ultima_pagina, pid, list_size(tabla->paginas));
		}
		offset = ultima_pagina->tamano_ocupado;
		numero_pagina = list_size(tabla->paginas) - 1;
		ultima_pagina->ultimo_uso =  unix_epoch();
		ultima_pagina->usado = true;
		// TODO: PAGINA UPDATE LRU/CLOCK
		progreso += completar_pagina(ultima_pagina, data, tam);
		//log_info(logger, "Progreso: %d / %d bytes", progreso, tam);
	}else{
		numero_pagina = list_size(tabla->paginas);
		offset = 0;
	}
	dl = numero_pagina * TAMANIO_PAGINA + offset;

	while(progreso < tam){
		int temp = agregar_pagina(tabla, data + progreso, tam - progreso, pid); 
		if(temp > 0){
			progreso += temp; 
			//log_info(logger, "Progreso: %d / %d bytes", progreso, tam);
		}else{
			// no hubo mas progreso, por ende no hay mas memoria
			return NULL;
		}
	}

	return dl;	
}

int agregar_pagina(tabla_paginas* tabla, void* data, int tam, int pid){
	// retorna lo que se logró guardar
	marco* marco = asignar_marco();
	if(marco == NULL){
		// no hay mas memoria
		return 0;
	}
	int num_pagina = list_size(tabla->paginas) + 1;
    marco->libre = false;
	marco->num_pagina = num_pagina;
	marco->pid = pid;
	
	pagina* pag = malloc(sizeof(pagina));
	pag->puntero_marco = marco;
	pag->disk_index = -1; // todavia no está en disco
	pag->modificada = true;
	pag->ultimo_uso =  unix_epoch();
	pag->usado = true;
	pag->en_memoria = true;

	list_add(tabla->paginas,pag);
	if(tam <= TAMANIO_PAGINA){
		memcpy(memoria_principal + marco->base, data, tam);
		pag->tamano_ocupado = tam;
		return tam;
	}else{
		memcpy(memoria_principal + marco->base, data, TAMANIO_PAGINA);
		pag->tamano_ocupado = TAMANIO_PAGINA;
		return TAMANIO_PAGINA;
	}
}


pagina* pagina_incompleta(tabla_paginas* tabla){
	int size = list_size(tabla->paginas);
	if(size > 0){
		pagina* ultima_pagina = list_get(tabla->paginas, size - 1);
		if(ultima_pagina->tamano_ocupado < TAMANIO_PAGINA)
			return ultima_pagina;
	}
	return NULL;
}

marco* buscar_marco_libre(){
    log_info(logger,"Buscando un marco libre");
    int size = list_size(marcos);
	for(int i=0; i<size; i++){
        marco* x = list_get(marcos, i);
        if(x->libre == true ){
            //log_info(logger, "Marco libre encontrado (base: %d)", x->base);
            return x;
        }
    }
    log_warning(logger, "No se encontró marco libre");
    return NULL;
}

marco* asignar_marco(){

	marco* marco_libre = buscar_marco_libre();
	if(marco_libre != NULL){
		marco_libre->libre = false;
		log_info(logger,"Marco asignado (base:%d)", marco_libre->base);
		intento_asignar_marco = 0;
		return marco_libre;
	}

	if(intento_asignar_marco == 1){
		intento_asignar_marco = 0;
		return NULL;
	}
	intento_asignar_marco = 1;
	int result = algoritmo_de_reemplazo();
	if(!result){
		// no hay mas memoria virtual tampoco
		return NULL;
	}
	return asignar_marco();
}

int algoritmo_de_reemplazo(){
	//devuelve 1 si fue exito
	pagina* pagina;
	if(strcmp(ALGORITMO_REEMPLAZO,"LRU")==0){
		pagina = get_lru();
	}else if(strcmp(ALGORITMO_REEMPLAZO,"CLOCK")==0){
		pagina = get_clock();
	}else{
		log_error(logger,"Algoritmo de reemplazo desconocido");
		exit(EXIT_FAILURE);
	}
	if(pagina->modificada){
		int result = swap_in(pagina);
		if(!result){
			return 0;
		}
	}
	pagina->en_memoria = false;
	pagina->puntero_marco->libre = true;
	pagina->puntero_marco = NULL;
	return 1;
}

int swap_in(pagina* pag){
	log_debug(logger, "Swap in pid: %d, pag: %d", pag->puntero_marco->pid,pag->puntero_marco->num_pagina);
	if(pag->disk_index == -1){
		//la pagina no está en disco, buscar espacio libre
		log_info(logger, "Pag nueva, le busco espacio en disco");
		int espacio_libre = -1;
		for(int i = 0; i < marcos_disco_size; i++){
			if(!bitmap_disco[i]){
				espacio_libre = i;
				bitmap_disco[i] = true;
				break;
			}
		}
		if(espacio_libre >= 0){
			pag->disk_index = espacio_libre;
		}else{
			// TODO RETURN FALLO
			log_error(logger, "No hay mas memoria virtual");
			return 0;
		}
	}
	// escribo la página en disco, accediendo directamente con su indice porque las busquedas secuenciales son para putos que no tienen memoria para tantos punteros
	log_info(logger,"Pag modificada, mando a disco");
	fseek(disco, TAMANIO_PAGINA * pag->disk_index, SEEK_SET);
	void* data = memoria_principal + pag->puntero_marco->base;
	fwrite(data, TAMANIO_PAGINA, 1, disco);
	return 1;
}

void swap_out(pagina* pagina){
	log_info(logger, "Swap out");
	pagina->puntero_marco = asignar_marco(); // asignar marco me pone el marco en libre->false

	fseek(disco, pagina->disk_index * TAMANIO_PAGINA, SEEK_SET);
	fread(memoria_principal + pagina->puntero_marco->base, TAMANIO_PAGINA, 1, disco);
}

void page_fault(pagina* pag, int pid, int num){
	pthread_mutex_lock(&m_swap);
	log_error(logger,"PF pid: %d, pag: %d", pid, num);
	swap_out(pag);
	pag->puntero_marco->pid = pid;
	pag->puntero_marco->num_pagina = num;
	pag->en_memoria = true;
	pag->modificada = false;
	pag->ultimo_uso =  unix_epoch();
	pag->usado = true;
	pthread_mutex_unlock(&m_swap);
}

int liberar_pagina(pagina* pagina, int offset, int faltante){
	// retorna lo que logró liberar
	if(offset + faltante <= TAMANIO_PAGINA){
		//log_info(logger, "Se resta el tamaño: %d", faltante);
		pagina->tamano_ocupado -= faltante;
		//log_info(logger, "La pagina quedó con: %d bytes", pagina->tamano_ocupado);
		return faltante; // pude liberar todo el tamaño
	}
	if(pagina->tamano_ocupado + offset <= TAMANIO_PAGINA){
		pagina->tamano_ocupado = 0;
		return TAMANIO_PAGINA - offset;
	}
	pagina->tamano_ocupado = offset;
	return TAMANIO_PAGINA - offset; // pude liberar hasta el fin de la página 
}

void liberar_paginas(tabla_paginas* tabla, int dl, int tam){
	int faltante = tam;
	int numero_pagina = dl / TAMANIO_PAGINA;
	int offset = dl % TAMANIO_PAGINA;
	pagina* pag;
	t_list* paginas_a_remover = list_create();
	if(offset > 0){
		//log_info(logger, "Se empieza a liberar en la pagina %d con offset %d", numero_pagina, offset);
		pag = list_get(tabla->paginas, numero_pagina);
		faltante -= liberar_pagina(pag, offset, faltante);
		if(pag->tamano_ocupado <= 0){
			list_add(paginas_a_remover, (void*) numero_pagina);
		}
		//log_info(logger, "Progreso: %d / %d bytes", tam-faltante, tam);
		numero_pagina++;
	}
	while(faltante > 0){
		//log_info(logger, "Se empieza a liberar en la pagina %d con offset %d", numero_pagina, 0);
		pag = list_get(tabla->paginas, numero_pagina);
		faltante -= liberar_pagina(pag, 0, faltante);
		if(pag->tamano_ocupado <= 0){
			list_add(paginas_a_remover, (void*) numero_pagina);
		}
		//log_info(logger, "Progreso: %d / %d bytes", tam-faltante, tam);
		numero_pagina++;
	}

	bool page_orderer(void* un_int, void* otro_int){
		int num = (int) un_int;
		int num2 = (int) otro_int;
		return num > num2;
	}
	list_sort(paginas_a_remover, page_orderer);

	void page_remover(void* un_int){
		int num = (int) un_int;
		//log_info(logger, "borrando pag: %d", num);
		pagina* pag2 = list_get(tabla->paginas, num);
		if(pag2->en_memoria){
			pag2->puntero_marco->libre = true;
		}
		bitmap_disco[pag2->disk_index] = false;
		list_remove(tabla->paginas, num);
		free(pag2);
	}

	list_iterate(paginas_a_remover, page_remover);
	
}

int matar_paginas_tcb(tabla_paginas* tabla, int tid){
	log_info(logger, "Eliminando TCB tid: %d", tid);
	char stid[8];
	sprintf(stid, "%d", tid);
	int dl_tcb = (int) dictionary_get(tabla->dl_tcbs, stid);
	if(dl_tcb == (int) NULL){
		return 0;
	}
	liberar_paginas(tabla, dl_tcb, sizeof(t_TCB));
	dictionary_remove(tabla->dl_tcbs, stid);
	log_info(logger, "Se termino de matar las paginas del TCB tid: %d", tid);
	return 1;
}

void matar_tabla_paginas(int pid){
	log_debug(logger, "Matando patota PID: %d", pid);
    
    void table_destroyer(void* una_tabla){
        tabla_paginas* tabla = (tabla_paginas*) una_tabla;

		void page_destroyer(void* una_pagina){
			pagina* pag = (pagina*) una_pagina;
			if(pag->en_memoria){
				pag->puntero_marco->libre = true;
			}
			bitmap_disco[pag->disk_index] = false;
			free(pag);
		}

		list_destroy_and_destroy_elements(tabla->paginas, page_destroyer);

		dictionary_destroy(tabla->dl_tcbs);

		free(tabla);
    }

    char spid[4];
    sprintf(spid, "%d", pid);
    dictionary_remove_and_destroy(tablas,spid,table_destroyer);
	//log_debug(logger, "Se completó la nismación de la tabla PID: %d", pid);
}

pagina* get_lru(){
	log_info(logger, "Ejecuto un swap por LRU");

	uint64_t lru_ts = unix_epoch();
    pagina* lru_p;
    
	void table_iterator(char* spid,void* una_tabla){
		tabla_paginas* tabla = (tabla_paginas*) una_tabla;

		void page_iterator(void* una_pagina){
			pagina* pag = (pagina*) una_pagina;
			if(pag->en_memoria && pag->ultimo_uso <= lru_ts){
				lru_ts = pag->ultimo_uso;
				lru_p = pag;
			}
		}

		list_iterate(tabla->paginas, page_iterator);
	}

	dictionary_iterator(tablas, table_iterator);
    return lru_p;
}

pagina* get_clock(){
	log_info(logger, "Ejecuto un swap por CLOCK");
    pagina* clock_p;
	int cantidad_marcos = list_size(marcos);
	marco* marco_actual;
	while(1){
		marco_actual = list_get(marcos, marco_clock);
		pagina* pag = get_pagina_from_marco(marco_actual);
		if(marco_clock < cantidad_marcos-1){
			marco_clock++;
		}else{
			marco_clock = 0;
		}
		//log_info(logger, "Puntero clock: %d", marco_clock);
		if(!pag->usado){
			clock_p = pag;
			break;
		}else{
			pag->usado = false;
		}
	}
    return clock_p;
}

pagina* get_pagina_from_marco(marco* marco){
	int pid = marco->pid;
	int num_pagina = marco->num_pagina;

	tabla_paginas* tabla = buscar_tabla(pid);
	return list_get(tabla->paginas, num_pagina-1);
}

void dump_paginacion(){
	char* path = string_from_format("./dump/Dump_%d.dmp",  time(NULL));
    FILE* file = fopen(path,"w");
    free(path);
	int num_marco = 0;
    void impresor_dump(void* un_marco){
		char* dump_row;
        marco* marquito = (marco*) un_marco;
		if(marquito->libre){
			dump_row = string_from_format("Marco: %d\tEstado: Libre\tProceso: -\tPagina: -\n", num_marco);
		}else{
        	dump_row = string_from_format("Marco: %d\tEstado: Ocupado\tProceso: %d\tPagina: %d\n", num_marco, marquito->pid, marquito->num_pagina);
		}
        txt_write_in_file(file, dump_row);
        free(dump_row);
		num_marco++;
    }

    txt_write_in_file(file, string_from_format("Dump: %s\n", temporal_get_string_time("%d/%m/%y %H:%M:%S")));
    list_iterate(marcos, impresor_dump);
    txt_close_file(file);
}

marco* crear_marco(int base, bool libre){
    marco* nuevo_marco = malloc(sizeof(marco));
    nuevo_marco->base = base;
    nuevo_marco->libre = libre;

    return nuevo_marco;
}

void liberar_lista_tcbs_paginacion(t_list* lista){
	if(strcmp(ESQUEMA_MEMORIA, "PAGINACION") == 0){
		void free_tcb(void* un_tcb){
			free(un_tcb);
		}
		list_destroy_and_destroy_elements(lista, free_tcb);
	}
}

tabla_paginas* crear_tabla_paginas(uint32_t pid){
	tabla_paginas* nueva_tabla = malloc(sizeof(tabla_paginas));
    log_info(logger,"se creo tabla de paginas de pid %d", pid);
	nueva_tabla->paginas = list_create();
	nueva_tabla->dl_tcbs = dictionary_create();
	pthread_mutex_init(&(nueva_tabla->mutex), NULL);
	char spid[4];
	sprintf(spid, "%d", pid);
    dictionary_put(tablas,spid,nueva_tabla);
    log_debug(logger,"Tabla de pid: %d creada",pid);
	return nueva_tabla;
}

// ████████╗███████╗░██████╗████████╗
// ╚══██╔══╝██╔════╝██╔════╝╚══██╔══╝
// ░░░██║░░░█████╗░░╚█████╗░░░░██║░░░
// ░░░██║░░░██╔══╝░░░╚═══██╗░░░██║░░░
// ░░░██║░░░███████╗██████╔╝░░░██║░░░
// ░░░╚═╝░░░╚══════╝╚═════╝░░░░╚═╝░░░

void imprimir_paginas(int pid){
    tabla_paginas* tabla = buscar_tabla(pid);
	if(tabla == NULL)
		return;
	int size = list_size(tabla->paginas);

    printf("\n<------ PAGINAS de tabla %d -----------\n", pid);
    for(int i = 0; i < size; i++) {
        pagina* s = list_get(tabla->paginas, i);
        printf("pagina %d, ocu: %d, en memoria: %s, disco: %d \n", i+1, s->tamano_ocupado, s->en_memoria?"Si":"No",s->disk_index);
    }
    printf("------------------->\n");

}

void imprimir_marcos(){
	int size = list_size(marcos);

    printf("\n<------ MARCOS -----------\n");
    for(int i = 0; i < size; i++) {
        marco* marco = list_get(marcos, i);
		if(marco->libre){
			printf("marco %d\tbase: %d\tlibre: true\t proceso: -\tpagina: -\n", i, marco->base);
		}else{
        	printf("marco %d\tbase: %d\tlibre: false\t proceso: %d\tpagina:%d\n", i, marco->base, marco->pid, marco->num_pagina);
		}
    }
    printf("------------------->\n");
}

void test_gestionar_tareas_paginacion(){
	t_archivo_tareas* arc = malloc(sizeof(t_archivo_tareas));
	arc->texto = "GENERAR_OXIGENO 10;1;1;1";
	arc->largo_texto = 25;
	arc->pid = 1;
	gestionar_tareas(arc);
    free(arc);

	tabla_paginas* tabla = (tabla_paginas*) buscar_tabla(1);
	
	char* tareas = (char*) rescatar_de_paginas(tabla, 0, tabla->dl_pcb, 1);
	log_info(logger,"Tareas: %s", tareas);
	free(tareas);

	t_TCB* tcb = malloc(sizeof(t_TCB));
    tcb->TID = 10001;
    tcb->coord_x = 1;
    tcb->coord_y = 2;
    tcb->estado_tripulante = 'N';
    gestionar_tcb(tcb);
    free(tcb);

	t_TCB* tcb3 = malloc(sizeof(t_TCB));
    tcb3->TID = 10002;
    tcb3->coord_x = 10;
    tcb3->coord_y = 100;
    tcb3->estado_tripulante = 'N';
    gestionar_tcb(tcb3);
    free(tcb3);

	t_archivo_tareas* arc2 = malloc(sizeof(t_archivo_tareas));
	arc2->texto = "GiNiRiR_iXiGiNi 10;1;1;1\nGiNiRiR_iXiGiNi 10;1;1;1";
	arc2->largo_texto = 50;
	arc2->pid = 2;
	gestionar_tareas(arc2);
    free(arc2);

	t_TCB* tcb2 = malloc(sizeof(t_TCB));
    tcb2->TID = 20001;
    tcb2->coord_x = 10;
    tcb2->coord_y = 100;
    tcb2->estado_tripulante = 'N';
    gestionar_tcb(tcb2);
    free(tcb2);

	tabla = (tabla_paginas*) buscar_tabla(2);
	char* tareas2 = (char*) rescatar_de_paginas(tabla, 0, tabla->dl_pcb, 2);
	log_info(logger,"Tareas: %s", tareas2);
	free(tareas2);

	t_TCB* tcb_r = (t_TCB*) buscar_tcb_por_tid(20001);
	log_debug(logger,"TID: %d, Coord_x: %d", tcb_r->TID, tcb_r->coord_x);
	free(tcb_r);

	imprimir_paginas(1);
	imprimir_paginas(2);
	imprimir_marcos();

	eliminar_tcb(10001);
	imprimir_paginas(1);
	eliminar_tcb(10002);

	imprimir_paginas(1);
	imprimir_marcos();
}

uint64_t unix_epoch() {

    long int ns;
    uint64_t all;
    time_t sec;
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);
    sec = spec.tv_sec;
    ns = spec.tv_nsec;

    all = (uint64_t) sec * 1000000000 + (uint64_t) ns;
    return all;
}
