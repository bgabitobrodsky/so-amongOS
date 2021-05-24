/*
 * i_mongo_store.c
 *
 *  Created on: 9 may. 2021
 *      Author: utnso
 */

#include "i_mongo_store.h"

#define	IP_MONGO_STORE config_get_string_value(config_mongo, "IP_MONGO_STORE") // Verificar sintaxis
#define PUERTO_MONGO_STORE config_get_string_value(config_mongo, "PUERTO_MONGO_STORE")

// Vars globales
t_log* logger_mongo;
t_config* config_mongo;

int main(int argc, char** argv){

	logger_mongo = log_create("mongo.log", "MONGO", 1, LOG_LEVEL_DEBUG); // Corregir nombres
	config_mongo = config_create("mongo.config");


	//int socket_oyente = crear_socket_oyente(IP_MONGO_STORE, PUERTO_MONGO_STORE); TODO HARCODEADO HASTA CAMBIAR LAS CONFIGS
	int socket_oyente = crear_socket_oyente("127.1.1.2", "4000");
	args_escuchar_mongo args_escuchar;
	args_escuchar.socket_oyente = socket_oyente;
    
	pthread_t hilo_escucha;	
	pthread_create(&hilo_escucha, NULL, (void*) escuchar_mongo, (void*) &args_escuchar);
	pthread_join(hilo_escucha, NULL);
	
	close(socket_oyente);
	log_destroy(logger_mongo);
	config_destroy(config_mongo);
}


void escuchar_mongo(void* args) { // args no se cierra, fijarse donde cerrarlo
	args_escuchar_mongo *p = malloc(sizeof(args_escuchar_mongo));
	p = args;
	int socket_escucha = p->socket_oyente;
	free(p->socket_oyente);
	free(p);

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
			printf("Hola soy el Mongo"); // Funcion enviada por parametro con puntero para que ejecuten los hijos del proceso
			close(socket_especifico); // Cumple proposito, se cierra socket hijo
			exit(0); // Returnea
		}

		close(socket_especifico); // En hilo padre se cierra el socket hijo, total al arrancar el while se vuelve a settear, evita "port leaks" supongo
	}
}
