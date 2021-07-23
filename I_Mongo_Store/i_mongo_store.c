/*
 * i_mongo_store.c
 *
 *  Created on: 9 may. 2021
 *      Author: utnso
 */

#include "i_mongo_store.h"

int socket_discordiador;
char** posiciones_sabotajes;
t_list* lista_bloques_ocupados;
int sistema_activo = 1;

int main(int argc, char** argv){

	// Se crean estructuras de registro y configuracion
	logger_mongo = log_create("mongo.log", "i_mongo_store", 1, 0); // Corregir nombres
	config_mongo = config_create("i_mongo_store.config");
	config_superbloque = config_create("superbloque.config");
	signal(SIGUSR1, sabotaje);
	posiciones_sabotajes = POSICIONES_SABOTAJE;
	lista_bloques_ocupados = list_create();

	FILE* f = fopen("mongo.log", "w");
    fclose(f);

	// Se crea la lista de bitacoras para los tripulantes, lista actua de registro para saber que tripulantes poseen bitacora en Mongo
	bitacoras = list_create();

	//Creo el diccionario de los locks
	crearDiccionarioLocks();

	// Se establecen estructuras para el setteo del server en escuchar_mongo
	int socket_oyente = crear_socket_oyente(IP_MONGO_STORE, PUERTO_MONGO_STORE);
	args_escuchar args_escuchar;
	args_escuchar.socket_oyente = socket_oyente;

	// Se settea el FileSystem
	iniciar_file_system();
	log_info(logger_mongo, "Se inicio el FileSystem correctamente.");

	// Se crean los mutexs de los distintos archivos que se alteran, bitacoras no necesitan por ser propias a cada tripulante (puede que se requiera un mutex para la lista)
	pthread_mutex_init(&mutex_oxigeno, NULL);
	pthread_mutex_init(&mutex_comida, NULL);
	pthread_mutex_init(&mutex_basura, NULL);
	// mutex_blocks

	pthread_t hilo_escucha;
	pthread_create(&hilo_escucha, NULL, (void*) escuchar_mongo, (void*) &args_escuchar);
	pthread_detach(hilo_escucha);

	while(sistema_activo){
		sleep(1);
	}

	// Se cierran vestigios del pasado
	cerrar_archivos();
	cerrar_mutexs();
	close(socket_oyente);
	list_destroy(bitacoras);
	log_info(logger_mongo, "El I_Mongo_Store finalizo su ejecucion.\n");
	log_destroy(logger_mongo);
	config_destroy(config_mongo);
	config_destroy(config_superbloque);
	free(path_superbloque);
	free(path_blocks);
	free(path_files);
	free(path_basura);
	free(path_comida);
	free(path_oxigeno);
	free(path_bitacoras);
}

void escuchar_mongo(void* args) {

    args_escuchar* p = args;
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

       		es_discordiador = 0; // Se cambia flujo para que los subsiguientes sean tripulantes
       		socket_discordiador = socket_especifico;

       		pthread_t hilo_disc;
       		pthread_create(&hilo_disc, NULL, (void*) manejo_discordiador, NULL);
       		pthread_detach(hilo_disc);

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
	}
}

