#include "paginacion.h"
#define ESQUEMA_MEMORIA config_get_string_value(config, "ESQUEMA_MEMORIA")
#define TAMANIO_PAGINA config_get_int_value(config, "TAMANIO_PAGINA")
#define ALGORITMO_REEMPLAZO config_get_string_value(config, "ALGORITMO_REEMPLAZO")
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

// MMMMMMMM               MMMMMMMMEEEEEEEEEEEEEEEEEEEEEEMMMMMMMM               MMMMMMMM     OOOOOOOOO     RRRRRRRRRRRRRRRRR   IIIIIIIIII               AAA               
// M:::::::M             M:::::::ME::::::::::::::::::::EM:::::::M             M:::::::M   OO:::::::::OO   R::::::::::::::::R  I::::::::I              A:::A              
// M::::::::M           M::::::::ME::::::::::::::::::::EM::::::::M           M::::::::M OO:::::::::::::OO R::::::RRRRRR:::::R I::::::::I             A:::::A             
// M:::::::::M         M:::::::::MEE::::::EEEEEEEEE::::EM:::::::::M         M:::::::::MO:::::::OOO:::::::ORR:::::R     R:::::RII::::::II            A:::::::A            
// M::::::::::M       M::::::::::M  E:::::E       EEEEEEM::::::::::M       M::::::::::MO::::::O   O::::::O  R::::R     R:::::R  I::::I             A:::::::::A           
// M:::::::::::M     M:::::::::::M  E:::::E             M:::::::::::M     M:::::::::::MO:::::O     O:::::O  R::::R     R:::::R  I::::I            A:::::A:::::A          
// M:::::::M::::M   M::::M:::::::M  E::::::EEEEEEEEEE   M:::::::M::::M   M::::M:::::::MO:::::O     O:::::O  R::::RRRRRR:::::R   I::::I           A:::::A A:::::A         
// M::::::M M::::M M::::M M::::::M  E:::::::::::::::E   M::::::M M::::M M::::M M::::::MO:::::O     O:::::O  R:::::::::::::RR    I::::I          A:::::A   A:::::A        
// M::::::M  M::::M::::M  M::::::M  E:::::::::::::::E   M::::::M  M::::M::::M  M::::::MO:::::O     O:::::O  R::::RRRRRR:::::R   I::::I         A:::::A     A:::::A       
// M::::::M   M:::::::M   M::::::M  E::::::EEEEEEEEEE   M::::::M   M:::::::M   M::::::MO:::::O     O:::::O  R::::R     R:::::R  I::::I        A:::::AAAAAAAAA:::::A      
// M::::::M    M:::::M    M::::::M  E:::::E             M::::::M    M:::::M    M::::::MO:::::O     O:::::O  R::::R     R:::::R  I::::I       A:::::::::::::::::::::A     
// M::::::M     MMMMM     M::::::M  E:::::E       EEEEEEM::::::M     MMMMM     M::::::MO::::::O   O::::::O  R::::R     R:::::R  I::::I      A:::::AAAAAAAAAAAAA:::::A    
// M::::::M               M::::::MEE::::::EEEEEEEE:::::EM::::::M               M::::::MO:::::::OOO:::::::ORR:::::R     R:::::RII::::::II   A:::::A             A:::::A   
// M::::::M               M::::::ME::::::::::::::::::::EM::::::M               M::::::M OO:::::::::::::OO R::::::R     R:::::RI::::::::I  A:::::A               A:::::A  
// M::::::M               M::::::ME::::::::::::::::::::EM::::::M               M::::::M   OO:::::::::OO   R::::::R     R:::::RI::::::::I A:::::A                 A:::::A 
// MMMMMMMM               MMMMMMMMEEEEEEEEEEEEEEEEEEEEEEMMMMMMMM               MMMMMMMM     OOOOOOOOO     RRRRRRRR     RRRRRRRIIIIIIIIIIAAAAAAA                   AAAAAAA

