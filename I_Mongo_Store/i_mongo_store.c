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
t_archivos archivos; // Declarar ese struct

int main(int argc, char** argv){

	logger_mongo = log_create("mongo.log", "MONGO", 1, LOG_LEVEL_DEBUG); // Corregir nombres
	config_mongo = config_create("i_mongo_store.config");

	int socket_oyente = crear_socket_oyente(IP_MONGO_STORE, PUERTO_MONGO_STORE);
	args_escuchar_mongo args_escuchar;
	args_escuchar.socket_oyente = socket_oyente;

	pthread_t iniciar_file_system;
	pthread_create(&iniciar_file_system, NULL, (void*) iniciar_file_system, NULL);
    
	pthread_t hilo_escucha;	
	pthread_create(&hilo_escucha, NULL, (void*) escuchar_mongo, (void*) &args_escuchar);

	pthread_join(hilo_escucha, NULL);
	pthread_join(iniciar_file_system, NULL);

	cerrar_archivos();
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
		t_estructura* mensaje = recepcion_y_deserializacion(socket_discordiador);
		reparar(mensaje);
		signal(reparado); 
		free(mensaje);
	}
}

void manejo_tripulante(int socket_tripulante) {
	while(1) {
		t_estructura* mensaje = recepcion_y_deserializacion(socket_discordiador);
		alterar(mensaje->codigo_operacion, mensaje->cantidad); 
		free(mensaje);
	}
}

void iniciar_file_system() {
	struct stat dir = {0};
	char* path_directorio = config_get_string_value(config_mongo, "PUNTO_MONTAJE");
	char* path_files = path_directorio;
	sprintf(path_files, "/Files");

	if (file_system_existente(path_directorio, dir)) {
		inicializar_archivos(path_files); // TODO: Revisar si open() funca como fopen()
	}
	else {
		mkdir(path_directorio, 0777); // TODO: Revisar que es lo de la derecha de mkdir, sacado de stackoverflow
		mkdir(path_files, 0777);
		inicializar_archivos(path_files);
		free(path_files);
	}

	free(path_directorio);
}

int file_system_existente(char* punto_montaje, stat dir) { // TODO: Verificar sintaxis stat
	return (stat(punto_montaje, &dir) != -1);
}

void inicializar_archivos(char* path_files) { // TODO: Puede romper, revisar repeticion de codigo
	char* path_oxigeno;
	sprintf(path_oxigeno, "%s/Oxigeno.ims", path_files);

	char* path_comida;
	sprintf(path_comida, "%s/Comida.ims", path_files);

	char* path_basura;
	sprintf(path_basura, "%s/Basura.ims", path_files);

	int filedescriptor_oxigeno = open(path_oxigeno, O_RDWR | O_APPEND | O_CREAT); // TODO: Ver que son esas constantes
	int filedescriptor_comida = open(path_comida, O_RDWR | O_APPEND | O_CREAT);   
	int filedescriptor_basura = open(path_basura, O_RDWR | O_APPEND | O_CREAT);

	FILE* file_oxigeno = fdopen(filedescriptor_oxigeno, "r+");
	FILE* file_comida = fdopen(filedescriptor_comida, "r+");
	FILE* file_basura = fdopen(filedescriptor_basura, "r+");

	archivos.oxigeno = file_oxigeno;
	archivos.comida = file_comida;
	archivos.basura = file_basura;
}

void alterar(int codigo_archivo, int cantidad) { // TODO: Revisar repeticion de codigo
	switch(codigo_archivo) { 
		case OXIGENO:
			if (cantidad > 0) 
				agregar(archivos.oxigeno, cantidad, 'O');
			else
				quitar(archivos.oxigeno, cantidad, 'O');
			break;
		case COMIDA: 
			if (cantidad > 0) 
				agregar(archivos.comida, cantidad, 'C');
			else
				quitar(archivos.comida, cantidad, 'C');
			break;
		case BASURA: 
			if (cantidad > 0) 
				agregar(archivos.basura, cantidad, 'B');
			else
				quitar(archivos.basura, cantidad, 'B');
			break;
	}
}

void agregar(FILE* archivo, int cantidad, char tipo) {
	for(int i = 0; i < cantidad; i++) {
		putc(tipo, archivo);
	}
}

void quitar(FILE* archivo, int cantidad, char tipo) {
	char c;
	int contador = 0;
	for (c = getc(archivo); c != EOF; c = getc(archivo))
        contador++;

	int nueva_cantidad = max(contador + cantidad, 0); // Cantidad es negativo en este caso
	fclose(archivo);
	fopen(archivo, "w"); // Reseteo archivo
	fclose(archivo);
	fopen(archivo, "r+"); // Lo reabro con r+ para no joder otras funciones, revisar

	agregar(archivo, nueva_cantidad, tipo);
}

void cerrar_archivos() {
	fclose(archivos.oxigeno);
	fclose(archivos.comida);
	fclose(archivos.basura);
}