/*
 * mi_ram_hq.c
 *
 *  Created on: 9 may. 2021
 *      Author: utnso
 */

#include "mi_ram_hq.h"
#include <comms/generales.h>

#define	IP config_get_string_value(config, "IP")
#define PUERTO config_get_string_value(config, "PUERTO")
#define TAMANIO_MEMORIA config_get_int_value(config, "TAMANIO_MEMORIA")
#define TAMANIO_PAGINA config_get_int_value(config, "TAMANIO_PAGINA")
#define ESQUEMA_MEMORIA config_get_string_value(config, "ESQUEMA_MEMORIA")
#define LIMIT_CONNECTIONS 10

void dump(int n){
	if(n == SIGUSR2){
		log_debug(logger,"Se inicia el dump de memoria");
		if(strcmp(ESQUEMA_MEMORIA, "SEGMENTACION") == 0){
			dump_segmentacion();
		}else if(strcmp(ESQUEMA_MEMORIA, "PAGINACION") == 0){

		}else{
			log_error(logger, "Esquema de memoria desconocido");
			exit(EXIT_FAILURE);
		}
		log_debug(logger,"Se terminó el dump de memoria");
	}
}

int main(int argc, char** argv) {
	
	// Reinicio el log
	FILE* f = fopen("mi_ram_hq.log", "w");
    fclose(f);
	// Inicializar
	logger = log_create("mi_ram_hq.log", "MI_RAM_HQ", 1, LOG_LEVEL_DEBUG);
	config = config_create("mi_ram_hq.config");
	signal(SIGUSR2,dump);

	iniciar_memoria();
	


	// tabla_paginas* tabla = crear_tabla_paginas(1);

	// marco* marco = list_get(marcos,0);

	// log_info(logger,"Se creo el marco");

	// pagina* pagina = crear_pagina(marco, 3);

	// list_add(tabla->paginas, pagina);
	// // agregar_paginas_segun_tamano(tabla, 100);

	// imprimir_paginas(1);
	// // //iniciar_mapa(); TODO dibujar mapa inicial vacio


	int socket_oyente = crear_socket_oyente(IP, PUERTO);
    	args_escuchar args_miram;
	args_miram.socket_oyente = socket_oyente;

	pthread_t hilo_escucha;
	pthread_create(&hilo_escucha, NULL, (void*) proceso_handler, (void*) &args_miram);

	//pthread_detach(hilo_escucha);
	pthread_join(hilo_escucha, NULL);
	close(socket_oyente);

	log_destroy(logger);
	config_destroy(config);

	return EXIT_SUCCESS;
}