tabla_paginas* crear_tabla_paginas(uint32_t pid){

	tabla_paginas* nueva_tabla = malloc(sizeof(tabla_paginas));
    log_debug(logger,"[PAG]: Creo tabla de paginas PID %d", pid);
	
	nueva_tabla->paginas = list_create();
	nueva_tabla->dl_tcbs = dictionary_create();
	pthread_mutex_init(&(nueva_tabla->mutex), NULL);

	char spid[4];
	sprintf(spid, "%d", pid);
	
	bloquear_lista_tablas();
    dictionary_put(tablas,spid,nueva_tabla);
	desbloquear_lista_tablas();
	
	return nueva_tabla;
}

int liberar_pagina(pagina* pagina, int offset, int faltante){
	// retorna lo que logró liberar
	if(offset + faltante <= TAMANIO_PAGINA){
		pagina->tamano_ocupado -= faltante;
		return faltante;
	}
	pagina->tamano_ocupado -= (TAMANIO_PAGINA - offset);
	return TAMANIO_PAGINA - offset; // pude liberar hasta el fin de la página 
}

void liberar_paginas(tabla_paginas* tabla, int dl, int tam, int pid){
	int faltante = tam;
	int num_pagina = dl / TAMANIO_PAGINA;
	int offset = dl % TAMANIO_PAGINA;
	pagina* pag;

	t_list* paginas_a_remover = list_create(); // lista de paginas enteras que tengo que eliminar de la tabla

	if(offset > 0){
		pag = list_get(tabla->paginas, num_pagina);
		bloquear_pagina(pag);
		faltante -= liberar_pagina(pag, offset, faltante);
		desbloquear_pagina(pag);

		if(pag->tamano_ocupado <= 0){
			list_add(paginas_a_remover, (void*) num_pagina);
		}
		num_pagina++;
	}

	while(faltante > 0){
		pag = list_get(tabla->paginas, num_pagina);
		bloquear_pagina(pag);
		faltante -= liberar_pagina(pag, 0, faltante);
		desbloquear_pagina(pag);

		if(pag->tamano_ocupado <= 0){
			list_add(paginas_a_remover, (void*) num_pagina);
		}
		num_pagina++;
	}

	void page_remover(void* un_int){
		int num = (int) un_int;
		pagina* pag2 = list_get(tabla->paginas, num);
		if(pag2->en_memoria){
			pag2->puntero_marco->libre = true;
			pag2->puntero_marco->num_pagina = 0;
			pag2->puntero_marco->pid = 0;
		}
		log_trace(logger,"[SWAP]: Libero el espacio en disco: %d, de PID: %d PAG: %d", pag2->disk_index, pid, num);
		bitmap_disco[pag->disk_index] = false;
		pag2->disk_index = -1;
		//list_remove(tabla->paginas, num);
		//free(pag2);
	}
	bloquear_lista_marcos();
	list_iterate(paginas_a_remover, page_remover);
	list_destroy(paginas_a_remover);
	desbloquear_lista_marcos();
}

int matar_paginas_tcb(tabla_paginas* tabla, int tid){
	char stid[8];
	sprintf(stid, "%d", tid);

	int dl_tcb = (int) dictionary_get(tabla->dl_tcbs, stid);
	if(dl_tcb == (int) NULL){
		log_error(logger, "[PAG]: TCB no encontrado TID: %d", tid);
		return 0;
	}

	liberar_paginas(tabla, dl_tcb, sizeof(t_TCB), tid / 10000);
	dictionary_remove(tabla->dl_tcbs, stid);
	return 1;
}

