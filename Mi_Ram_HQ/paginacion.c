#include "paginacion.h"
#define ESQUEMA_MEMORIA config_get_string_value(config, "ESQUEMA_MEMORIA")
#define TAMANIO_PAGINA config_get_int_value(config, "TAMANIO_PAGINA")
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

int sobreescribir_paginas(tabla_paginas* tabla, void* data, int dl, int tam){
	pagina* pagina;
	int progreso = 0;
	int numero_pagina = dl / TAMANIO_PAGINA;
	int offset = dl % TAMANIO_PAGINA;
	if(offset > 0){
		pagina = list_get(tabla->paginas, numero_pagina);
		progreso += escribir_en_marco(pagina->puntero_marco, data + progreso, offset, tam - progreso);
		numero_pagina++;
		//log_info(logger, "Progreso: %d / %d bytes", progreso, tam);
	}
	while(progreso < tam){
		pagina = list_get(tabla->paginas, numero_pagina);
		progreso += rescatar_de_marco(pagina->puntero_marco, data + progreso, 0, tam - progreso);
		numero_pagina++;
		//log_info(logger, "Progreso: %d / %d bytes", progreso, tam);
	}
}

int escribir_en_marco(marco* marco, void* data, int offset, int tam){
	// retorna lo que se logró escribir
	if(tam <= TAMANIO_PAGINA){
		memcpy(memoria_principal + marco->base + offset, data, tam);
		return tam;
	}else{
		memcpy(memoria_principal + marco->base + offset, data, TAMANIO_PAGINA);
		return TAMANIO_PAGINA;
	}
}