void proceso_handler(void* args) {
	log_info(logger,"Se inicia el servidor multi-hilo");
	args_escuchar* p = malloc(sizeof(args_escuchar));
	p = args;
	int socket_escucha = p->socket_oyente;

	int addrlen, socket_especifico;
	struct sockaddr_in address;

    addrlen = sizeof(address);

	// struct sockaddr_storage direccion_a_escuchar;
	// socklen_t tamanio_direccion;

	if (listen(socket_escucha, LIMIT_CONNECTIONS) == -1)
		log_error(logger,"Error al configurar recepcion de mensajes");

	while (1) {
		if ((socket_especifico = accept(socket_escucha, (struct sockaddr*) &address, (socklen_t *) &addrlen)) > 0) {
			// Maté la verificación
			log_info(logger, "Se conecta un nuevo proceso");

			hilo_tripulante* parametros = malloc(sizeof(hilo_tripulante));

			parametros->socket = socket_especifico;
			//parametros->ip_cliente = inet_ntoa(address.sin_addr);
			//parametros->puerto_cliente = ntohs(address.sin_port);
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
		int result;
		//sleep(1); //para que no se rompa en casos de bug o tiempos de espera

		switch(mensaje_recibido->codigo_operacion) {

			case ARCHIVO_TAREAS:
				log_info(logger, "Recibido contenido del archivo\n");
				printf("\tpid:%i. \n\tlongitud; %i. \n%s\n", mensaje_recibido->archivo_tareas->pid, mensaje_recibido->archivo_tareas->largo_texto, mensaje_recibido->archivo_tareas->texto);

				if(gestionar_tareas(mensaje_recibido->archivo_tareas)){
					enviar_codigo(EXITO, parametros->socket);
				} else{
					enviar_codigo(FALLO, parametros->socket);
				}
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
				free(una_tarea);

				break;

			case RECIBIR_TCB:
				// log_info(logger, "Recibido tripulante %i, estado: %c, pos: %i %i, puntero_pcb: %i, sig_ins %i\n", (int) mensaje_recibido->tcb->TID, (char) mensaje_recibido->tcb->estado_tripulante, (int) mensaje_recibido->tcb->coord_x, (int) mensaje_recibido->tcb->coord_y, (int) mensaje_recibido->tcb->puntero_a_pcb, (int) mensaje_recibido->tcb->siguiente_instruccion);
				if(gestionar_tcb(mensaje_recibido->tcb)){
					enviar_codigo(EXITO, parametros->socket);
				} else{
					enviar_codigo(FALLO, parametros->socket);
				}
				break;

			case ACTUALIZAR:
				// log_info(logger, "Tripulante %i, estado: %c, pos: %i %i\n", (int) mensaje_recibido->tcb->TID, (char) mensaje_recibido->tcb->estado_tripulante, (int) mensaje_recibido->tcb->coord_x, (int) mensaje_recibido->tcb->coord_y);
				if(actualizar_tcb(mensaje_recibido->tcb)){
					log_info(logger, "Actualizado el TCB %i.", mensaje_recibido->tcb->TID);
				} else{
					log_error(logger, "No se pudo actualizar el TCB %i.", mensaje_recibido->tid);
				}
				break;

			case T_SIGKILL:
				log_info(logger, "Expulsar Tripulante.\n");

				if(eliminar_tcb(mensaje_recibido->tid)){
					enviar_codigo(EXITO, parametros->socket);
					log_info(logger, "%i -KILLED", mensaje_recibido->tid);
				}
				else{
					log_warning(logger, "No se pudo eliminar a %i");
					enviar_codigo(FALLO, parametros->socket);
				}

				break;

			case LISTAR_POR_PID:
				log_info(logger, "Recibido pedido de tripulantes.\n");

				t_list* tcbs_de_esta_patota = buscar_tcbs_por_pid(mensaje_recibido->pid);
				if(tcbs_de_esta_patota != NULL){
					for(int i = 0; i < list_size(tcbs_de_esta_patota); i++){
						t_TCB* aux = list_get(tcbs_de_esta_patota, i);
						t_buffer* buffer = serializar_tcb(*aux);
						empaquetar_y_enviar(buffer, RECIBIR_TCB, parametros->socket);
					}
				}

				enviar_codigo(EXITO, parametros->socket);

				break;

			case DESCONEXION:
				log_info(logger, "Se desconecto un cliente.\n");
				flag = 0;
				// close(parametros->socket);
				break;

			default:
				log_info(logger, "Se recibio un codigo invalido.\n");
				printf("El codigo es %d\n", mensaje_recibido->codigo_operacion);
				break;
		}
		// free(mensaje_recibido);
	}

}




void iniciar_memoria(){
	memoria_principal = malloc(TAMANIO_MEMORIA);
	tablas = dictionary_create();
	if(strcmp(ESQUEMA_MEMORIA,"SEGMENTACION")==0){
		log_debug(logger,"Se inicia memoria con esquema se SEGMENTACION");
		segmentos = list_create();
		segmento* segmento_principal = crear_segmento(0,TAMANIO_MEMORIA,true);
		list_add(segmentos,segmento_principal);
	}else if(strcmp(ESQUEMA_MEMORIA,"PAGINACION")==0){
		
		log_info(logger,"Se inicia memoria con esquema de PAGINACION");
		marcos = list_create();

		int cantidad_marcos = TAMANIO_MEMORIA/TAMANIO_PAGINA;
		
		for(int i=0; i < cantidad_marcos ; i++) {
			marco* marco = crear_marco(TAMANIO_PAGINA * i, true);

			list_add(marcos,marco);
		}

	}else{
		log_error(logger,"Esquema de memoria desconocido");
		exit(EXIT_FAILURE);
	}
}

void* buscar_tabla(int pid){	
	log_debug(logger,"Se comienza la busqueda de tabla de pid: %d",pid);
	char spid[4];
	sprintf(spid, "%d", pid);
	void* tabla = dictionary_get(tablas,spid);
	if(tabla == NULL){
		log_warning(logger,"Tabla no encontrada");
		return NULL;
	}
	log_debug(logger,"Tabla encontrada, pid: %d",pid);
	return tabla;
}

int gestionar_tareas(t_archivo_tareas* archivo){
	//char** string_tareas = string_split(archivo_tareas->texto, "\n");
	//int cantidad_tareas = contar_palabras(string_tareas);
	int pid_patota = archivo->pid;
	int tamanio_tareas = archivo->largo_texto * sizeof(char);

	if(strcmp(ESQUEMA_MEMORIA, "SEGMENTACION") == 0){
		tabla_segmentos* tabla = (tabla_segmentos*) buscar_tabla(pid_patota);
		if(tabla == NULL){ 
			tabla = crear_tabla_segmentos(pid_patota);
		}
		
		//Creamos segmento para tareas y lo guardamos en la tabla de la patota
		log_debug(logger, "Comienza la creación del segmento de tareas con PID: %d", pid_patota);
		segmento* segmento_tareas = asignar_segmento(tamanio_tareas);
		if(segmento_tareas == NULL){
			matar_tabla_segmentos(pid_patota);
			return 0;
		}
		segmento_tareas->tipo = S_TAREAS;
		void* puntero_a_tareas = memoria_principal + segmento_tareas->base;
		memcpy(puntero_a_tareas, archivo->texto, tamanio_tareas);
		tabla->segmento_tareas = segmento_tareas;
		log_debug(logger,"Se termino la creación del segmento de tareas con PID: %d, con base: %d", pid_patota, segmento_tareas->base);

		//Creamos el PCB
		log_debug(logger, "Comienza la creación de PCB con PID: %d", pid_patota);
		t_PCB* pcb = malloc(sizeof(t_PCB));
		pcb->PID = pid_patota;
		pcb->direccion_tareas = (uint32_t) puntero_a_tareas;
		log_debug(logger,"Se termino la creación de PCB con PID: %d", pid_patota);

		//Creamos el segmento para el PCB y lo guardamos en la tabla de la patota
		log_debug(logger, "Comienza la creación del segmento para el PCB con PID: %d", pid_patota);
		segmento* segmento_pcb = asignar_segmento(sizeof(t_PCB));
		if(segmento_pcb == NULL){
			matar_tabla_segmentos(pid_patota);
			return 0;
		}
		segmento_pcb->tipo = S_PCB;
		memcpy(memoria_principal + segmento_pcb->base, pcb, sizeof(t_PCB));
		tabla->segmento_pcb = segmento_pcb;
		log_debug(logger, "Se terminó la creación del segmento para el PCB con PID: %d, con base: %d", pid_patota,segmento_pcb->base);
		free(pcb);
		return 1;
	}else if(strcmp(ESQUEMA_MEMORIA, "PAGINACION") == 0){
		tabla_paginas* tabla = (tabla_paginas*) buscar_tabla(pid_patota);
		if(tabla == NULL){ 
			tabla = crear_tabla_paginas(pid_patota);
		}else{
			log_error(logger,"La tabla ya existía pid: %d",pid_patota);
			return 0;
		}
		int dl_tareas = agregar_paginas_segun_tamano(tabla, archivo->texto, tamanio_tareas);

		t_PCB* pcb = malloc(sizeof(t_PCB));
		pcb->PID = pid_patota;
		pcb->direccion_tareas = dl_tareas;
		int dl_pcb = agregar_paginas_segun_tamano(tabla, pcb, sizeof(t_PCB));
		tabla->dl_tareas = dl_tareas;
		tabla->dl_pcb = dl_pcb;
		return 1;
	}else{
		log_error(logger, "Esquema de memoria desconocido");
		exit(EXIT_FAILURE);
	}
}

int gestionar_tcb(t_TCB* tcb){
	int pid = tcb->TID / 10000;
	size_t tamanio_tcb = sizeof(t_TCB);
	log_debug(logger, "Comienza la creación del TCB, TID: %d", tcb->TID);
	if(strcmp(ESQUEMA_MEMORIA, "SEGMENTACION") == 0){
		tabla_segmentos* tabla = (tabla_segmentos*) buscar_tabla(pid);
		if(tabla == NULL){ 
			tabla = crear_tabla_segmentos(pid);
		}
		
		//Creamos segmento para el tcb y lo guardamos en la tabla de la patota
		segmento* segmento_tcb = asignar_segmento(tamanio_tcb);
		if(segmento_tcb == NULL){
			matar_tabla_segmentos(pid);
			return 0;
		}
		segmento_tcb->tipo = S_TCB;
			// direccion donde está guardado el pcb
		void* puntero_a_pcb = memoria_principal + tabla->segmento_pcb->base; 
		tcb->puntero_a_pcb = (uint32_t) puntero_a_pcb;
			// direccion donde está guardada la string de tareas, como estoy creando el tcb, la siguiente tarea va a ser la primera
		void* puntero_a_tareas = memoria_principal + tabla->segmento_tareas->base; 
		tcb->siguiente_instruccion = (uint32_t) puntero_a_tareas;

		memcpy(memoria_principal + segmento_tcb->base, tcb, sizeof(t_TCB));

		list_add(tabla->segmentos_tcb, segmento_tcb);

	}else if(strcmp(ESQUEMA_MEMORIA, "PAGINACION") == 0){
		//TODO Guardar tcb en tabla de paginas
		return 0;
	}else{
		log_error(logger, "Esquema de memoria desconocido");
		exit(EXIT_FAILURE);
	}
	log_debug(logger,"Se termino la creación de TCB, TID: %d", tcb->TID);
	return 1;
}

t_TCB* buscar_tcb_por_tid(int tid){
	int pid = tid / 10000;
	t_TCB* tcb_recuperado;
	log_debug(logger,"Comienza la busqueda de TCB por TID: %d", tid);
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
			log_warning(logger,"TCB con TID: %d no encontrado", tid);
			return NULL;
		}
		tcb_recuperado = memoria_principal + segmento_tcb->base;

		if(tcb_recuperado == NULL){
			log_warning(logger,"TCB con TID: %d no encontrado", tid);
			return NULL;
		}

		log_debug(logger,"TCB con TID: %d encontrado", tid);
		
	}else if(strcmp(ESQUEMA_MEMORIA,"PAGINACION")==0){
		
	}else{
		log_error(logger,"Esquema de memoria desconocido");
		exit(EXIT_FAILURE);
	}
	return tcb_recuperado;
}

