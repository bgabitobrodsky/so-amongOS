/*
 * i_mongo_store.c
 *
 *  Created on: 9 may. 2021
 *      Author: utnso
 */

#include "i_mongo_store.h"

#define	IP_MONGO_STORE config_get_string_value(config_mongo, "IP") // Verificar sintaxis
#define PUERTO_MONGO_STORE config_get_string_value(config_mongo, "PUERTO")

// Vars globales
t_log* logger_mongo;
t_config* config_mongo;
t_archivos archivos;

int main(int argc, char** argv){

	logger_mongo = log_create("mongo.log", "MONGO", 1, LOG_LEVEL_DEBUG); // Corregir nombres
	config_mongo = config_create("i_mongo_store.config");

	int socket_oyente = crear_socket_oyente(IP_MONGO_STORE, PUERTO_MONGO_STORE);
	args_escuchar_mongo args_escuchar;
	args_escuchar.socket_oyente = socket_oyente;

	iniciar_file_system();

	// Ver si es correcto considerandose que se usa fork(), no serian ULTs sino KLTs
	pthread_mutex_init(mutex_oxigeno, NULL);
	pthread_mutex_init(mutex_comida, NULL);
	pthread_mutex_init(mutex_basura, NULL);
    
	pthread_t hilo_escucha;	
	pthread_create(&hilo_escucha, NULL, (void*) escuchar_mongo, (void*) &args_escuchar);

	pthread_join(hilo_escucha, NULL); // Cambiar por lo que dijo Seba

	cerrar_archivos();
	cerrar_mutexs();
	close(socket_oyente);
	log_destroy(logger_mongo);
	config_destroy(config_mongo);
}


void escuchar_mongo(void* args) { // args no se cierra, fijarse donde cerrarlo
	args_escuchar_mongo *p = malloc(sizeof(args_escuchar_mongo));
	p = args;
	int socket_escucha = p->socket_oyente;
	int es_discordiador = 1;

	struct sockaddr_storage direccion_a_escuchar;
	socklen_t tamanio_direccion;
	int socket_especifico; // Sera el socket hijo que hara la conexion con el cliente

	if (listen(socket_escucha, 10) == -1) // Se pone el socket a esperar llamados, con una cola maxima dada por el 2do parametro, se eligio 10 arbitrariamente //TODO esto esta hardcodeado
		printf("Error al configurar recepcion de mensajes\n"); // Se verifica

	struct sigaction sa;
	sa.sa_handler = sigchld_handler; // Limpieza de procesos muertos
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
			if (es_discordiador) {
				sabotaje(socket_especifico);
				es_discordiador = 0;
			else {
				manejo_tripulante(socket_especifico); 
			}
			close(socket_especifico); // Cumple proposito, se cierra socket hijo
			exit(0); // Returnea
		}

		close(socket_especifico); // En hilo padre se cierra el socket hijo, total al arrancar el while se vuelve a settear, evita "port leaks" supongo
	}
}

void sabotaje(int socket_discordiador) {
	while(1) {
		wait(SIGUSR1);
		enviar_codigo(SABOTAJE, socket_discordiador);
		wait(verificacion);
		t_estructura* mensaje = recepcion_y_deserializacion(socket_discordiador); // TODO: Agregar cosas a Estructura
		reparar(mensaje);
		signal(reparado); 
		free(mensaje);
	}
}

int file_system_existente(char* punto_montaje, stat dir) { // TODO: Verificar sintaxis stat
	return (stat(punto_montaje, &dir) != -1);
}

void iniciar_file_system() {
	struct stat dir = {0};
	char* path_directorio = config_get_string_value(config_mongo, "PUNTO_MONTAJE");
	char* path_files = path_directorio;
	sprintf(path_files, "/Files");
	char* path_bitacoras = path_files;
	sprintf(path_bitacoras, "/Bitacoras");

	if (file_system_existente(path_directorio, dir)) {
		inicializar_archivos(path_files); // TODO: Revisar si open() funca como fopen()
	}
	else {
		mkdir(path_directorio, 0777); // TODO: Revisar que es lo de la derecha de mkdir, sacado de stackoverflow
		mkdir(path_files, 0777);
		mkdir(path_bitacoras, 0777); // TODO: Crear bloque y superbloque

		inicializar_archivos(path_files);

		free(path_files);
		free(path_bitacoras);
	}

	free(path_directorio);
}

void cerrar_archivos() {
	fclose(archivos.oxigeno);
	fclose(archivos.comida);
	fclose(archivos.basura);
}

void cerrar_mutexs() {
	pthread_mutex_destroy(mutex_oxigeno);
	pthread_mutex_destroy(mutex_comida);
	pthread_mutex_destroy(mutex_basura);
}