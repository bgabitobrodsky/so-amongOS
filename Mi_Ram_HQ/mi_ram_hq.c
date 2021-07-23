#include "mi_ram_hq.h"

#define	IP config_get_string_value(config, "IP")
#define PUERTO config_get_string_value(config, "PUERTO")
#define TAMANIO_MEMORIA config_get_int_value(config, "TAMANIO_MEMORIA")
#define TAMANIO_PAGINA config_get_int_value(config, "TAMANIO_PAGINA")
#define ESQUEMA_MEMORIA config_get_string_value(config, "ESQUEMA_MEMORIA")
#define PATH_SWAP config_get_string_value(config, "PATH_SWAP")
#define TAMANIO_SWAP config_get_int_value(config, "TAMANIO_SWAP")
#define LIMIT_CONNECTIONS 10
#define ASSERT_CREATE(nivel, id, err)                                                   \
    if(err) {                                                                           \
        nivel_destruir(nivel);                                                          \
        nivel_gui_terminar();                                                           \
        fprintf(stderr, "Error al crear '%c': %s\n", id, nivel_gui_string_error(err));  \
        return EXIT_FAILURE;                                                            \
    }

bool mapa_on = false;

int main(int argc, char** argv) {
	
	// Reinicio el log
	FILE* f = fopen("mi_ram_hq.log", "w");
    fclose(f);

	logger = log_create("mi_ram_hq.log", "MI_RAM_HQ", !mapa_on, LOG_LEVEL_DEBUG);
	config = config_create("mi_ram_hq.config");
	signal(SIGUSR1, signal_compactacion);
	signal(SIGUSR2, dump);

	pthread_mutex_init(&m_swap,NULL);
	pthread_mutex_init(&m_lista_marcos,NULL);
	pthread_mutex_init(&m_lista_segmentos,NULL);
	pthread_mutex_init(&m_mapa,NULL);
	pthread_mutex_init(&m_disco,NULL);

	iniciar_memoria();
	iniciar_mapa();

	int socket_oyente = crear_socket_oyente(IP, PUERTO);
    	args_escuchar args_miram;
	args_miram.socket_oyente = socket_oyente;

	pthread_t hilo_escucha;
	pthread_create(&hilo_escucha, NULL, (void*) proceso_handler, (void*) &args_miram);

	//pthread_detach(hilo_escucha);
	pthread_join(hilo_escucha, NULL);

	close(socket_oyente);
	matar_mapa();
	free(bitmap_disco);
	free(memoria_principal);
	fclose(disco);
	log_destroy(logger);
	config_destroy(config);

	return EXIT_SUCCESS;
}

//         CCCCCCCCCCCCC     OOOOOOOOO     NNNNNNNN        NNNNNNNNEEEEEEEEEEEEEEEEEEEEEEXXXXXXX       XXXXXXXIIIIIIIIII     OOOOOOOOO     NNNNNNNN        NNNNNNNN
//      CCC::::::::::::C   OO:::::::::OO   N:::::::N       N::::::NE::::::::::::::::::::EX:::::X       X:::::XI::::::::I   OO:::::::::OO   N:::::::N       N::::::N
//    CC:::::::::::::::C OO:::::::::::::OO N::::::::N      N::::::NE::::::::::::::::::::EX:::::X       X:::::XI::::::::I OO:::::::::::::OO N::::::::N      N::::::N
//   C:::::CCCCCCCC::::CO:::::::OOO:::::::ON:::::::::N     N::::::NEE::::::EEEEEEEEE::::EX::::::X     X::::::XII::::::IIO:::::::OOO:::::::ON:::::::::N     N::::::N
//  C:::::C       CCCCCCO::::::O   O::::::ON::::::::::N    N::::::N  E:::::E       EEEEEEXXX:::::X   X:::::XXX  I::::I  O::::::O   O::::::ON::::::::::N    N::::::N
// C:::::C              O:::::O     O:::::ON:::::::::::N   N::::::N  E:::::E                X:::::X X:::::X     I::::I  O:::::O     O:::::ON:::::::::::N   N::::::N
// C:::::C              O:::::O     O:::::ON:::::::N::::N  N::::::N  E::::::EEEEEEEEEE       X:::::X:::::X      I::::I  O:::::O     O:::::ON:::::::N::::N  N::::::N
// C:::::C              O:::::O     O:::::ON::::::N N::::N N::::::N  E:::::::::::::::E        X:::::::::X       I::::I  O:::::O     O:::::ON::::::N N::::N N::::::N
// C:::::C              O:::::O     O:::::ON::::::N  N::::N:::::::N  E:::::::::::::::E        X:::::::::X       I::::I  O:::::O     O:::::ON::::::N  N::::N:::::::N
// C:::::C              O:::::O     O:::::ON::::::N   N:::::::::::N  E::::::EEEEEEEEEE       X:::::X:::::X      I::::I  O:::::O     O:::::ON::::::N   N:::::::::::N
// C:::::C              O:::::O     O:::::ON::::::N    N::::::::::N  E:::::E                X:::::X X:::::X     I::::I  O:::::O     O:::::ON::::::N    N::::::::::N
//  C:::::C       CCCCCCO::::::O   O::::::ON::::::N     N:::::::::N  E:::::E       EEEEEEXXX:::::X   X:::::XXX  I::::I  O::::::O   O::::::ON::::::N     N:::::::::N
//   C:::::CCCCCCCC::::CO:::::::OOO:::::::ON::::::N      N::::::::NEE::::::EEEEEEEE:::::EX::::::X     X::::::XII::::::IIO:::::::OOO:::::::ON::::::N      N::::::::N
//    CC:::::::::::::::C OO:::::::::::::OO N::::::N       N:::::::NE::::::::::::::::::::EX:::::X       X:::::XI::::::::I OO:::::::::::::OO N::::::N       N:::::::N
//      CCC::::::::::::C   OO:::::::::OO   N::::::N        N::::::NE::::::::::::::::::::EX:::::X       X:::::XI::::::::I   OO:::::::::OO   N::::::N        N::::::N
//         CCCCCCCCCCCCC     OOOOOOOOO     NNNNNNNN         NNNNNNNEEEEEEEEEEEEEEEEEEEEEEXXXXXXX       XXXXXXXIIIIIIIIII     OOOOOOOOO     NNNNNNNN         NNNNNNN