void matar_tabla_paginas(int pid){
    bloquear_lista_tablas();
	log_debug(logger, "[PAG]: Matando patota PID: %d", pid);

    void table_destroyer(void* una_tabla){
        tabla_paginas* tabla = (tabla_paginas*) una_tabla;

		void page_destroyer(void* una_pagina){
			pagina* pag = (pagina*) una_pagina;
			if(pag->en_memoria){
				pag->puntero_marco->libre = true;
			}
			if(pag->disk_index >= 0){
				bitmap_disco[pag->disk_index] = false;
			}
			free(pag);
		}

		list_destroy_and_destroy_elements(tabla->paginas, page_destroyer);

		dictionary_destroy(tabla->dl_tcbs);

		free(tabla);
    }

    char spid[4];
    sprintf(spid, "%d", pid);
    dictionary_remove_and_destroy(tablas,spid,table_destroyer);
	
	log_info(logger, "Se mató la tabla de paginas");
	desbloquear_lista_tablas();
}

int sobreescribir_paginas(tabla_paginas* tabla, void* data, int dl, int tam, int pid){
	pagina* pagina;
	int progreso = 0;
	int num_pagina = dl / TAMANIO_PAGINA;
	int offset = dl % TAMANIO_PAGINA;

	if(offset > 0){
		pagina = get_pagina(tabla->paginas, pid, num_pagina);
		if(pagina != NULL){
			pagina->modificada = true;
			progreso += escribir_en_marco(pagina->puntero_marco, data + progreso, offset, tam - progreso);
			desbloquear_pagina(pagina);
			num_pagina++;
			log_trace(logger, "[PAG]: Escribiendo... %d / %d bytes", progreso, tam);
		}else{
			log_error(logger, "Se intentó escribir sobre una página que ya fue eliminada");
			return 0;
		}
	}
	while(progreso < tam){
		pagina = get_pagina(tabla->paginas, pid, num_pagina);
		if(pagina != NULL){
			pagina->modificada = true;
			progreso += escribir_en_marco(pagina->puntero_marco, data + progreso, 0, tam - progreso);
			desbloquear_pagina(pagina);
			num_pagina++;
			log_trace(logger, "[PAG]: Escribiendo... %d / %d bytes", progreso, tam);
		}else{
			log_error(logger, "Se intentó escribir sobre una página que ya fue eliminada");
			return 0;
		}
	}
	return 1;
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
	void* data = malloc(tam); // puntero a retornar con la info solicitada
	pagina* pagina; 
	int faltante = tam;
	int num_pagina = dl / TAMANIO_PAGINA;
	int offset = dl % TAMANIO_PAGINA;

	if(offset > 0){
		pagina = get_pagina(tabla->paginas, pid, num_pagina);
		if(pagina == NULL)
			return NULL;
		faltante -= rescatar_de_marco(pagina->puntero_marco, data + tam - faltante, offset, faltante);
		desbloquear_pagina(pagina);
		log_trace(logger, "[PAG]: Leyendo... %d / %d bytes", tam - faltante, tam);
		num_pagina++;
	}

	while(faltante > 0){

		pagina = get_pagina(tabla->paginas, pid, num_pagina);
		if(pagina == NULL)
			return NULL;
		faltante -= rescatar_de_marco(pagina->puntero_marco, data + tam - faltante, 0, faltante);
		desbloquear_pagina(pagina);
		log_trace(logger, "[PAG]: Leyendo... %d / %d bytes", tam - faltante, tam);
		num_pagina++;
	}
	
	return data;
}

int rescatar_de_marco(marco* marco, void* data, int offset, int tam){
	//esta funcion retorna lo que de rescató de memoria
	int tam_rescate = MIN(tam, TAMANIO_PAGINA - offset);
	memcpy(data, memoria_principal + marco->base + offset, tam_rescate);
	return tam_rescate;
}

