/*
 * i_mongo_store.c
 *
 *  Created on: 9 may. 2021
 *      Author: utnso
 */

#include "i_mongo_store.h"

#define	IP_MONGO_STORE config_get_string_value(config_mongo, "IP") // Verificar sintaxis
#define PUERTO_MONGO_STORE config_get_string_value(config_mongo, "PUERTO")
#define LIMIT_CONNECTIONS 10
int sistema_activo = 1;

int main(int argc, char** argv){

	// Se crean estructuras de registro y configuracion
	logger_mongo = log_create("mongo.log", "MONGO", 1, 0); // Corregir nombres
	config_mongo = config_create("i_mongo_store.config");
	signal(SIGUSR1, sabotaje);

	// Se crea la lista de bitacoras para los tripulantes, lista actua de registro para saber que tripulantes poseen bitacora en Mongo
	bitacoras = list_create();

	// Se establecen estructuras para el setteo del server en escuchar_mongo
	int socket_oyente = crear_socket_oyente(IP_MONGO_STORE, PUERTO_MONGO_STORE);
	args_escuchar args_escuchar;
	args_escuchar.socket_oyente = socket_oyente;

	// Se settea el FileSystem
	iniciar_file_system();
	log_info(logger_mongo, "Se inicio el FileSystem correctamente.\n");

	// Se crean los mutexs de los distintos archivos que se alteran, bitacoras no necesitan por ser propias a cada tripulante (puede que se requiera un mutex para la lista)
	pthread_mutex_init(&mutex_oxigeno, NULL);
	pthread_mutex_init(&mutex_comida, NULL);
	pthread_mutex_init(&mutex_basura, NULL);
	// mutex_blocks

	pthread_t hilo_escucha;
	pthread_create(&hilo_escucha, NULL, (void*) escuchar_mongo, (void*) &args_escuchar);
	pthread_detach(hilo_escucha);

	/*while(sistema_activo){
		usleep(1);
	}*/ //TODO ver cuando debería terminarse el mongo

	// Se cierra to.do lo que se usa
	cerrar_archivos();
	cerrar_mutexs();
	close(socket_oyente);
	list_destroy(bitacoras);
	log_info(logger_mongo, "El I_Mongo_Store finalizo su ejecucion.\n");
	log_destroy(logger_mongo);
	config_destroy(config_mongo);
	free(path_directorio);
	free(path_superbloque);
	free(path_blocks);
	free(path_files);
	free(path_basura);
	free(path_comida);
	free(path_oxigeno);
	free(path_bitacoras);
}

void escuchar_mongo(void* args) { // TODO args no se cierra, fijarse donde cerrarlo
    args_escuchar *p = malloc(sizeof(args_escuchar));
    p = args;
    int socket_escucha = p->socket_oyente;

    int es_discordiador = 1;

    struct sockaddr_storage direccion_a_escuchar;
    socklen_t tamanio_direccion;
    int socket_especifico;

    if (listen(socket_escucha, LIMIT_CONNECTIONS) == -1)
        log_info(logger_mongo, "Error al configurar recepcion de mensajes\n");

    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        log_info(logger_mongo, "Error al limpiar procesos\n");
        exit(1);
    }

	while (1) {
        tamanio_direccion = sizeof(direccion_a_escuchar);
        socket_especifico = accept(socket_escucha, (struct sockaddr*) &direccion_a_escuchar, &tamanio_direccion); // Se acepta (por FIFO si no me equivoco) el llamado entrante a socket escucha

		if (socket_especifico != -1) {

		// Se verifica que la primera conexion a Mongo sea del modulo Discordiador, debe ser asi por defecto
        if (es_discordiador == 1) {
       		es_discordiador = 0; // Se cambia flujo para que to.do lo subsiguiente sean tripulantes

       		hilo_discordiador* parametros = malloc(sizeof(hilo_tripulante));
       		parametros->socket = socket_especifico;
       		pthread_t un_hilo_discordiador;
       		pthread_create(&un_hilo_discordiador, NULL, (void*) sabotaje, (void *) parametros);
       		pthread_detach(un_hilo_discordiador);
       		//Falta cerrar sockets, hacerlo despues de juntar hilos
       	}
       	else { // Flujo para tripulantes
       		hilo_tripulante* parametros = malloc(sizeof(hilo_tripulante));
       		parametros->socket = socket_especifico;
       		pthread_t un_hilo_tripulante;
       		pthread_create(&un_hilo_tripulante, NULL, (void*) manejo_tripulante, (void *) parametros);
       		pthread_detach(un_hilo_tripulante);
       		//Falta cerrar sockets, hacerlo despues de juntar hilos
           	}
       	}

		// TODO: Ver si este close explota hilos
		close(socket_especifico); // En hilo padre se cierra el socket hijo, total al arrancar el while se vuelve a settear, evita "port leaks" supongo
	}
}