void proceso_handler(void* args) {
	log_debug(logger,"Se inicia el servidor multi-hilo");
	args_escuchar* p = malloc(sizeof(args_escuchar));
	p = args;
	int socket_escucha = p->socket_oyente;

	int addrlen, socket_especifico;
	struct sockaddr_in address;

    addrlen = sizeof(address);

	if (listen(socket_escucha, LIMIT_CONNECTIONS) == -1)
		log_error(logger,"Error al configurar recepcion de mensajes");

	while (1) {
		if ((socket_especifico = accept(socket_escucha, (struct sockaddr*) &address, (socklen_t *) &addrlen)) > 0) {
			log_info(logger, "Se conecta un nuevo cliente");

			hilo_tripulante* parametros = malloc(sizeof(hilo_tripulante));

			parametros->socket = socket_especifico;
			pthread_t un_hilo_tripulante;

			pthread_create(&un_hilo_tripulante, NULL, (void*) atender_clientes, (void *) parametros);

			pthread_detach(un_hilo_tripulante);
		}
	}
}

void atender_clientes(void* param) {
	hilo_tripulante* parametros = param;

	int flag = 1;
	log_info(logger, "Atendiendo. %i\n", parametros->socket);

	while(flag) {
		t_estructura* mensaje_recibido = recepcion_y_deserializacion(parametros->socket);
		switch(mensaje_recibido->codigo_operacion) {

			case ARCHIVO_TAREAS:
				log_info(logger, "Recibido contenido del archivo\n");
				if(gestionar_tareas(mensaje_recibido->archivo_tareas)){
					enviar_codigo(EXITO, parametros->socket);
				} else{
					enviar_codigo(FALLO, parametros->socket);
				}
				////free(mensaje_recibido);
				break;

			case PEDIR_TAREA:
				log_info(logger, "Pedido de tarea recibido, tid: %i\n", mensaje_recibido->tid);
				t_tarea* una_tarea = buscar_siguiente_tarea(mensaje_recibido->tid);
				if(una_tarea != NULL){
					t_buffer* buffer_tarea = serializar_tarea(*una_tarea);
					empaquetar_y_enviar(buffer_tarea, TAREA, parametros->socket);
				}else{
					// esto puede ser por algun fallo o porque ya no queden tareas
					enviar_codigo(FALLO, parametros->socket);
				}
				//free(mensaje_recibido);
				break;

			case RECIBIR_TCB:
				if(gestionar_tcb(mensaje_recibido->tcb)){
					enviar_codigo(EXITO, parametros->socket);
				} else{
					enviar_codigo(FALLO, parametros->socket);
				}
				//free(mensaje_recibido);
				break;

			case ACTUALIZAR:
				if(actualizar_tcb(mensaje_recibido->tcb)){
					if(mensaje_recibido->tcb->estado_tripulante != 'F'){
						log_info(logger, "Actualizado el TCB %i.", mensaje_recibido->tcb->TID);
					}
				} else{
					log_error(logger, "No se pudo actualizar el TCB %i.", mensaje_recibido->tcb->TID);
				}
				//free(mensaje_recibido);
				break;

			case T_SIGKILL:
				if(eliminar_tcb(mensaje_recibido->tid)){
					enviar_codigo(EXITO, parametros->socket);
					log_info(logger, "%i -KILLED", mensaje_recibido->tid);
				}
				else{
					log_warning(logger, "No se pudo eliminar a %i", mensaje_recibido->tid);
					enviar_codigo(FALLO, parametros->socket);
				}
				//free(mensaje_recibido);
				break;

			case LISTAR_POR_PID:
				log_info(logger, "Recibido pedido de tripulantes.\n");
				t_list* tcbs_de_esta_patota = buscar_tcbs_por_pid(mensaje_recibido->pid);
				if(tcbs_de_esta_patota != NULL){
					for(int i = 0; i < list_size(tcbs_de_esta_patota); i++){
						t_TCB* aux = list_get(tcbs_de_esta_patota, i);
						t_buffer* buffer = serializar_tcb(*aux);
						empaquetar_y_enviar(buffer, RECIBIR_TCB, parametros->socket);
						desbloquear_segmento_por_tid(aux->TID);
					}
				}
				// PAGINACION: hay que liberar esta maldita lista (se chequea si es pag en la func.)
				liberar_lista_tcbs_paginacion(tcbs_de_esta_patota);
				enviar_codigo(EXITO, parametros->socket);
				//free(mensaje_recibido);
				break;

			case DESCONEXION:
				log_info(logger, "Se desconecto un cliente.\n");
				flag = 0;
				//free(mensaje_recibido);
				// close(parametros->socket);
				break;

			default:
				log_info(logger, "Se recibio un codigo invalido.");
				log_info(logger, "El codigo es %d", mensaje_recibido->codigo_operacion);
				//free(mensaje_recibido);
				break;
		}
	}

}

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