int agregar_paginas_segun_tamano(tabla_paginas* tabla, void* data, int tam, int pid){
	// esta función devuelve la dirección lógica de lo que se guardó
	int dl;
	int progreso = 0;
	int offset = 0;

	bloquear_tabla(tabla);
	int num_pagina = list_size(tabla->paginas) - 1;

	if(num_pagina >= 0){ // hay alguna pagina
		pagina* ultima_pagina = get_pagina(tabla->paginas, pid, num_pagina);
		if(ultima_pagina != NULL){
			if(ultima_pagina->tamano_ocupado < TAMANIO_PAGINA ){ // la ultima pagina no está llena
				offset = ultima_pagina->tamano_ocupado;
				progreso += escribir_en_marco(ultima_pagina->puntero_marco, data, offset, tam);
				ultima_pagina->tamano_ocupado += progreso;
				log_trace(logger, "Escribiendo... : %d / %d bytes", progreso, tam);
			}else{
				num_pagina++;
			}
			desbloquear_pagina(ultima_pagina);
		}
	}else{
		num_pagina = 0;
	}
	// armo la DL a retornar
	dl = num_pagina * TAMANIO_PAGINA + offset;

	while(progreso < tam){
		int temp = agregar_pagina(tabla, data + progreso, tam - progreso, pid);
		if(temp > 0){
			progreso += temp;
			log_trace(logger, "Escribiendo... %d / %d bytes", progreso, tam);
		}else{
			// no hubo mas progreso, por ende no hay mas memoria
			return 99999; // se retorna estó porque NULL = 0 y me lo tomaría como la DL 0
		}
	}
	desbloquear_tabla(tabla);
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
	pag->disk_index = get_disk_index();
	if(pag->disk_index == -1){
		// no hay mas memoria
		pag->puntero_marco->libre = true;
		free(pag);
		return 0;
	}
	pag->modificada = true;
	pag->ultimo_uso =  get_timestamp();
	pag->usado = true;
	pag->en_memoria = true;
	pthread_mutex_init(&(pag->mutex),NULL);

	list_add(tabla->paginas,pag);
	pag->tamano_ocupado = escribir_en_marco(pag->puntero_marco, data, 0, tam);
	return pag->tamano_ocupado;
}

marco* buscar_marco_libre(){
    log_info(logger,"[PAG]: Buscando un marco libre");
    int size = list_size(marcos);
	for(int i=0; i<size; i++){
        marco* x = list_get(marcos, i);
        if(x->libre){
			x->libre = false;
            return x;
        }
    }
    log_warning(logger, "[PAG]: No se encontró marco libre");
    return NULL;
}

marco* asignar_marco(){
	bloquear_lista_marcos();
	marco* marco_libre = buscar_marco_libre();
	if(marco_libre != NULL){
		intento_asignar_marco = 0;
		desbloquear_lista_marcos();
		return marco_libre;
	}

	if(intento_asignar_marco == 1){
		intento_asignar_marco = 0;
		desbloquear_lista_marcos();
		return NULL;
	}

	intento_asignar_marco = 1;
	algoritmo_de_reemplazo();
	desbloquear_lista_marcos();
	return asignar_marco();
}