void* rescatar_de_paginas(tabla_paginas* tabla, int dl, int tam){
	void* data = malloc(tam);
	pagina* pagina; 
	int faltante = tam;
	int numero_pagina = dl / TAMANIO_PAGINA;
	int offset = dl % TAMANIO_PAGINA;
	//log_info(logger, "Se empieza a buscar en la pagina %d con offset %d, tam %d", numero_pagina, offset, tam);
	if(offset > 0){
		pagina = list_get(tabla->paginas, numero_pagina);
		faltante -= rescatar_de_marco(pagina->puntero_marco, data + tam - faltante, offset, faltante);
		numero_pagina++;
		//log_info(logger, "Progreso: %d / %d bytes", tam-faltante, tam);
	}
	while(faltante > 0){
		pagina = list_get(tabla->paginas, numero_pagina);
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
	if(disponible - tam >= 0){
		pagina->tamano_ocupado += tam;
		memcpy(memoria_principal + marco->base + ocupado, data, tam);
		return tam; // pude guardar todo
	}

	pagina->tamano_ocupado = TAMANIO_PAGINA;
	memcpy(memoria_principal + marco->base + ocupado, data, disponible);
	return disponible; // pude guardar solo el tamaño disponible 
}

int agregar_paginas_segun_tamano(tabla_paginas* tabla, void* data, int tam){
	// esta función devuelve la dirección lógica de lo que se guardó
	int dl;
	int progreso = 0;
	// para calcular la dl
	int numero_pagina;
	int offset;

	pagina* ultima_pagina = pagina_incompleta(tabla);
	if(ultima_pagina != NULL){ // si hay una pagina incompleta
		offset = ultima_pagina->tamano_ocupado;
		numero_pagina = list_size(tabla->paginas) - 1;
		progreso += completar_pagina(ultima_pagina, data, tam);
		//log_info(logger, "Progreso: %d / %d bytes", progreso, tam);
	}else{
		numero_pagina = list_size(tabla->paginas);
		offset = 0;
	}
	dl = numero_pagina * TAMANIO_PAGINA + offset;

	while(progreso < tam){
		progreso += agregar_pagina(tabla, data + progreso, tam - progreso);
		//log_info(logger, "Progreso: %d / %d bytes", progreso, tam);
	}
	return dl;	
}

int agregar_pagina(tabla_paginas* tabla, void* data, int tam){
	// retorna lo que se logró guardar
	marco* marco = asignar_marco();
	pagina* pagina = crear_pagina(marco);
	list_add(tabla->paginas,pagina);

	if(tam <= TAMANIO_PAGINA){
		memcpy(memoria_principal + marco->base, data, tam);
		pagina->tamano_ocupado = tam;
		return tam;
	}else{
		memcpy(memoria_principal + marco->base, data, TAMANIO_PAGINA);
		pagina->tamano_ocupado = TAMANIO_PAGINA;
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


pagina* crear_pagina(marco* marco){
	pagina* pagina = malloc(sizeof(pagina));
    marco->libre = false;
	pagina->puntero_marco = marco;
	//log_info(logger,"Se crea página para el marco de base %d", marco->base);
	return pagina;
}

/*
int cantidad_marcos_completos(int tam){
    log_info(logger,"Calculando cantidad de marcos para el tamaño %d", tam);
	int marcos = tam / TAMANIO_PAGINA;  
	return marcos;
}

int ocupa_marco_incompleto(int tam){
	int parte_ocupada = tam % TAMANIO_PAGINA; 
	return parte_ocupada;
}

void liberar_marco(int num_marco){
    marco* x = list_get(marcos, num_marco);
    x->libre = true;
    log_info(logger, "Se libera el marco %d", num_marco);

}*/

marco* crear_marco(int base, bool libre){
    marco* nuevo_marco = malloc(sizeof(marco));
    nuevo_marco->base = base;
    nuevo_marco->libre = libre;

    return nuevo_marco;
}

tabla_paginas* crear_tabla_paginas(uint32_t pid){
	tabla_paginas* nueva_tabla = malloc(sizeof(tabla_paginas));
    //log_info(logger,"se creo tabla de paginas de pid %d", pid);
	nueva_tabla->paginas = list_create();
	nueva_tabla->dl_tcbs = dictionary_create();
	char spid[4];
	sprintf(spid, "%d", pid);
    dictionary_put(tablas,spid,nueva_tabla);
    //log_debug(logger,"Tabla de pid: %d creada",pid);
	return nueva_tabla;
}


marco* buscar_marco_libre(){
    //log_info(logger,"Buscando un marco libre");
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
		//Si el marco es del tamano justo, no tengo que reordenar
		marco_libre->libre = false;
		//log_info(logger,"Marco asignado (base:%d)", marco_libre->base);
		return marco_libre;
	}
	else{
		//TODO: efectuar swap
	}
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
		pag2->puntero_marco->libre = true;
		list_remove(tabla->paginas, num);
		free(pag2);
	}

	list_iterate(paginas_a_remover, page_remover);
	
}

int matar_paginas_tcb(tabla_paginas* tabla, int tid){
	log_info(logger, "Eliminando TCB tid: %d", tid);
	char stid[6];
	sprintf(stid, "%d", tid);
	int dl_tcb = dictionary_get(tabla->dl_tcbs, stid);
	if(dl_tcb == NULL){
		return 0;
	}
	liberar_paginas(tabla, dl_tcb, sizeof(t_TCB));
	dictionary_remove(tabla->dl_tcbs, stid);
	//log_info(logger, "Se termino de matar las paginas del TCB tid: %d", tid);
	return 1;
}

void matar_tabla_paginas(int pid){
	log_debug(logger, "Matando patota PID: %d", pid);
    
    void table_destroyer(void* una_tabla){
        tabla_paginas* tabla = (tabla_paginas*) una_tabla;

		void page_destroyer(void* una_pagina){
			pagina* pag = (pagina*) una_pagina;
			pag->puntero_marco->libre = true;
			free(pag);
		}

		list_destroy_and_destroy_elements(tabla->paginas, page_destroyer);

		void dl_tcb_destroyer(void* un_int){
			int* num = (int*) un_int;
			free(num);
		}
		dictionary_destroy_and_destroy_elements(tabla->dl_tcbs, dl_tcb_destroyer);

		free(tabla);
    }

    char spid[4];
    sprintf(spid, "%d", pid);
    dictionary_remove_and_destroy(tablas,spid,table_destroyer);

	//log_debug(logger, "Se completó la nismación de la tabla PID: %d", pid);
}

liberar_lista_tcbs_paginacion(t_list* lista){
	if(strcmp(ESQUEMA_MEMORIA, "PAGINACION") == 0){
		void free_tcb(void* un_tcb){
			free(un_tcb);
		}
		list_destroy_and_destroy_elements(lista, free_tcb);
	}
}
// TEST

void imprimir_paginas(int pid){
    tabla_paginas* tabla = buscar_tabla(pid);
	int size = list_size(tabla->paginas);

    printf("\n<------ PAGINAS de tabla %d -----------\n", pid);
    for(int i = 0; i < size; i++) {
        pagina* s = list_get(tabla->paginas, i);
        printf("pagina %d base: %d, tamaño ocupado: %d \n", i, s->puntero_marco->base, s->tamano_ocupado);
    }
    printf("------------------->\n");

}

void imprimir_marcos(){
	int size = list_size(marcos);

    printf("\n<------ MARCOS -----------\n");
    for(int i = 0; i < size; i++) {
        marco* marco = list_get(marcos, i);
        printf("marco %d\tbase: %d\tlibre: %s \n", i, marco->base, marco->libre ? "true" : "false");
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
	
	char* tareas = (char*) rescatar_de_paginas(tabla, 0, tabla->dl_pcb);
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
    //gestionar_tcb(tcb3);
    free(tcb3);

	imprimir_paginas(1);
	imprimir_marcos();

	eliminar_tcb(10001);
	//eliminar_tcb(10002);

	imprimir_marcos();
}