void iniciar_memoria(){
	memoria_principal = malloc(TAMANIO_MEMORIA);
	tablas = dictionary_create();
	if(strcmp(ESQUEMA_MEMORIA,"SEGMENTACION")==0){

		log_debug(logger,"[SEG]: Se inicia memoria con esquema se SEGMENTACION");
		segmentos = list_create();
		segmento* segmento_principal = crear_segmento(0,TAMANIO_MEMORIA,true);
		list_add(segmentos,segmento_principal);

	}else if(strcmp(ESQUEMA_MEMORIA,"PAGINACION")==0){

		log_debug(logger,"Se inicia memoria con esquema de PAGINACION");
		marcos = list_create();
		marco_clock = 0;
		int cantidad_marcos = TAMANIO_MEMORIA/TAMANIO_PAGINA;
		
		for(int i=0; i < cantidad_marcos ; i++) {
			marco* marco = crear_marco(TAMANIO_PAGINA * i, true);

			list_add(marcos,marco);
		}

		//disco = fopen(PATH_SWAP, "w+b");
		disco = fopen("swapFile.bin", "w+b");
		marcos_disco_size = TAMANIO_SWAP / TAMANIO_PAGINA;
		bitmap_disco = malloc(sizeof(bool) * marcos_disco_size);
		for(int i = 0; i < marcos_disco_size; i++){
			bitmap_disco[i] = false;
		}
	}else{
		log_error(logger,"Esquema de memoria desconocido");
		exit(EXIT_FAILURE);
	}
}

void* buscar_tabla(int pid){
	bloquear_lista_tablas();
	char spid[4];
	sprintf(spid, "%d", pid);
	void* tabla = dictionary_get(tablas,spid);
	desbloquear_lista_tablas();
	return tabla;
}

int gestionar_tareas(t_archivo_tareas* archivo){
	int pid = archivo->pid;
	int tamanio_tareas = archivo->largo_texto * sizeof(char) + 1;

	if(strcmp(ESQUEMA_MEMORIA, "SEGMENTACION") == 0){
		tabla_segmentos* tabla = crear_tabla_segmentos(pid);
		bloquear_tabla(tabla);
		//Creamos segmento para tareas y lo guardamos en la tabla de la patota
		log_info(logger, "[SEG]: Creando segmento de tareas, PID: %d", pid);
		segmento* segmento_tareas = asignar_segmento(tamanio_tareas);
		if(segmento_tareas == NULL){
			matar_tabla_segmentos(pid);
			return 0;
		}
		bloquear_segmento(segmento_tareas);
		segmento_tareas->tipo = S_TAREAS;
		void* puntero_a_tareas = memoria_principal + segmento_tareas->base;
		memcpy(puntero_a_tareas, archivo->texto, tamanio_tareas);
		tabla->segmento_tareas = segmento_tareas;
		desbloquear_segmento(segmento_tareas);

		//Creamos el PCB y un segmento para guardarlo
		t_PCB* pcb = malloc(sizeof(t_PCB));
		pcb->PID = pid;
		pcb->direccion_tareas = (uint32_t) puntero_a_tareas;

		log_info(logger, "[SEG]: Creando segmento para el PCB con PID: %d", pid);
		segmento* segmento_pcb = asignar_segmento(sizeof(t_PCB));
		if(segmento_pcb == NULL){
			matar_tabla_segmentos(pid);
			return 0;
		}
		
		bloquear_segmento(segmento_pcb);
		segmento_pcb->tipo = S_PCB;
		memcpy(memoria_principal + segmento_pcb->base, pcb, sizeof(t_PCB));
		tabla->segmento_pcb = segmento_pcb;
		desbloquear_segmento(segmento_pcb);
		desbloquear_tabla(tabla);
		free(pcb);
		return 1;
	}else if(strcmp(ESQUEMA_MEMORIA, "PAGINACION") == 0){
		tabla_paginas* tabla = (tabla_paginas*) buscar_tabla(pid);
		if(tabla == NULL){ 
			tabla = crear_tabla_paginas(pid);
		}
		log_info(logger, "Guardando tareas con PID: %d", pid);
		int dl_tareas = agregar_paginas_segun_tamano(tabla, (void*) archivo->texto, tamanio_tareas, pid);
		//log_error(logger, "TAREAS: %s", archivo->texto);
		if(dl_tareas == 99999){
			matar_tabla_paginas(pid);
			return 0;
		}
		tabla->dl_tareas = dl_tareas;
		log_info(logger, "Se terminó de guardar las tareas de pid: %d, dirección lógica: %d", pid, dl_tareas);

		log_info(logger, "Guardando PCB con PID: %d", pid);
		t_PCB* pcb = malloc(sizeof(t_PCB));
		pcb->PID = pid;
		pcb->direccion_tareas = dl_tareas;
		int dl_pcb = agregar_paginas_segun_tamano(tabla, (void*) pcb, sizeof(t_PCB), pid);
		if(dl_pcb == 99999){
			matar_tabla_paginas(pid);
			return 0;
		}
		tabla->dl_pcb = dl_pcb;
		log_info(logger, "Se terminó la creación del PCB de pid: %d, direccion lógica: %d", pid, dl_pcb);
		
		free(pcb);

		return 1;
	}else{
		log_error(logger, "Esquema de memoria desconocido");
		exit(EXIT_FAILURE);
	}
}