void manejo_discordiador(){
	t_estructura* mensaje;

	int flag = 1;

	while(flag) {
		mensaje = recepcion_y_deserializacion(socket_discordiador);

		switch(mensaje->codigo_operacion) {
			case PEDIR_BITACORA:
				log_trace(logger_mongo, "Nos piden una bitacora");
				t_bitacora* bitacora_tripulante = obtener_bitacora(mensaje->tid);
				if(bitacora_tripulante != NULL){
					char* bitacora = rescatar_bitacora(bitacora_tripulante->path);
					if(bitacora == NULL){
						log_debug(logger_mongo, "El tripulante no tenia nada en la bitacora.");
						enviar_codigo(FALLO, socket_discordiador);
						log_debug(logger_mongo, "Envio un fallo.");

					} else {
						log_debug(logger_mongo, "Conseguimos la bitacora.");
						t_archivo_tareas texto_archivo;
						texto_archivo.texto = malloc(strlen(bitacora) + 1);
						strcpy(texto_archivo.texto, bitacora);
						log_debug(logger_mongo, "La bitacora tiene: %s.", texto_archivo.texto);
						texto_archivo.largo_texto = strlen(bitacora);

						t_buffer* b_bitacora = serializar_archivo_tareas(texto_archivo);
						empaquetar_y_enviar(b_bitacora, BITACORA, socket_discordiador);
						log_debug(logger_mongo, "Enviada bitacora del tripulante %i", mensaje->tid);
					}
				} else {
					log_debug(logger_mongo, "El tripulante no tenia bitacora.");
					enviar_codigo(FALLO, socket_discordiador);
				}
				break;

			case DESCONEXION:
				log_info(logger_mongo, "Se desconecto un cliente.");
				flag = 0;
				log_error(logger_mongo, "Se desconecto un cliente.");
				sistema_activo = 0;
				// close(socket_discordiador);
				break;

			default:
				log_info(logger_mongo, "Se recibio un codigo invalido.");
				log_info(logger_mongo, "El codigo es %d", socket_discordiador);
				break;
		}
		free(mensaje);
	}
}

// TODO: Se ejecutaria aunque no haya un tripu disponible en discordiador
void sabotaje(int n) {

	// Se espera que set reciba la signal correspondiente
	if (n == SIGUSR1) {
		// log_error(logger_mongo, "Se detecto un sabotaje.\n");
		// Se avisa y se espera a Discordiador que tome las acciones correspondientes al sabotaje
		enviar_posicion_sabotaje(socket_discordiador);

		// Se activaria el protocolo fcsk
		reparar();

		log_warning(logger_mongo, "Se reparo el sabotaje.");

	}

}

void iniciar_file_system() {
	log_trace(logger_mongo, "INICIO iniciar_file_system");
	// Se crea estructura para verificar directorios
	struct stat dir = {0};

	// Se obtiene el path donde estara el arbol de directorios, de config
	path_directorio = PUNTO_MONTAJE;

	// Se settea el path a files, carpeta dentro del punto de montaje
	path_files = malloc(strlen(path_directorio) + strlen("/Files") + 1);
	strncpy(path_files, path_directorio, strlen(path_directorio) + 1);
	path_files = strcat(path_files, "/Files");

	// Se settea el path a bitacoras, carpeta dentro de files
	path_bitacoras = malloc(strlen(path_files) + strlen("/Bitacoras") + 1);
	strncpy(path_bitacoras, path_files, strlen(path_files) + 1);
	path_bitacoras = strcat(path_bitacoras, "/Bitacoras");

	// Se verifica si ya tengo carpetas hechas, o sea, un filesystem
	if ((stat(path_directorio, &dir) != -1)) {
		log_info(logger_mongo, "Se detecto un FileSystem existente.\n");
		inicializar_archivos_preexistentes();
	}
	else {
		// Como no hay carpetas, se crean
		mkdir(path_directorio, (mode_t) 0777);
		mkdir(path_files, (mode_t) 0777);
		mkdir(path_bitacoras, (mode_t) 0777);
		log_info(logger_mongo, "Se creo un FileSystem.\n");
		// Se asignan los archivos como antes
		inicializar_archivos();
	}

	pthread_t un_hilo; // Estaria bueno crearlo en main
	pthread_create(&un_hilo, NULL, (void*) sincronizar_blocks, NULL);
	pthread_detach(un_hilo);
	//log_trace(logger_mongo, "FIN iniciar_file_system");
}

void sincronizar_blocks() {

	while(1) {
		memcpy(mapa, directorio.mapa_blocks, CANTIDAD_BLOQUES * TAMANIO_BLOQUE);
		msync(mapa, CANTIDAD_BLOQUES * TAMANIO_BLOQUE, MS_SYNC);
		for(int i = 0; i < TIEMPO_SINCRONIZACION; i++){
			sleep(1);
			if(!sistema_activo){
				pthread_exit(NULL);
			}
		}
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

void liberar_lista(t_list* lista){
	if(list_is_empty(lista)){
		list_destroy_and_destroy_elements(lista, free);
	}else{
		list_destroy(lista);
	}
}
