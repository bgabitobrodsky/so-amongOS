/*
 * mi_ram_hq.c
 *
 *  Created on: 9 may. 2021
 *      Author: utnso
 */

//TODO: por alguna razon, no funcionan los printf en este modulo.

#include "mi_ram_hq.h"

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

	// Reinicio el log
	FILE* f = fopen("mi_ram_hq.log", "w");
    fclose(f);

	// Inicializar
	logger = log_create("mi_ram_hq.log", "MI_RAM_HQ", 1, LOG_LEVEL_DEBUG);
	config = config_create("mi_ram_hq.config");

	iniciar_memoria();

	lista_tcb = list_create();
	lista_pcb = list_create();

    //int socket_oyente = crear_socket_oyente(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
    //escuchar_miram((void*) socket_oyente);//debug
    
	int socket_oyente = crear_socket_oyente(IP, PUERTO);

    args_escuchar args_miram;
	args_miram.socket_oyente = socket_oyente;

	pthread_t hilo_escucha;
	pthread_create(&hilo_escucha, NULL, (void*) escuchar_miram, (void*) &args_miram);

	pthread_join(hilo_escucha, NULL);
	close(socket_oyente);

	log_destroy(logger);
	config_destroy(config);
	free(memoria_principal)
	free(memoria);

	return EXIT_SUCCESS;
}

void atender_clientes(int socket_hijo) { // TODO miram no termina ni siquiera si muere discordiador. una forma de arreglarlo es hacer que estas funciones devuelvan valores.
	int flag = 1;
	log_info(logger, "Atendiendo.\n");

	    while(flag) {
	    	t_estructura* mensaje_recibido = recepcion_y_deserializacion(socket_hijo); // Hay que pasarle en func hijos dentro de socketes.c al socket hijo, y actualizar los distintos punteros a funcion
	    	sleep(1); //para que no se rompa en casos de bug o tiempos de espera

			switch(mensaje_recibido->codigo_operacion) {

				case MENSAJE:
					//log_info(logger, "Mensaje recibido\n");
					break;

				case PEDIR_TAREA:
					log_info(logger, "Pedido de tarea recibido\n");
					break;

				case COD_TAREA:
					log_info(logger, "Recibo una tarea\n");
					//free(mensaje_recibido->tarea);
					break;

				case RECIBIR_PCB:
					log_info(logger, "Recibo un pcb\n");
					//list_add(lista_pcb, (void*) mensaje_recibido->pcb);
					//almacenar_pcb(mensaje_recibido); //TODO en un futuro, capaz no podamos recibir el PCB por quedarnos sin memoria
					free(mensaje_recibido->pcb);
					break;

				case RECIBIR_TCB:
					//log_info(logger, "Recibo una tcb\n");
					//log_info(logger, "Tripulante %i, estado: %c, pos: %i %i, puntero_pcb: %i, sig_ins %i\n", (int) mensaje_recibido->tcb->TID, (char) mensaje_recibido->tcb->estado_tripulante, (int) mensaje_recibido->tcb->coord_x, (int) mensaje_recibido->tcb->coord_y, (int) mensaje_recibido->tcb->puntero_a_pcb, (int) mensaje_recibido->tcb->siguiente_instruccion);
					list_add(lista_tcb, (void*) mensaje_recibido->tcb);
					free(mensaje_recibido->tcb);
					//printf("Tripulante 1 pos: %c %c\n", (int) mensaje_recibido->tcb->coord_x, (int) mensaje_recibido->tcb->coord_y);
					break;

				case DESCONEXION:
					log_info(logger, "Se desconecto el modulo Discordiador");
					flag = 0;
					break;

				default:
					log_info(logger, "Se recibio un codigo invalido.\n");
					//printf("El codigo es %d\n", mensaje_recibido->codigo_operacion);
					break;
			}
			free(mensaje_recibido);
	    }

}

void escuchar_miram(void* args) { // No se libera args, ver donde liberar
	args_escuchar* p = malloc(sizeof(args_escuchar));
	p = args;
	int socket_escucha = p->socket_oyente;
	//int socket_escucha = (int) args; //Tema de testeos, no borrar

	struct sockaddr_storage direccion_a_escuchar;
	socklen_t tamanio_direccion;
	int socket_especifico; // Sera el socket hijo que hara la conexion con el cliente

	if (listen(socket_escucha, 10) == -1) //TODO esto esta hardcodeado
		printf("Error al configurar recepcion de mensajes\n");

	struct sigaction sa;
		sa.sa_handler = sigchld_handler;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = SA_RESTART;
		if (sigaction(SIGCHLD, &sa, NULL) == -1) {
			printf("Error al limpiar procesos\n");
			exit(1);
		}

	while (1) { // Loop infinito donde aceptara clientes
			tamanio_direccion = sizeof(direccion_a_escuchar);
			socket_especifico = accept(socket_escucha, (struct sockaddr*) &direccion_a_escuchar, &tamanio_direccion); // Se acepta (por FIFO si no me equivoco) el llamado entrante a socket escucha

			if (!fork()) {
				close(socket_escucha);
				atender_clientes(socket_especifico);
				close(socket_especifico); 
				exit(0);
			}

			close(socket_especifico); // En hilo padre se cierra el socket hijo, total al arrancar el while se vuelve a settear, evita "port leaks" supongo
		}
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
