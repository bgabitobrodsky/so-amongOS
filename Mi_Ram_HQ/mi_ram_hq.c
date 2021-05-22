/*
 * mi_ram_hq.c
 *
 *  Created on: 9 may. 2021
 *      Author: utnso
 */

#include "mi_ram_hq.h"

#define	IP_MI_RAM_HQ config_get_string_value(config_miramhq, "IP_MI_RAM_HQ")
#define PUERTO_MI_RAM_HQ config_get_string_value(config_miramhq, "PUERTO_MI_RAM_HQ")

// Vars globales
t_log* logger_miramhq;
t_config* config_miramhq;

int main(int argc, char** argv) {

	logger_miramhq = log_create("mi_ram_hq.log", "MI_RAM_HQ", 1, LOG_LEVEL_DEBUG);
	config_miramhq = config_create("mi_ram_hq.config");

    int socket_oyente = crear_socket_oyente(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ); // Se podria delegar a un hilo
	pthread_t hilo_escucha;
    void (*p_escuchar_miram)(int) = &escuchar_miram;
	pthread_create(&hilo_escucha, NULL, p_escuchar_miram(socket_oyente), NULL);
	pthread_join(hilo_escucha, NULL);

    close(socket_server);
	free(p_escuchar_miram);
    log_destroy(logger);
    config_destroy(config);

	return EXIT_SUCCESS;
}

void atender_clientes(int socket_hijo) {
	    while(1) {
			t_estructura* mensaje_recibido = recepcion_y_deserializacion(socket_hijo); // Hay que pasarle en func hijos dentro de socketes.c al socket hijo, y actualizar los distintos punteros a funcion
		
			switch(mensaje_recibido->codigo_operacion) {
				case MENSAJE:
					log_info(logger, "Mensaje recibido");
					break;

				case PEDIR_TAREA:
					log_info(logger, "Pedido de tarea recibido");
					break;

				case COD_TAREA:
					printf("Recibo una tarea");
					break;

				case -1: // Puede que rompa si discordiador no envia mensajes, mejor hacerlo un codigo de por si
					log_info(logger, "Se desconecto el modulo Discordiador");
					return EXIT_FAILURE;
		}
	}
}

void escuchar_miram(int socket_escucha){
	struct sockaddr_storage direccion_a_escuchar;
	socklen_t tamanio_direccion;
	int socket_especifico; // Sera el socket hijo que hara la conexion con el cliente

	if (listen(socket_escucha, 10) == -1) // Se pone el socket a esperar llamados, con una cola maxima dada por el 2do parametro, se eligio 10 arbitrariamente //TODO esto esta hardcodeado
		printf("Error al configurar recepcion de mensajes\n"); // Se verifica

	/*sa.sa_handler = sigchld_handler; // Limpieza de procesos muertos, ctrl C ctrl V del Beej, porlas
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		printf("Error al limpiar procesos\n");
		exit(1);
	}*/

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