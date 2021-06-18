/*
 * mi_ram_hq.c
 *
 *  Created on: 9 may. 2021
 *      Author: utnso
 */

// TODO: por alguna razon, no funcionan los printf en este modulo.

#include "mi_ram_hq.h"
#include <comms/generales.h>


#define	IP config_get_string_value(config, "IP")
#define PUERTO config_get_string_value(config, "PUERTO")
#define TAMANIO_MEMORIA config_get_int_value(config, "TAMANIO_MEMORIA")
#define ESQUEMA_MEMORIA config_get_string_value(config, "ESQUEMA_MEMORIA")
#define CRITERIO_SELECCION config_get_string_value(config, "CRITERIO_SELECCION")

t_log* logger;
t_config* config;

t_list* lista_tcb;
t_list* lista_pcb;	

typedef struct hilo_tripulante{
	int socket;
	char* ip_cliente;
	char* puerto_cliente;
	void (*atender)(char*);
} hilo_tripulante;

char estado_tripulante[4] = {'N', 'R', 'E', 'B'};

int main(int argc, char** argv) {
	
	// Reinicio el log
	FILE* f = fopen("mi_ram_hq.log", "w");
    fclose(f);

	// Inicializar
	logger = log_create("mi_ram_hq.log", "MI_RAM_HQ", 1, LOG_LEVEL_DEBUG);
	config = config_create("mi_ram_hq.config");

	lista_tcb = list_create();
	lista_pcb = list_create();

	iniciar_memoria();

	segmento* seg = asignar_segmento(sizeof(int));
	segmento* seg2 = asignar_segmento(sizeof(char));
	segmento* seg3 = asignar_segmento(sizeof(char[10]));
	segmento* seg4 = asignar_segmento(sizeof(t_TCB));
	printSegmentosList();

    //int socket_oyente = crear_socket_oyente(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
    //escuchar_miram((void*) socket_oyente);//debug
    
	int socket_oyente = crear_socket_oyente(IP, PUERTO);

  	args_escuchar args_miram;
	args_miram.socket_oyente = socket_oyente;

	pthread_t hilo_escucha;
	pthread_create(&hilo_escucha, NULL, (void*) proceso_handler, (void*) &args_miram);
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
	//int socket_escucha = (int) args; //Tema de testeos, no borrar

    int addrlen, socket_especifico;
    struct sockaddr_in address;

    addrlen = sizeof(address);

	// struct sockaddr_storage direccion_a_escuchar;
	// socklen_t tamanio_direccion;

	if (listen(socket_escucha, 10) == -1) //TODO esto esta hardcodeado
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




void atender_clientes(void* param) { // TODO miram no termina ni siquiera si muere discordiador. una forma de arreglarlo es hacer que estas funciones devuelvan valores.
	hilo_tripulante* parametros = param;

	int flag = 1;
	log_info(logger, "Atendiendo. %i\n", parametros->socket);

	while(flag) {
		t_estructura* mensaje_recibido = recepcion_y_deserializacion(parametros->socket);

		sleep(1); //para que no se rompa en casos de bug o tiempos de espera

		switch(mensaje_recibido->codigo_operacion) {

			case MENSAJE:
				log_info(logger, "Mensaje recibido\n");
				break;

			case PEDIR_TAREA:
				log_info(logger, "Pedido de tarea recibido\n");
				break;

			case COD_TAREA:
				log_info(logger, "Recibo una tarea\n");
				//free(mensaje_recibido->tarea);
				break;

			case RECIBIR_PCB:
				//log_info(logger, "Recibo una pcb\n");
				//list_add(lista_pcb, (void*) mensaje_recibido->pcb);
				//almacenar_pcb(mensaje_recibido); //TODO en un futuro, capaz no podamos recibir el PCB por quedarnos sin memoria
				free(mensaje_recibido->pcb);
				break;

			case RECIBIR_TCB:
				log_info(logger, "Recibo una tcb\n");
				log_info(logger, "Tripulante %i, estado: %c, pos: %i %i, puntero_pcb: %i, sig_ins %i\n", (int) mensaje_recibido->tcb->TID, (char) mensaje_recibido->tcb->estado_tripulante, (int) mensaje_recibido->tcb->coord_x, (int) mensaje_recibido->tcb->coord_y, (int) mensaje_recibido->tcb->puntero_a_pcb, (int) mensaje_recibido->tcb->siguiente_instruccion);
				list_add(lista_tcb, (void*) mensaje_recibido->tcb);
				free(mensaje_recibido->tcb);
				//printf("Tripulante 1 pos: %c %c\n", (int) mensaje_recibido->tcb->coord_x, (int) mensaje_recibido->tcb->coord_y);
				break;

			case DESCONEXION:
				log_info(logger, "Se desconecto un cliente.");
				flag = 0;
				break;

			default:
				log_info(logger, "Se recibio un codigo invalido.\n");
				printf("El codigo es %d\n", mensaje_recibido->codigo_operacion);
				break;
		}
		free(mensaje_recibido);
	}

}


t_PCB* crear_pcb(char* path){
	t_PCB* pcb = malloc(sizeof(t_PCB));
	pcb -> PID = 1;
	pcb -> direccion_tareas = (uint32_t) path;
	return pcb;
}

t_TCB crear_tcb(t_PCB* pcb, int tid, char* posicion){
	t_TCB tcb;
	tcb.TID = tid;
	tcb.estado_tripulante = estado_tripulante[NEW];
	tcb.coord_x = posicion[0];
	tcb.coord_y = posicion[2];
	tcb.siguiente_instruccion = 5; //TODO
	tcb.puntero_a_pcb = 7;
	return tcb;
}

void ordenar_segmentos(){
    bool segmento_anterior(segmento* segmento_antes, segmento* segmento_despues) {
        return segmento_antes->base < segmento_despues->base;
    }
	log_info(logger,"Comienzo a ordenar los segmentos");
    list_sort(segmentos, (void*) segmento_anterior);
	log_info(logger,"Segmentos ordenados");
    return;
}

segmento* crear_segmento(int base, int tam, bool libre){
    segmento* nuevo_segmento = malloc(sizeof(segmento));
    nuevo_segmento->base = base;
    nuevo_segmento->tam = tam;
    nuevo_segmento->libre = libre;

    return nuevo_segmento;
}

segmento* buscar_segmento_libre(int tam){
	if (strcmp(CRITERIO_SELECCION, "FF") == 0) {
        log_debug(logger, "Empieza busqueda FirstFit");
        return first_fit(tam);
    } else if (strcmp(CRITERIO_SELECCION, "BF") == 0) {
        log_debug(logger, "Empieza busqueda BestFit");
        return best_fit(tam);
    } else {
        log_error(logger, "Metodo de asignacion desconocido");
        exit(EXIT_FAILURE);
    }
}

segmento* first_fit(int tam){
    int size = list_size(segmentos);
    for(int i=0; i<size; i++){
        segmento* x = list_get(segmentos, i);
        if(x->libre == true && tam <= x->tam){
            log_info(logger, "Segmento libre encontrado (base: %d)", x->base);
            return x;
        }
    }
    log_warning(logger, "No se encontró segmento");
    return NULL;
}

segmento* best_fit(int tam){
    int size = list_size(segmentos);
    t_list* candidatos = list_create();
    for(int i=0; i<size; i++){
        segmento* x = list_get(segmentos, i);
        if(x->libre == true && tam <= x->tam){
            log_info(logger, "Segmento libre con suficiente espacio encontrado (base: %d)", x->base);
            if(tam == x->tam){
				log_info(logger, "Mejor segmento encontrado (base:%d)", x->base);
				return x;
			}
            list_add(candidatos, x);
        }
    }
    log_debug(logger, "Buscando el mejor segmento");
    int candidatos_size = list_size(candidatos);
    if(candidatos_size != 0){
        segmento* best_fit;
        int best_fit_diff = 999999;
        for(int i=0; i<candidatos_size; i++){
            segmento* y = list_get(candidatos, i);
            int diff = y->tam - tam;
            if(best_fit_diff > diff){
                best_fit_diff = diff;
                best_fit = y;
            }
        }
        log_info(logger, "Mejor segmento encontrado (base:%d)", best_fit->base);
        return best_fit;
    }else{
        log_warning(logger, "No hay segmentos disponibles");
        return NULL;
    }
}

segmento* asignar_segmento(int tam){
	segmento* segmento_libre = buscar_segmento_libre(tam);
	if(segmento_libre != NULL){
		//Si el segmento es del tamaño justo, no tengo que reordenar
		if(segmento_libre->tam == tam){
			segmento_libre->libre = false;
			log_info(logger,"Segmento asignado (base:%d)", segmento_libre->base);
			return segmento_libre;
		}
		//Si no tengo que dividir el segmento
		else{
			segmento* nuevo_segmento = crear_segmento(segmento_libre->base,tam,false);
			list_add(segmentos,nuevo_segmento);
			segmento_libre->base += tam;
			segmento_libre->tam -= tam;
			log_info(logger,"Segmento asignado (base:%d)", nuevo_segmento->base);
			//Ordeno los segmentos por base ascendente
			ordenar_segmentos();

			return nuevo_segmento;
		}
	}
}

tabla_segmentos* crear_tabla_segmentos(uint32_t pid){
	tabla_segmentos* nueva_tabla = malloc(sizeof(tabla_segmentos));
	nueva_tabla->pid = pid;
	nueva_tabla->segmentos_tcb = list_create();
	return nueva_tabla;
}

tabla_paginas* crear_tabla_paginas(uint32_t pid){
	tabla_paginas* nueva_tabla = malloc(sizeof(tabla_paginas));
	nueva_tabla->pid = pid;
	nueva_tabla->paginas = list_create();
	return nueva_tabla;
}

void iniciar_memoria(){
	if(strcmp(ESQUEMA_MEMORIA,"SEGMENTACION")==0){
		log_info(logger,"Se inicia memoria con esquema se SEGMENTACION");
		segmentos = list_create();
		segmento* segmento_principal = crear_segmento(0,TAMANIO_MEMORIA,true);
		list_add(segmentos,segmento_principal);
	}else if(strcmp(ESQUEMA_MEMORIA,"PAGINACION")==0){
		log_info(logger,"Se inicia memoria con esquema de PAGINACION");
		paginas = list_create();
	}else{
		log_error(logger,"Esquema de memoria desconocido");
		exit(EXIT_FAILURE);
	}
}


void printSegmentosList() {
    int size = list_size(segmentos);
    printf("<--------------------------------------------\n");
    for(int i=0; i<size; i++) {
        segmento *s = list_get(segmentos, i);
        printf("base: %d, tam: %d, is_free: %s\n", s->base, s->tam, s->libre ? "true" : "false");
    }
    printf("-------------------------------------------->\n");
}