// VVVVVVVV           VVVVVVVVIIIIIIIIIIRRRRRRRRRRRRRRRRR   TTTTTTTTTTTTTTTTTTTTTTTUUUUUUUU     UUUUUUUU           AAA               LLLLLLLLLLL             
// V::::::V           V::::::VI::::::::IR::::::::::::::::R  T:::::::::::::::::::::TU::::::U     U::::::U          A:::A              L:::::::::L             
// V::::::V           V::::::VI::::::::IR::::::RRRRRR:::::R T:::::::::::::::::::::TU::::::U     U::::::U         A:::::A             L:::::::::L             
// V::::::V           V::::::VII::::::IIRR:::::R     R:::::RT:::::TT:::::::TT:::::TUU:::::U     U:::::UU        A:::::::A            LL:::::::LL             
//  V:::::V           V:::::V   I::::I    R::::R     R:::::RTTTTTT  T:::::T  TTTTTT U:::::U     U:::::U        A:::::::::A             L:::::L               
//   V:::::V         V:::::V    I::::I    R::::R     R:::::R        T:::::T         U:::::D     D:::::U       A:::::A:::::A            L:::::L               
//    V:::::V       V:::::V     I::::I    R::::RRRRRR:::::R         T:::::T         U:::::D     D:::::U      A:::::A A:::::A           L:::::L               
//     V:::::V     V:::::V      I::::I    R:::::::::::::RR          T:::::T         U:::::D     D:::::U     A:::::A   A:::::A          L:::::L               
//      V:::::V   V:::::V       I::::I    R::::RRRRRR:::::R         T:::::T         U:::::D     D:::::U    A:::::A     A:::::A         L:::::L               
//       V:::::V V:::::V        I::::I    R::::R     R:::::R        T:::::T         U:::::D     D:::::U   A:::::AAAAAAAAA:::::A        L:::::L               
//        V:::::V:::::V         I::::I    R::::R     R:::::R        T:::::T         U:::::D     D:::::U  A:::::::::::::::::::::A       L:::::L               
//         V:::::::::V          I::::I    R::::R     R:::::R        T:::::T         U::::::U   U::::::U A:::::AAAAAAAAAAAAA:::::A      L:::::L         LLLLLL
//          V:::::::V         II::::::IIRR:::::R     R:::::R      TT:::::::TT       U:::::::UUU:::::::UA:::::A             A:::::A   LL:::::::LLLLLLLLL:::::L
//           V:::::V          I::::::::IR::::::R     R:::::R      T:::::::::T        UU:::::::::::::UUA:::::A               A:::::A  L::::::::::::::::::::::L
//            V:::V           I::::::::IR::::::R     R:::::R      T:::::::::T          UU:::::::::UU A:::::A                 A:::::A L::::::::::::::::::::::L
//             VVV            IIIIIIIIIIRRRRRRRR     RRRRRRR      TTTTTTTTTTT            UUUUUUUUU  AAAAAAA                   AAAAAAALLLLLLLLLLLLLLLLLLLLLLLL

void algoritmo_de_reemplazo(){
	pagina* pagina;

	if(strcmp(ALGORITMO_REEMPLAZO,"LRU")==0){
		pagina = get_lru();
	}else if(strcmp(ALGORITMO_REEMPLAZO,"CLOCK")==0){
		pagina = get_clock();
	}else{
		log_error(logger,"Algoritmo de reemplazo desconocido");
		exit(EXIT_FAILURE);
	}

	swap_in(pagina);
	desbloquear_pagina(pagina);
	desbloquear_paginas_en_memoria();
}

void swap_in(pagina* pag){
	log_info(logger, "[SWAP]: SWAP-IN PID: %d, PAG: %d", pag->puntero_marco->pid,pag->puntero_marco->num_pagina);
	// escribo la página en disco, accediendo directamente con su indice
	if(pag->modificada){
		void* data = memoria_principal + pag->puntero_marco->base;
		bloquear_disco();
		log_info(logger, "[SWAP]: Escribo en disco en la pos: %d", pag->disk_index);
		fseek(disco, TAMANIO_PAGINA * pag->disk_index, SEEK_SET);
		fwrite(data, TAMANIO_PAGINA, 1, disco);
		desbloquear_disco();
	}
	pag->en_memoria = false;
	pag->puntero_marco->libre = true;
	pag->puntero_marco = NULL;
}

void swap_out(pagina* pagina){
	log_info(logger, "[SWAP]: SWAP-OUT");
	pagina->puntero_marco = asignar_marco(); // asignar marco me pone el marco en libre->false
	//bloquear_swap();

	bloquear_disco();
	log_info(logger, "[SWAP]: Leo en disco en la pos: %d", pagina->disk_index);
	fseek(disco, pagina->disk_index * TAMANIO_PAGINA, SEEK_SET);
	fread(memoria_principal + pagina->puntero_marco->base, TAMANIO_PAGINA, 1, disco);
	desbloquear_disco();
	//desbloquear_swap();
}