int gestionar_tcb(t_TCB* tcb){
	int pid = tcb->TID / 10000;
	size_t tamanio_tcb = sizeof(t_TCB);

	if(strcmp(ESQUEMA_MEMORIA, "SEGMENTACION") == 0){
		tabla_segmentos* tabla = (tabla_segmentos*) buscar_tabla(pid);
		if(tabla == NULL){ 
			tabla = crear_tabla_segmentos(pid);
		}
		
		//Creamos segmento para el tcb y lo guardamos en la tabla de la patota
		log_info(logger, "[SEG]: Creando segmento para TCB TID: %d", tcb->TID);
		segmento* segmento_tcb = asignar_segmento(tamanio_tcb);
		if(segmento_tcb == NULL){
			// si no hay mas memoria se mata toda la patota
			matar_tabla_segmentos(pid);
			return 0;
		}
		bloquear_segmento(segmento_tcb);
		segmento_tcb->tipo = S_TCB;
			// direccion donde está guardado el pcb
		void* puntero_a_pcb = memoria_principal + tabla->segmento_pcb->base; 
		tcb->puntero_a_pcb = (uint32_t) puntero_a_pcb;
			// direccion donde está guardada la string de tareas, como estoy creando el tcb, la siguiente tarea va a ser la primera
		void* puntero_a_tareas = memoria_principal + tabla->segmento_tareas->base; 
		tcb->siguiente_instruccion = (uint32_t) puntero_a_tareas;
		memcpy(memoria_principal + segmento_tcb->base, tcb, sizeof(t_TCB));
		desbloquear_segmento(segmento_tcb);

		
		list_add(tabla->segmentos_tcb, segmento_tcb);
		
	}else if(strcmp(ESQUEMA_MEMORIA, "PAGINACION") == 0){
		tabla_paginas* tabla = (tabla_paginas*) buscar_tabla(pid);
		if(tabla == NULL){ 
			// esto no tendria que pasar, pero por las dudas
			log_error(logger,"La tabla no existe pid: %d",pid);
			return 0;
		}

		log_info(logger, "[PAG]: Guardando TCB TID: %d", tcb->TID);
		tcb->siguiente_instruccion = 0;
		tcb->puntero_a_pcb = tabla->dl_pcb;
		int dl_tcb = agregar_paginas_segun_tamano(tabla, (void*) tcb, sizeof(t_TCB), pid);
		if(dl_tcb == 99999){
			matar_tabla_paginas(pid);
			return 0;
		}
		char stid[8];
		sprintf(stid,"%d", tcb->TID);
		dictionary_put(tabla->dl_tcbs, stid, (void*) dl_tcb);
	}else{
		log_error(logger, "Esquema de memoria desconocido");
		exit(EXIT_FAILURE);
	}
	mapa_iniciar_tcb(tcb);
	return 1;
}