t_list* buscar_tcbs_por_pid(int pid){
	log_debug(logger, "Comienza la busqueda de todos los TCB por pid: %d",pid);
	if(strcmp(ESQUEMA_MEMORIA,"SEGMENTACION")==0){

		tabla_segmentos* tabla = (tabla_segmentos*) buscar_tabla(pid);
		if(tabla == NULL){
			return NULL;
		}

		void* transformer(void* un_segmento){
			segmento* segmento_tcb = (segmento*) un_segmento;
			return memoria_principal + segmento_tcb->base;
		}
		log_debug(logger,"Encontrados TCBs de la patota con pid: %d", pid);
		return list_map(tabla->segmentos_tcb, transformer);

	}else if(strcmp(ESQUEMA_MEMORIA,"PAGINACION")==0){
		return NULL;
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

		t_TCB* tcb = buscar_tcb_por_tid(tid);
		if(tcb == NULL){
			// tcb no encontrado, no debería pasar pero por las dudas viste
			return NULL;
		}
		char* puntero_a_tareas = (char*) tcb->siguiente_instruccion;
		
		if(puntero_a_tareas == NULL){
			// Ya no quedan tareas
			log_warning(logger, "Ya no quedan tareas para el tripulante %d", tcb->TID);
			return NULL;
		}
		char** palabras = string_split(puntero_a_tareas, "\n");

		char* str_tarea = palabras[0];
		log_info(logger, "Tarea: %s", str_tarea);

		//se crea la struct de tarea para devolver, despues hay que mandarle free
		tarea = crear_tarea(str_tarea);
		//me fijo si hay un \0 despues de la tarea, si no hay significa que esta era la ultima :'(
		log_error(logger,"Largo: %d", strlen(str_tarea));
		
		if(palabras[1] == NULL){
			// no hay proxima tarea
			tcb->siguiente_instruccion = (uint32_t) NULL;
		}else{
			tcb->siguiente_instruccion += strlen(str_tarea) + 1; // +1 por el \n
		}
		liberar_puntero_doble(palabras);
		log_debug(logger,"Se encontro la tarea para el tripulante %d",tid);
		return tarea;
			
	}else if(strcmp(ESQUEMA_MEMORIA, "PAGINACION") == 0){
		return NULL;
	}else{
		log_error(logger, "Esquema de memoria desconocido");
		exit(EXIT_FAILURE);
	}
	if(tarea == NULL){
		log_warning(logger,"No se encontro la tarea para el tripulante %d",tid);
		return NULL;
	}
}