void page_fault(pagina* pag, int pid, int num){
	log_error(logger,"[SWAP]: PageFault PID: %d, pag: %d, pos.disco: %d", pid, num + 1, pag->disk_index);
	swap_out(pag);
	pag->puntero_marco->pid = pid;
	pag->puntero_marco->num_pagina = num + 1;
	pag->en_memoria = true;
	pag->modificada = false;
}

pagina* get_lru(){
	log_info(logger, "[SWAP]: Ejecuto busqueda por LRU");
	bloquear_paginas_en_memoria();
	uint64_t lru_ts = get_timestamp();
    pagina* lru_p = NULL;
    
	void page_iterator(void* un_marco){
		marco* marc = (marco*) un_marco;
		pagina* pagin = get_pagina_from_marco(marc);
		if(pagin != NULL){
			if(pagin->ultimo_uso <= lru_ts){
				lru_ts = pagin->ultimo_uso;
				lru_p = pagin;
			}
		}
	}

	list_iterate(marcos, page_iterator);

    return lru_p;
}

pagina* get_clock(){
	bloquear_paginas_en_memoria();
	log_info(logger, "[SWAP]: Ejecuto busqueda por CLOCK");
    pagina* clock_p;
	int cantidad_marcos = list_size(marcos);
	marco* marco_actual;
	while(1){
		marco_actual = list_get(marcos, marco_clock);
		pagina* pag = get_pagina_from_marco(marco_actual);
		if(pag != NULL && pag->disk_index != -1){
			if(marco_clock < cantidad_marcos-1){
				marco_clock++;
			}else{
				marco_clock = 0;
			}
			
			if(!pag->usado){
				clock_p = pag;
				break;
			}else{
				pag->usado = false;
			}
		}else{
			log_error(logger, "[SWAP]: Se quiso acceder a una página ya liberada");
		}
	}
    return clock_p;
}

pagina* get_pagina_from_marco(marco* marco){
	if(marco->libre)
		return NULL;

	int pid = marco->pid;
	int num_pagina = marco->num_pagina;

	tabla_paginas* tabla = buscar_tabla(pid);
	return list_get(tabla->paginas, num_pagina - 1);
}

marco* crear_marco(int base, bool libre){
    marco* nuevo_marco = malloc(sizeof(marco));
    nuevo_marco->base = base;
    nuevo_marco->libre = libre;
    return nuevo_marco;
}

void liberar_lista_tcbs_paginacion(t_list* lista){
	void free_tcb(void* un_tcb){
		free(un_tcb);
	}
	list_destroy_and_destroy_elements(lista, free_tcb);
}

void dump_paginacion(){
	bloquear_lista_marcos();
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
	char* temporal = temporal_get_string_time("%d/%m/%y %H:%M:%S");
	char* string_a_printear = string_from_format("Dump: %s\n", temporal);
    txt_write_in_file(file, string_a_printear);
    list_iterate(marcos, impresor_dump);
	free(temporal);
	free(string_a_printear);
    txt_close_file(file);
	desbloquear_lista_marcos();
}