void sabotaje(int parametro) {
	int flag = 1;
	int socket_discordiador;

	if (flag)
		socket_discordiador = parametro; // WTF is this? --> --> La primera conexión recibe el socket de mongo, el resto son por sabotajes. --> --> Claro pero en flujo son iguales socket y parametro, nunca llega un tripu aca entonces distincion al pedo, ya se hizo
	
	// Se cicla infinitamente en espera a sabotajes
	while(1) {

		// Se espera que set reciva la signal correspondiente
		if (parametro == SIGUSR1) {
			log_info(logger_mongo, "Se detecto un sabotaje.\n");
			// Se avisa y se espera a Discordiador que tome las acciones correspondientes al sabotaje
			enviar_codigo(SABOTAJE, socket_discordiador);

			t_estructura* listo = recepcion_y_deserializacion(socket_discordiador);

			if (listo->codigo_operacion == LISTO) {

				// Se envian la primera posicion no enviada hasta el momento
				enviar_posicion_sabotaje(socket_discordiador);

				// Se espera a que Discordiador envie un designado para reparar
				t_estructura* mensaje = recepcion_y_deserializacion(socket_discordiador);

				// Se activaria el protocolo fcsk
				char* rotura = reparar();

				log_info(logger_mongo, "Se reparo el sabotaje.\n");
				log_info(logger_mongo, "Se habia saboteado %s.\n", rotura);
				// Se avisa fin de sabotaje al Discordiador para que continue sus operaciones
				enviar_codigo(LISTO, socket_discordiador);

				free(mensaje);
			}
		}
	}
}

void iniciar_file_system() {
	// Se crea estructura para verificar directorios
	struct stat dir = {0};

	// Se obtiene el path donde estara el arbol de directorios, de config
	path_directorio = config_get_string_value(config_mongo, "PUNTO_MONTAJE");

	// Se settea el path a files, carpeta dentro del punto de montaje
	path_files = malloc(strlen(path_directorio) + strlen("/Files") + 1);
	strncpy(path_files, path_directorio, strlen(path_directorio) + 1);
	path_files = strcat(path_files, "/Files");

	// Se settea el path a bitacoras, carpeta dentro de files
	path_bitacoras = malloc(strlen(path_files) + strlen("/Bitacoras") + 1);
	strncpy(path_bitacoras , path_files, strlen(path_files) + 1);
	path_bitacoras = strcat(path_bitacoras, "/Bitacoras");

	// Se verifica si ya tengo carpetas hechas, o sea, un filesystem
	if ((stat(path_directorio, &dir) != -1)) {
		log_info(logger_mongo, "Se detecto un FileSystem existente.\n");
		inicializar_archivos_preexistentes();
	}
	else {
		// Como no hay carpetas, se crean
		mkdir(path_directorio, 0777);
		mkdir(path_files, 0777);
		mkdir(path_bitacoras, 0777);
		log_info(logger_mongo, "Se creo un FileSystem.\n");
		// Se asignan los archivos como antes
		inicializar_archivos();
	}
	log_error(logger_mongo, "4");
	pthread_t un_hilo; // Estaria bueno crearlo en main
	pthread_create(&un_hilo, NULL, (void*) sincronizar_blocks, NULL);
	pthread_detach(un_hilo);
	log_error(logger_mongo, "5");
}

void sincronizar_blocks() {
	while(1) {
		msync(directorio.mapa_blocks, CANTIDAD_BLOQUES * TAMANIO_BLOQUE, MS_SYNC);
		sleep(config_get_int_value(config_mongo, "TIEMPO_SINCRONIZACION"));
	}
}

void cerrar_archivos() {
	fclose(recurso.oxigeno);
	fclose(recurso.comida);
	fclose(recurso.basura);
}

void cerrar_mutexs() {
	pthread_mutex_destroy(&mutex_oxigeno);
	pthread_mutex_destroy(&mutex_comida);
	pthread_mutex_destroy(&mutex_basura);
}