t_TCB* buscar_tcb_por_tid(int tid){
	int pid = tid / 10000;
	t_TCB* tcb_recuperado;
	log_debug(logger,"Buscando TCB de TID: %d", tid);
	if(strcmp(ESQUEMA_MEMORIA,"SEGMENTACION")==0){
		
		tabla_segmentos* tabla = (tabla_segmentos*) buscar_tabla(pid);
		if(tabla == NULL){
			return NULL;
		}

		bool buscador(void* un_segmento){
			segmento* seg_tcb = (segmento*) un_segmento;
			t_TCB* tcb = memoria_principal + seg_tcb->base;
			return tcb->TID == tid;
		}
		segmento* segmento_tcb = list_find(tabla->segmentos_tcb, buscador);
		if(segmento_tcb == NULL){
			return NULL;
		}
		//hay que desbloquearlo despues
		bloquear_segmento(segmento_tcb);
		tcb_recuperado = memoria_principal + segmento_tcb->base;
	
	}else if(strcmp(ESQUEMA_MEMORIA,"PAGINACION")==0){
		tabla_paginas* tabla = (tabla_paginas*) buscar_tabla(pid);
		if(tabla == NULL){
			return NULL;
		}
		char stid[8];
		sprintf(stid, "%d", tid);
		tcb_recuperado = (t_TCB*) rescatar_de_paginas(tabla, (int) dictionary_get(tabla->dl_tcbs,stid), sizeof(t_TCB), pid);
		if(tcb_recuperado == NULL){
			return NULL;
		}
	}else{
		log_error(logger,"Esquema de memoria desconocido");
		exit(EXIT_FAILURE);
	}
	return tcb_recuperado;
}

t_list* buscar_tcbs_por_pid(int pid){
	log_debug(logger, "Buscando TCBs por pid: %d",pid);
	if(strcmp(ESQUEMA_MEMORIA,"SEGMENTACION")==0){

		tabla_segmentos* tabla = (tabla_segmentos*) buscar_tabla(pid);
		if(tabla == NULL){
			return NULL;
		}

		void* transformer(void* un_segmento){
			segmento* segmento_tcb = (segmento*) un_segmento;
			bloquear_segmento(segmento_tcb);
			//hay que desbloquearlo despues
			return memoria_principal + segmento_tcb->base;
		}
		return list_map(tabla->segmentos_tcb, transformer);

	}else if(strcmp(ESQUEMA_MEMORIA,"PAGINACION")==0){
		tabla_paginas* tabla = buscar_tabla(pid);
		if(tabla == NULL){
			return NULL; // tabla no encontrada, no debería pasar pero por las dudas viste
		}
		t_list* lista_tcbs = list_create();
		//hay que liberarla despues
		void tcb_finder(char* stid, void* una_dl){
			int dl = (int) una_dl;
			t_TCB* tcb = rescatar_de_paginas(tabla,dl,sizeof(t_TCB), pid);
			list_add(lista_tcbs, tcb);
		}
		
		dictionary_iterator(tabla->dl_tcbs, tcb_finder);
		
		return lista_tcbs;
	}else{
		log_error(logger,"Esquema de memoria desconocido");
		exit(EXIT_FAILURE);
	}
}

t_tarea* buscar_siguiente_tarea(int tid){
	int pid = tid / 10000;
	t_tarea* tarea = NULL;
	log_debug(logger, "Buscando tarea para tripulante, TID: %d", tid);
	if(strcmp(ESQUEMA_MEMORIA, "SEGMENTACION") == 0){
		
		tabla_segmentos* tabla = buscar_tabla(pid);
		if(tabla == NULL){
			return NULL; // tabla no encontrada, no debería pasar pero por las dudas viste
		}
		bloquear_segmento(tabla->segmento_tareas);
		t_TCB* tcb = buscar_tcb_por_tid(tid);
		if(tcb == NULL){
			// tcb no encontrado, no debería pasar pero por las dudas viste
			desbloquear_segmento(tabla->segmento_tareas);
			return NULL;
		}
		char* puntero_a_tareas = (char*) tcb->siguiente_instruccion;
		
		if(puntero_a_tareas == NULL){
			// Ya no quedan tareas
			log_warning(logger, "Ya no quedan tareas para el tripulante %d", tcb->TID);
			desbloquear_segmento(tabla->segmento_tareas);
			return NULL;
		}
		char** palabras = string_split(puntero_a_tareas, "\n");
		char* str_tarea = palabras[0];
		log_info(logger, "Tarea para TID: %d encontrada: %s", tid, str_tarea);

		//se crea la struct de tarea para devolver, despues hay que mandarle free
		tarea = crear_tarea(str_tarea);		
		if(palabras[1] == NULL){
			// no hay proxima tarea
			tcb->siguiente_instruccion = (uint32_t) NULL;
		}else{
			tcb->siguiente_instruccion += strlen(str_tarea) + 1; // +1 por el \n
		}
		liberar_puntero_doble(palabras);

		desbloquear_segmento_por_tid(tid);
		desbloquear_segmento(tabla->segmento_tareas);
		return tarea;
			
	}else if(strcmp(ESQUEMA_MEMORIA, "PAGINACION") == 0){
		tabla_paginas* tabla = buscar_tabla(pid);
		if(tabla == NULL){
			return NULL; // tabla no encontrada, no debería pasar pero por las dudas viste
		}
		t_TCB* tcb = buscar_tcb_por_tid(tid);
		if(tcb == NULL){
			//no deberia pasar pero por las dudas viste
			return NULL;
		}
		int dl_tarea_tcb = tcb->siguiente_instruccion;

		if(dl_tarea_tcb == 999999){
			// Ya no quedan tareas
			log_warning(logger, "Ya no quedan tareas para el tripulante %d", tid);
			return NULL;
		}
		char* str_tareas = rescatar_de_paginas(tabla, dl_tarea_tcb, tabla->dl_pcb - dl_tarea_tcb, pid); // 0 porque las tareas siempre estan al inicio de todo
		//char* tareas_restantes = string_substring_from(str_tareas, dl_tarea_tcb);
		char** palabras = string_split(str_tareas, "\n");
		char* str_tarea = palabras[0];
		log_info(logger, "Tarea para TID: %d encontrada: %s", tid, str_tarea);
		tarea = crear_tarea(str_tarea);

		if(palabras[1] == NULL){
			// no hay proxima tarea
			tcb->siguiente_instruccion = 999999; // como es que NULL = 0 en este lenguaje choto
		}else{
			tcb->siguiente_instruccion += strlen(str_tarea) + 1; // +1 por el \n
		}

		char stid[8];
		sprintf(stid, "%d", tid);
		int dl_tcb = (int) dictionary_get(tabla->dl_tcbs, stid);
		//log_info(logger,"Actualizando TCB");
		sobreescribir_paginas(tabla, (void*) tcb, dl_tcb, sizeof(t_TCB), pid);

		liberar_puntero_doble(palabras);
		//free(tareas_restantes);
		free(str_tareas);
		free(tcb);
		return tarea;
	}else{
		log_error(logger, "Esquema de memoria desconocido");
		exit(EXIT_FAILURE);
		return NULL;
	}
}