int eliminar_tcb(int tid){ // devuelve 1 si ta ok, 0 si falló algo
	log_debug(logger,"Comenzó el sacrificio del TCB TID: %d", tid);
	int pid = tid / 10000;
	if(strcmp(ESQUEMA_MEMORIA, "SEGMENTACION") == 0){
		tabla_segmentos* tabla = buscar_tabla(pid);
		if(tabla == NULL){
			return 0; // tabla no encontrada, no debería pasar pero por las dudas viste
		}

		bool buscador(void* un_segmento){
			segmento* seg_tcb = (segmento*) un_segmento;
			t_TCB* tcb = memoria_principal + seg_tcb->base;
			return tcb->TID == tid;
		}
		segmento* segmento_tcb = list_find(tabla->segmentos_tcb, buscador);
		if(segmento_tcb == NULL){
			log_error(logger, "TCB TID: %d logró escapar, (no se encontró)",tid);
			return 0;
		}
		liberar_segmento(segmento_tcb->base);

		list_remove_by_condition(tabla->segmentos_tcb, buscador);
		log_debug(logger,"TID: %d ahora descansa con Odín", tid);

		if(list_size(tabla->segmentos_tcb) < 1){
			// Si era el ultimo tripulante: MUERTE A LA TABLA ENTERA
			matar_tabla_segmentos(pid);
		}
		return 1;
	}else if(strcmp(ESQUEMA_MEMORIA, "PAGINACION") == 0){
		return 0;
	}else{
		log_error(logger, "Esquema de memoria desconocido");
		exit(EXIT_FAILURE);
	}
}

int actualizar_tcb(t_TCB* nuevo_tcb){
	log_debug(logger,"Actualizando TCB TID: %d", nuevo_tcb->TID);
	t_TCB* tcb = buscar_tcb_por_tid(nuevo_tcb->TID);
	if(tcb == NULL){
		return 0;
	}
	if(nuevo_tcb->estado_tripulante == 'F'){
		eliminar_tcb(nuevo_tcb->TID);
		return 1;
	}
	tcb->coord_x = nuevo_tcb->coord_x;
	tcb->coord_y = nuevo_tcb->coord_y;
	tcb->estado_tripulante = nuevo_tcb->estado_tripulante;
	log_debug(logger,"Actualizado TCB TID: %d", tcb->TID);
	return 1;
}
