/*
 * mi_ram_hq.c
 *
 *  Created on: 9 may. 2021
 *      Author: utnso
 */

#include "mi_ram_hq.h"

int main(int argc, char** argv){

    int mi_ram_fd = iniciar_servidor();
	logger = log_create("log.log", "Servidor", 1, LOG_LEVEL_DEBUG);
    log_info(logger, "Mi ram hq lista");

    int discordiador_fd = esperar_discordiador(mi_ram_fd);

    while(1){
		int cod_op = leer_operacion(discordiador_fd);
		switch(cod_op){
		case MENSAJE:
			printf("RECIBIDO");
			break;
		case -1:
			printf("el cliente se desconecto. Terminando servidor");
			return EXIT_FAILURE;
		default:
			printf("Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
	return EXIT_SUCCESS;

    /*pthread_t hilo_escucha;
	pthread_create(&hilo_escucha, NULL, (void*) escuchar_alos_cliente, NULL);

	pthread_join(hilo_escucha, NULL);*/
}

/*
void escuchar_alos_cliente(){
    int socket_oyente = crear_socket_oyente("127.0.0.2", "4000");
    
    escuchar(socket_oyente, (void*) hola);

}

void hola(){
    printf("Hola");
}
*/
