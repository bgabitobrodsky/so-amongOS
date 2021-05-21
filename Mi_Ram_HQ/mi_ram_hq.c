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
	(void*) p_atender_clientes = atender_clientes();

    int socket_server = crear_socket_oyente(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ); // Se podria delegar a un hilo
	escuchar(socket_server, p_atender_clientes);

    close(socket_server);

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