int eliminar_tcb(int tid){ // devuelve 1 si ta ok, 0 si falló algo
	log_info(logger,"Eliminando TCB TID: %d", tid);
	int pid = tid / 10000;
	if(strcmp(ESQUEMA_MEMORIA, "SEGMENTACION") == 0){
		tabla_segmentos* tabla = (tabla_segmentos*) buscar_tabla(pid);
		if(tabla == NULL){
			return 0; // tabla no encontrada, no debería pasar pero por las dudas viste
		}
		bloquear_tabla(tabla);

		bool buscador(void* un_segmento){
			segmento* seg_tcb = (segmento*) un_segmento;
			t_TCB* tcb = memoria_principal + seg_tcb->base;
			return tcb->TID == tid;
		}
		segmento* segmento_tcb = list_find(tabla->segmentos_tcb, buscador);
		if(segmento_tcb == NULL){
			log_error(logger, "TCB no encontrado TID: %d",tid);
			desbloquear_tabla(tabla);
			return 0;
		}
		matar_tcb_en_mapa(tid);
		segmento_tcb->libre = true;
		desbloquear_segmento(segmento_tcb);

		list_remove_by_condition(tabla->segmentos_tcb, buscador);
		log_debug(logger,"TCB eliminado TID: %d", tid);
		
		if(list_size(tabla->segmentos_tcb) < 1){
			matar_tabla_segmentos(pid);
		}
		desbloquear_tabla(tabla);
		return 1;
	}else if(strcmp(ESQUEMA_MEMORIA, "PAGINACION") == 0){
		tabla_paginas* tabla = (tabla_paginas*) buscar_tabla(pid);
		if(tabla == NULL){
			return 0; // tabla no encontrada, no debería pasar pero por las dudas viste
		}
		bloquear_tabla(tabla);
		if(dictionary_size(tabla->dl_tcbs) == 1){
			matar_tcb_en_mapa(tid);
			matar_tabla_paginas(pid);
			return 1;
		}else{
			int result = matar_paginas_tcb(tabla, tid);
			if(result){
				log_info(logger, "Se mató al TCB tid: %d", tid);
				matar_tcb_en_mapa(tid);
				desbloquear_tabla(tabla);
				return 1;
			}else{
				// error
				desbloquear_tabla(tabla);

				return 0;
			}
			desbloquear_tabla(tabla);
			return 0;
		}
		desbloquear_tabla(tabla);
	}else{
		log_error(logger, "Esquema de memoria desconocido");
		exit(EXIT_FAILURE);
	}
}

int actualizar_tcb(t_TCB* nuevo_tcb){
	log_debug(logger,"Actualizando TCB TID: %d", nuevo_tcb->TID);
	int pid = nuevo_tcb->TID / 10000;
	char stid[8];
	sprintf(stid, "%d",nuevo_tcb->TID);
	if(nuevo_tcb->estado_tripulante == 'F'){
		log_info(logger, "Tripulante en estado exit");
		eliminar_tcb(nuevo_tcb->TID);
		return 1;
	}
	
	if(strcmp(ESQUEMA_MEMORIA, "SEGMENTACION") == 0){
		t_TCB* tcb = buscar_tcb_por_tid(nuevo_tcb->TID);
		if(tcb == NULL){
			return 0;
		}
		tcb->coord_x = nuevo_tcb->coord_x;
		tcb->coord_y = nuevo_tcb->coord_y;
		tcb->estado_tripulante = nuevo_tcb->estado_tripulante;
		desbloquear_segmento_por_tid(tcb->TID);
	}else if(strcmp(ESQUEMA_MEMORIA, "PAGINACION") == 0){
		tabla_paginas* tabla = (tabla_paginas*) buscar_tabla(pid);
		if(tabla == NULL){
			return 0; // tabla no encontrada, no debería pasar pero por las dudas viste
		}
		// no me traigo el tcb actual sino sobreescribo directamente sus paginas
		int dl_tcb = (int) dictionary_get(tabla->dl_tcbs, stid);
		int result = sobreescribir_paginas(tabla, nuevo_tcb, dl_tcb, sizeof(uint32_t) * 3 + sizeof(char), pid);
		if(!result){
			return 0;
		}
	}else{
		log_error(logger, "Esquema de memoria desconocido");
		exit(EXIT_FAILURE);
	}
	actualizar_tcb_en_mapa(nuevo_tcb);
	
	return 1;
}

