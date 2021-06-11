/*
 * mi_ram_hq.c
 *
 *  Created on: 9 may. 2021
 *      Author: utnso
 */

#include "mi_ram_hq.h"

#define	IP_MI_RAM_HQ config_get_string_value(config_miramhq, "IP")
#define PUERTO_MI_RAM_HQ config_get_string_value(config_miramhq, "PUERTO")

// Vars globales
t_log* logger_miramhq;
t_config* config_miramhq;

t_list* lista_tcb;
t_list* lista_pcb;

int main(int argc, char** argv) {
	logger_miramhq = log_create("mi_ram_hq.log", "MI_RAM_HQ", 1, LOG_LEVEL_DEBUG);
	config_miramhq = config_create("mi_ram_hq.config");
	char* tamanio_memoria = config_get_string_value(config_miramhq, "TAMANIO_MEMORIA");
	char* memoria = malloc(atoi(tamanio_memoria));
	free(tamanio_memoria);

	lista_tcb = list_create();
	lista_pcb = list_create();

    int socket_oyente = crear_socket_oyente(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ); //Se podria delegar a un hilo
    //int socket_oyente = crear_socket_oyente("127.0.0.1", "25430"); //Se podria delegar a un hilo

    args_escuchar args_miram;
	args_miram.socket_oyente = socket_oyente;

	pthread_t hilo_escucha;
	pthread_create(&hilo_escucha, NULL, (void*) escuchar_miram, (void*) &args_miram);
	pthread_join(hilo_escucha, NULL);

	close(socket_oyente);
	log_destroy(logger_miramhq);
	config_destroy(config_miramhq);
	free(memoria);

	return EXIT_SUCCESS;
}

void atender_clientes(int socket_hijo) { // TODO miram no termina ni siquiera si muere discordiador. una forma de arreglarlo es hacer que estas funciones devuelvan valores.
	int flag = 1;
	t_estructura* mensaje_recibido;
	    while(flag) { //TODO cambiar por do while  y liberar la estructura
			mensaje_recibido = recepcion_y_deserializacion(socket_hijo); // Hay que pasarle en func hijos dentro de socketes.c al socket hijo, y actualizar los distintos punteros a funcion
			//sleep(1); //no quitar. sirve para testeos

			switch(mensaje_recibido->codigo_operacion) {
				case MENSAJE:
					log_info(logger_miramhq, "Mensaje recibido");
					break;

				case PEDIR_TAREA:
					log_info(logger_miramhq, "Pedido de tarea recibido");
					break;

				case COD_TAREA:
					log_info(logger_miramhq, "Recibo una tarea");
					break;

				case RECIBIR_PCB:
					log_info(logger_miramhq, "Recibo una pcb");
					list_add(lista_pcb, (void*) mensaje_recibido->pcb);
					//almacenar_pcb(mensaje_recibido); //TODO en un futuro, capaz no podamos recibir el PCB por quedarnos sin memoria
					break;

				case RECIBIR_TCB:
					log_info(logger_miramhq, "Recibo una tcb");
					log_info(logger_miramhq, "Tripulante: %i, estado: %c pos: %i %i\n", (int) mensaje_recibido->tcb->TID, (char) mensaje_recibido->tcb->estado_tripulante, (int) mensaje_recibido->tcb->coord_x, (int) mensaje_recibido->tcb->coord_y);
					list_add(lista_tcb, (void*) mensaje_recibido->tcb);

					//printf("Tripulante 1 pos: %c %c\n", (int) mensaje_recibido->tcb->coord_x, (int) mensaje_recibido->tcb->coord_y);
					break;

				case DESCONEXION:
					log_info(logger_miramhq, "Se desconecto el modulo Discordiador");
					flag = 0;
					break;

				default:
					log_info(logger_miramhq, "Se recibio un codigo invalido.\n");
					printf("El codigo es %d\n", mensaje_recibido->codigo_operacion);
					break;
			}
	    }

}

void escuchar_miram(void* args) { // No se libera args, ver donde liberar
	args_escuchar* p = malloc(sizeof(args_escuchar));
	p = args;
	int socket_escucha = p->socket_oyente;

	struct sockaddr_storage direccion_a_escuchar;
	socklen_t tamanio_direccion;
	int socket_especifico; // Sera el socket hijo que hara la conexion con el cliente

	if (listen(socket_escucha, 10) == -1) // Se pone el socket a esperar llamados, con una cola maxima dada por el 2do parametro, se eligio 10 arbitrariamente //TODO esto esta hardcodeado
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

			if (!fork()) { // Se crea un proceso hijo si se pudo forkear correctamente
				close(socket_escucha); // Cierro escucha en este hilo, total no sirve mas
				atender_clientes(socket_especifico); // Funcion enviada por parametro con puntero para que ejecuten los hijos del proceso
				close(socket_especifico); // Cumple proposito, se cierra socket hijo
				exit(0); // Returnea
			}

			close(socket_especifico); // En hilo padre se cierra el socket hijo, total al arrancar el while se vuelve a settear, evita "port leaks" supongo
		}
}
