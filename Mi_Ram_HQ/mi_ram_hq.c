/*
 * mi_ram_hq.c
 *
 *  Created on: 9 may. 2021
 *      Author: utnso
 */

// TODO: por alguna razon, no funcionan los printf en este modulo.

#include "mi_ram_hq.h"
#include <comms/generales.h>


#define	IP_MI_RAM_HQ config_get_string_value(config, "IP")
#define PUERTO_MI_RAM_HQ config_get_string_value(config, "PUERTO")
#define TAMANIO_MEMORIA config_get_int_value(config, "TAMANIO_MEMORIA")
#define ESQUEMA_MEMORIA config_get_string_value(config, "ESQUEMA_MEMORIA")
#define CRITERIO_SELECCION config_get_string_value(config, "CRITERIO_SELECCION")

t_log* logger;
t_config* config;

t_list* lista_tcb;
t_list* lista_pcb;

char estado_tripulante[4] = {'N', 'R', 'E', 'B'};

int main(int argc, char** argv) {

	

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

    //int socket_oyente = crear_socket_oyente(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
    //escuchar_miram((void*) socket_oyente);//debug
    
	int socket_oyente = crear_socket_oyente(IP, PUERTO);

  	args_escuchar args_miram;
	args_miram.socket_oyente = socket_oyente;

	pthread_t hilo_escucha;
	pthread_create(&hilo_escucha, NULL, (void*) proceso_handler, (void*) &args_miram);
	pthread_join(hilo_escucha, NULL);

	pthread_join(hilo_escucha, NULL);
	close(socket_oyente);

	log_destroy(logger);
	config_destroy(config);
	free(memoria_principal)
	free(memoria);

	return EXIT_SUCCESS;
}

void proceso_handler(void* args) {
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
		printf("Error al configurar recepcion de mensajes\n");

	while (1) {
		if ((socket_especifico = accept(socket_escucha, (struct sockaddr*) &address, (socklen_t *) &addrlen)) > 0) {
			// Maté la verificación
			log_info(logger_miramhq, "Se conecta un nuevo proceso");

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
	log_info(logger_miramhq, "Atendiendo. %i\n", parametros->socket);

	while(flag) {
		t_estructura* mensaje_recibido = recepcion_y_deserializacion(parametros->socket);

		sleep(1); //para que no se rompa en casos de bug o tiempos de espera

		switch(mensaje_recibido->codigo_operacion) {

			case MENSAJE:
				log_info(logger_miramhq, "Mensaje recibido\n");
				break;

			case PEDIR_TAREA:
				log_info(logger_miramhq, "Pedido de tarea recibido\n");
				break;

			case COD_TAREA:
				log_info(logger_miramhq, "Recibo una tarea\n");
				//free(mensaje_recibido->tarea);
				break;

			case RECIBIR_PCB:
				//log_info(logger_miramhq, "Recibo una pcb\n");
				//list_add(lista_pcb, (void*) mensaje_recibido->pcb);
				//almacenar_pcb(mensaje_recibido); //TODO en un futuro, capaz no podamos recibir el PCB por quedarnos sin memoria
				free(mensaje_recibido->pcb);
				break;

			case RECIBIR_TCB:
				log_info(logger_miramhq, "Recibo una tcb\n");
				log_info(logger_miramhq, "Tripulante %i, estado: %c, pos: %i %i, puntero_pcb: %i, sig_ins %i\n", (int) mensaje_recibido->tcb->TID, (char) mensaje_recibido->tcb->estado_tripulante, (int) mensaje_recibido->tcb->coord_x, (int) mensaje_recibido->tcb->coord_y, (int) mensaje_recibido->tcb->puntero_a_pcb, (int) mensaje_recibido->tcb->siguiente_instruccion);
				list_add(lista_tcb, (void*) mensaje_recibido->tcb);
				free(mensaje_recibido->tcb);
				//printf("Tripulante 1 pos: %c %c\n", (int) mensaje_recibido->tcb->coord_x, (int) mensaje_recibido->tcb->coord_y);
				break;

			case DESCONEXION:
				log_info(logger_miramhq, "Se desconecto un cliente.");
				flag = 0;
				break;

			default:
				log_info(logger_miramhq, "Se recibio un codigo invalido.\n");
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


t_PCB* crear_pcb(char* path){
	t_PCB* pcb = malloc(sizeof(t_PCB));
	pcb -> PID =1;
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



tabla_segmentos* crear_tabla_segmentos(uint32_t pid){
	tabla_segmentos* nueva_tabla = malloc(sizeof(tabla_segmentos));
	nueva_tabla->pid = pid;
	nueva_tabla->segmentos = list_create();
	return nueva_tabla;
}

void iniciar_memoria(){
	memoria_principal = malloc(TAMANIO_MEMORIA);
	if(strcmp(ESQUEMA_MEMORIA,"SEGMENTACION")==0){
		log_info(logger,"Se inicia memoria con esquema se SEGMENTACION");
		segmentos = list_create();
	}else if(strcmp(ESQUEMA_MEMORIA,"PAGINACION")==0){
		log_info(logger,"Se inicia memoria con esquema de PAGINACION");
		paginas = list_create();
	}else{
		log_error(logger,"Esquema de memoria desconocido");
		exit(EXIT_FAILURE);
	}
}