void signal_compactacion(int n){
	if(n == SIGUSR1){
		if(strcmp(ESQUEMA_MEMORIA, "SEGMENTACION") == 0){
			compactacion();
		}
	}
}

void dump(int n){
	if(n == SIGUSR2){
		log_debug(logger,"Se inicia el dump de memoria");
		if(strcmp(ESQUEMA_MEMORIA, "SEGMENTACION") == 0){
			dump_segmentacion();
		}else if(strcmp(ESQUEMA_MEMORIA, "PAGINACION") == 0){
			dump_paginacion();
		}else{
			log_error(logger, "Esquema de memoria desconocido");
			exit(EXIT_FAILURE);
		}
		log_debug(logger,"Se terminó el dump de memoria");
	}
}

// MMMMMMMM               MMMMMMMM               AAA               PPPPPPPPPPPPPPPPP                  AAA               
// M:::::::M             M:::::::M              A:::A              P::::::::::::::::P                A:::A              
// M::::::::M           M::::::::M             A:::::A             P::::::PPPPPP:::::P              A:::::A             
// M:::::::::M         M:::::::::M            A:::::::A            PP:::::P     P:::::P            A:::::::A            
// M::::::::::M       M::::::::::M           A:::::::::A             P::::P     P:::::P           A:::::::::A           
// M:::::::::::M     M:::::::::::M          A:::::A:::::A            P::::P     P:::::P          A:::::A:::::A          
// M:::::::M::::M   M::::M:::::::M         A:::::A A:::::A           P::::PPPPPP:::::P          A:::::A A:::::A         
// M::::::M M::::M M::::M M::::::M        A:::::A   A:::::A          P:::::::::::::PP          A:::::A   A:::::A        
// M::::::M  M::::M::::M  M::::::M       A:::::A     A:::::A         P::::PPPPPPPPP           A:::::A     A:::::A       
// M::::::M   M:::::::M   M::::::M      A:::::AAAAAAAAA:::::A        P::::P                  A:::::AAAAAAAAA:::::A      
// M::::::M    M:::::M    M::::::M     A:::::::::::::::::::::A       P::::P                 A:::::::::::::::::::::A     
// M::::::M     MMMMM     M::::::M    A:::::AAAAAAAAAAAAA:::::A      P::::P                A:::::AAAAAAAAAAAAA:::::A    
// M::::::M               M::::::M   A:::::A             A:::::A   PP::::::PP             A:::::A             A:::::A   
// M::::::M               M::::::M  A:::::A               A:::::A  P::::::::P            A:::::A               A:::::A  
// M::::::M               M::::::M A:::::A                 A:::::A P::::::::P           A:::::A                 A:::::A 
// MMMMMMMM               MMMMMMMMAAAAAAA                   AAAAAAAPPPPPPPPPP          AAAAAAA                   AAAAAAA


void iniciar_mapa(){
	if(mapa_on){
		nivel_gui_inicializar();
		nivel_gui_get_area_nivel(&cols, &rows);
		nivel = nivel_crear("AmongOS - NO MATAR A REY DE FUEGO");
		nivel_gui_dibujar(nivel);
		mapa_indices = dictionary_create();
		ultima_clave_mapa = 1;
	}
}

void mapa_iniciar_tcb(t_TCB* tcb){
	if(mapa_on){
		bloquear_mapa();
		personaje_crear(nivel, ultima_clave_mapa, tcb->coord_x, tcb->coord_y);
		nivel_gui_dibujar(nivel);

		char stid[8];
		sprintf(stid, "%d", tcb->TID);

		//char* clave = malloc(sizeof(char));
		//memcpy(clave, &ultima_clave_mapa, sizeof(char));
		log_trace(logger,"[MAPA]: Guardo map_key de TCB TID: %d, map_key: %c", tcb->TID, ultima_clave_mapa);
		dictionary_put(mapa_indices, stid, (void*) ultima_clave_mapa);

		ultima_clave_mapa++;
		desbloquear_mapa();
	}
}

char* get_clave_mapa_por_tid(int tid){
	if(mapa_on){
		char stid[8];
		sprintf(stid, "%d",tid);
		char* clave_mapa = (char*) dictionary_get(mapa_indices,stid);
		if(clave_mapa == NULL){
			log_error(logger,"[MAPA]: Error: clave en mapa no encontrada");
		}
		log_trace(logger,"[MAPA]: Encontre la clave %c", clave_mapa);
		return clave_mapa;
	}
	return NULL;
}