uint64_t get_timestamp() {

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

int get_disk_index(){
	//la pagina no está en disco, buscar espacio libre
	log_info(logger, "[SWAP]: página nueva, le busco espacio en disco");
	bloquear_disco();
	for(int i = 0; i < marcos_disco_size; i++){
		if(!bitmap_disco[i]){
			bitmap_disco[i] = true;
			log_info(logger, "[SWAP]: Espacio disco asignado: %d", i);
			desbloquear_disco();
			return i;
		}
	}
	desbloquear_disco();
	log_error(logger, "No hay mas memoria virtual");
	return -1;
}

//   SSSSSSSSSSSSSSS EEEEEEEEEEEEEEEEEEEEEEMMMMMMMM               MMMMMMMM               AAA               FFFFFFFFFFFFFFFFFFFFFF     OOOOOOOOO     RRRRRRRRRRRRRRRRR        OOOOOOOOO        SSSSSSSSSSSSSSS 
//  SS:::::::::::::::SE::::::::::::::::::::EM:::::::M             M:::::::M              A:::A              F::::::::::::::::::::F   OO:::::::::OO   R::::::::::::::::R     OO:::::::::OO    SS:::::::::::::::S
// S:::::SSSSSS::::::SE::::::::::::::::::::EM::::::::M           M::::::::M             A:::::A             F::::::::::::::::::::F OO:::::::::::::OO R::::::RRRRRR:::::R  OO:::::::::::::OO S:::::SSSSSS::::::S
// S:::::S     SSSSSSSEE::::::EEEEEEEEE::::EM:::::::::M         M:::::::::M            A:::::::A            FF::::::FFFFFFFFF::::FO:::::::OOO:::::::ORR:::::R     R:::::RO:::::::OOO:::::::OS:::::S     SSSSSSS
// S:::::S              E:::::E       EEEEEEM::::::::::M       M::::::::::M           A:::::::::A             F:::::F       FFFFFFO::::::O   O::::::O  R::::R     R:::::RO::::::O   O::::::OS:::::S            
// S:::::S              E:::::E             M:::::::::::M     M:::::::::::M          A:::::A:::::A            F:::::F             O:::::O     O:::::O  R::::R     R:::::RO:::::O     O:::::OS:::::S            
//  S::::SSSS           E::::::EEEEEEEEEE   M:::::::M::::M   M::::M:::::::M         A:::::A A:::::A           F::::::FFFFFFFFFF   O:::::O     O:::::O  R::::RRRRRR:::::R O:::::O     O:::::O S::::SSSS         
//   SS::::::SSSSS      E:::::::::::::::E   M::::::M M::::M M::::M M::::::M        A:::::A   A:::::A          F:::::::::::::::F   O:::::O     O:::::O  R:::::::::::::RR  O:::::O     O:::::O  SS::::::SSSSS    
//     SSS::::::::SS    E:::::::::::::::E   M::::::M  M::::M::::M  M::::::M       A:::::A     A:::::A         F:::::::::::::::F   O:::::O     O:::::O  R::::RRRRRR:::::R O:::::O     O:::::O    SSS::::::::SS  
//        SSSSSS::::S   E::::::EEEEEEEEEE   M::::::M   M:::::::M   M::::::M      A:::::AAAAAAAAA:::::A        F::::::FFFFFFFFFF   O:::::O     O:::::O  R::::R     R:::::RO:::::O     O:::::O       SSSSSS::::S 
//             S:::::S  E:::::E             M::::::M    M:::::M    M::::::M     A:::::::::::::::::::::A       F:::::F             O:::::O     O:::::O  R::::R     R:::::RO:::::O     O:::::O            S:::::S
//             S:::::S  E:::::E       EEEEEEM::::::M     MMMMM     M::::::M    A:::::AAAAAAAAAAAAA:::::A      F:::::F             O::::::O   O::::::O  R::::R     R:::::RO::::::O   O::::::O            S:::::S
// SSSSSSS     S:::::SEE::::::EEEEEEEE:::::EM::::::M               M::::::M   A:::::A             A:::::A   FF:::::::FF           O:::::::OOO:::::::ORR:::::R     R:::::RO:::::::OOO:::::::OSSSSSSS     S:::::S
// S::::::SSSSSS:::::SE::::::::::::::::::::EM::::::M               M::::::M  A:::::A               A:::::A  F::::::::FF            OO:::::::::::::OO R::::::R     R:::::R OO:::::::::::::OO S::::::SSSSSS:::::S
// S:::::::::::::::SS E::::::::::::::::::::EM::::::M               M::::::M A:::::A                 A:::::A F::::::::FF              OO:::::::::OO   R::::::R     R:::::R   OO:::::::::OO   S:::::::::::::::SS 
//  SSSSSSSSSSSSSSS   EEEEEEEEEEEEEEEEEEEEEEMMMMMMMM               MMMMMMMMAAAAAAA                   AAAAAAAFFFFFFFFFFF                OOOOOOOOO     RRRRRRRR     RRRRRRR     OOOOOOOOO      SSSSSSSSSSSSSSS   

pagina* get_pagina(t_list* paginas, int pid, int num_pagina){
	pagina* pagina = list_get(paginas, num_pagina);
	if(pagina->disk_index == -1){
		log_error(logger, "Se intentó acceder a una pagina ya liberada");
		return NULL; // pagina ya fue eliminada bro
	}
	bloquear_pagina(pagina);
	if(!pagina->en_memoria){
		bloquear_swap();
		page_fault(pagina, pid, num_pagina);
		desbloquear_swap();
	}
	pagina->ultimo_uso =  get_timestamp();
	pagina->usado = true;
	return pagina;
}

void bloquear_pagina(pagina* pagina){
	pthread_mutex_lock(&(pagina->mutex));
	log_trace(logger,"[SEM]: Bloqueo pagina");
}

void desbloquear_pagina(pagina* pagina){
	pthread_mutex_unlock(&(pagina->mutex));
	log_trace(logger,"[SEM]: Desbloqueo pagina");
}

void bloquear_lista_marcos(){
	pthread_mutex_lock(&m_lista_marcos);
	log_trace(logger,"[SEM]: Bloqueo lista marcos");
}

void desbloquear_lista_marcos(){
	pthread_mutex_unlock(&m_lista_marcos);
	log_trace(logger,"[SEM]: Desbloqueo lista marcos");
}

void bloquear_swap(){
	pthread_mutex_lock(&m_swap);
	log_trace(logger,"[SEM]: Bloqueo SWAP");
}

void desbloquear_swap(){
	pthread_mutex_unlock(&m_swap);
	log_trace(logger,"[SEM]: Desbloqueo SWAP");
}

void bloquear_disco(){
	pthread_mutex_lock(&m_disco);
	log_trace(logger,"[SEM]: Bloqueo disco");
}

void desbloquear_disco(){
	pthread_mutex_unlock(&m_disco);
	log_trace(logger,"[SEM]: Desbloqueo disco");
}

void bloquear_paginas_en_memoria(){
	void page_locker(void* un_marco){
		marco* marc = (marco*) un_marco;
		pagina* pag = get_pagina_from_marco(marc);
		if(pag != NULL)
			bloquear_pagina(pag);
	}
	list_iterate(marcos,page_locker);
	log_trace(logger,"[SEM]: Bloqueo paginas en memoria");
}

void desbloquear_paginas_en_memoria(){
	void page_unlocker(void* un_marco){
		marco* marc = (marco*) un_marco;
		pagina* pag = get_pagina_from_marco(marc);
		if(pag != NULL)
			desbloquear_pagina(pag);
	}
	log_trace(logger,"[SEM]: Desbloqueo paginas en memoria");
	list_iterate(marcos,page_unlocker);
}



test(){

}

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
	arc->texto = "GENERAR_OXIGENO 10;1;1;1\nGENERAR_OXIGENO 10;2;2;2\nGENERAR_OXIGENO 10;3;2;2\nGENERAR_OXIGENO 10;4;2;2";
	arc->largo_texto = 99;
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

    tcb->TID = 10002;
    tcb->coord_x = 1;
    tcb->coord_y = 2;
    tcb->estado_tripulante = 'N';
    gestionar_tcb(tcb);
    free(tcb);

	buscar_siguiente_tarea(10001);
	buscar_siguiente_tarea(10001);
	buscar_siguiente_tarea(10002);
	buscar_siguiente_tarea(10002);
	buscar_siguiente_tarea(10002);
	buscar_siguiente_tarea(10002);
	buscar_siguiente_tarea(10002);
	buscar_siguiente_tarea(10001);

	imprimir_paginas(1);
	imprimir_marcos();
}