void matar_tcb_en_mapa(int tid){
	if(mapa_on){
		bloquear_mapa();
		char stid[8];
		sprintf(stid, "%d",tid);
		char clave_mapa = get_clave_mapa_por_tid(tid);
		if(clave_mapa != NULL){
			item_borrar(nivel, clave_mapa);
			nivel_gui_dibujar(nivel);
			log_trace(logger, "[MAPA]: Elimino TCB TID: %d", tid);
			dictionary_remove(mapa_indices, stid);
		}
		desbloquear_mapa();
	}
}

void actualizar_tcb_en_mapa(t_TCB* tcb){
	if(mapa_on){
		bloquear_mapa();
		char clave_mapa = get_clave_mapa_por_tid(tcb->TID);
		log_trace(logger, "[MAPA]: Muevo el TCB TID: %d,\t clave en mapa: %c, \t nueva pos: %d|%d",tcb->TID,clave_mapa,tcb->coord_x,tcb->coord_y);
		item_mover(nivel, clave_mapa, tcb->coord_x, tcb->coord_y);
		nivel_gui_dibujar(nivel);
		desbloquear_mapa();
	}
}

void matar_mapa(){
	if(mapa_on){
		nivel_destruir(nivel);
		nivel_gui_terminar();
	}
}
                                     
//                AAA               UUUUUUUU     UUUUUUUUXXXXXXX       XXXXXXX
//               A:::A              U::::::U     U::::::UX:::::X       X:::::X
//              A:::::A             U::::::U     U::::::UX:::::X       X:::::X
//             A:::::::A            UU:::::U     U:::::UUX::::::X     X::::::X
//            A:::::::::A            U:::::U     U:::::U XXX:::::X   X:::::XXX
//           A:::::A:::::A           U:::::D     D:::::U    X:::::X X:::::X   
//          A:::::A A:::::A          U:::::D     D:::::U     X:::::X:::::X    
//         A:::::A   A:::::A         U:::::D     D:::::U      X:::::::::X     
//        A:::::A     A:::::A        U:::::D     D:::::U      X:::::::::X     
//       A:::::AAAAAAAAA:::::A       U:::::D     D:::::U     X:::::X:::::X    
//      A:::::::::::::::::::::A      U:::::D     D:::::U    X:::::X X:::::X   
//     A:::::AAAAAAAAAAAAA:::::A     U::::::U   U::::::U XXX:::::X   X:::::XXX
//    A:::::A             A:::::A    U:::::::UUU:::::::U X::::::X     X::::::X
//   A:::::A               A:::::A    UU:::::::::::::UU  X:::::X       X:::::X
//  A:::::A                 A:::::A     UU:::::::::UU    X:::::X       X:::::X
// AAAAAAA                   AAAAAAA      UUUUUUUUU      XXXXXXX       XXXXXXX

int tamanio_tarea(t_tarea* tarea){
    int tam = sizeof(uint32_t) * 5;
    tam += tarea->largo_nombre * sizeof(char);
    return tam;
}

void bloquear_tabla(void* una_tabla){
	if(strcmp(ESQUEMA_MEMORIA, "SEGMENTACION") == 0){
		tabla_segmentos* tabla = (tabla_segmentos*) una_tabla;
		pthread_mutex_lock(&(tabla->mutex));
	}else if(strcmp(ESQUEMA_MEMORIA, "PAGINACION") == 0){
		tabla_paginas* tabla = (tabla_paginas*) una_tabla;
		pthread_mutex_lock(&(tabla->mutex));
	}
	log_trace(logger,"[SEM]: Bloqueo tabla");
}

void desbloquear_tabla(void* una_tabla){
	if(strcmp(ESQUEMA_MEMORIA, "SEGMENTACION") == 0){
		tabla_segmentos* tabla = (tabla_segmentos*) una_tabla;
		pthread_mutex_unlock(&(tabla->mutex));
	}else if(strcmp(ESQUEMA_MEMORIA, "PAGINACION") == 0){
		tabla_paginas* tabla = (tabla_paginas*) una_tabla;
		pthread_mutex_unlock(&(tabla->mutex));
	}
	log_trace(logger,"[SEM]: Desbloqueo tabla");
}

void bloquear_lista_tablas(){
	pthread_mutex_lock(&m_tablas);
	log_trace(logger,"[SEM]: Bloqueo lista de tablas");
}

void desbloquear_lista_tablas(){
	pthread_mutex_unlock(&m_tablas);
	log_trace(logger,"[SEM]: Desloqueo lista de tablas");
}

void bloquear_mapa(){
	pthread_mutex_lock(&m_mapa);
	log_trace(logger,"[SEM]: Bloqueo mapa");
}

void desbloquear_mapa(){
	pthread_mutex_unlock(&m_mapa);
	log_trace(logger,"[SEM]: Desloqueo mapa");
}
