#include "mongo_tripulantes.h"

extern pthread_mutex_t sem_bitacoras;

void manejo_tripulante(void* socket) {

	int socket_tripulante = ((hilo_tripulante*) socket)->socket;
	char* posicion_tripulante;
	t_TCB* tripulante;

	while(1) {
		// Se espera a ver que manda el tripulante
		log_info(logger_mongo, "Se espera mensaje del tripulante.");

		t_estructura* mensaje = recepcion_y_deserializacion(socket_tripulante);
		// log_trace(logger_mongo, "El código recibido es: %i", mensaje->codigo_operacion);

		// Si es primera conexion, se crea la bitacora y se asigna a la lista
		if (mensaje->codigo_operacion == RECIBIR_TCB) {
		    log_info(logger_mongo, "Pedido de crear bitacora del tripulante %i.", mensaje->tcb->TID);
		    posicion_tripulante = formatear_posicion(mensaje->tcb->coord_x, mensaje->tcb->coord_y);
		    log_info(logger_mongo, "La posicion inicial del tripulante es: %c|%c", posicion_tripulante[0], posicion_tripulante[2]);
			crear_estructuras_tripulante(mensaje->tcb, socket_tripulante);
			log_trace(logger_mongo, "Se creo la bitacora del tripulante %i.", mensaje->tcb->TID);
			tripulante = mensaje->tcb;
		}
		// Si no lo es, puede ser o agregar/quitar recursos o cambiar informacion en la bitacora
		else {
			// Codigos mayores a Basura y menores a Sabotaje corresponden a asignaciones de bitacora
			if (mensaje->codigo_operacion > BASURA && mensaje->codigo_operacion < SABOTAJE) {
				log_info(logger_mongo, "Pedido de modificar bitacora.");
				modificar_bitacora(mensaje, &posicion_tripulante, socket_tripulante);
				log_info(logger_mongo, "Se modifico la bitacora.");
			}

			// Si es otro codigo
			else if(mensaje->codigo_operacion > BITACORA && mensaje->codigo_operacion < MOVIMIENTO){
				log_info(logger_mongo, "Pedido alterar la cantidad de recurso %s", conseguir_tipo(conseguir_char(mensaje->codigo_operacion))); // Revisar
				log_trace(logger_mongo, "Numero de codigo: %i", mensaje->codigo_operacion);
				alterar(mensaje->codigo_operacion, mensaje->cantidad); 
			}
		}

		// Ultimo mensaje del tripulante, al morir o algo, sera la desconexion, lo cual borra la bitacora y libera los recursos
		if (mensaje->codigo_operacion == DESCONEXION) { // Tripulante avisa desconexion para finalizar proceso
			log_info(logger_mongo, "Se desconecto un tripulante.");
			free(posicion_tripulante);
			free(mensaje);
			free(socket);
			// Aca finalizaria el hilo creado por el tripulante al conectarse a Mongo
			pthread_exit(NULL);
		}

		free(mensaje);
	}
}

char* rescatar_bitacora(char* path){
	log_trace(logger_mongo, "Rescatando bitacora.");
	t_list* lista_bloques_bitacora = get_lista_bloques(path);

	int lectura = 0;
	int size = tamanio_archivo(path);

	if(size == 0){
		return NULL;
	}

	char* string = malloc(size + 1);
	// log_trace(logger_mongo, "Tamanio de la bitacora es: %i", size);
	int* aux;

	// lockearLectura(path_blocks);

	for(int i = 0; i < list_size(lista_bloques_bitacora); i++){
		aux = list_get(lista_bloques_bitacora, i);

		for(int j = 0; j < TAMANIO_BLOQUE; j++) {
			string[lectura] = *(directorio.mapa_blocks + (TAMANIO_BLOQUE * *aux) + j);
			lectura++;

			if(lectura == size){
				string[size] = '\0';
				// unlockear(path_blocks);

				return string;
			}
		}
	}

	string[size] = '\0';
	// unlockear(path_blocks);

	return string;
}

void crear_estructuras_tripulante(t_TCB* tcb, int socket_tripulante) {
	// Se obtiene el path particular del tripulante, identificado con su TID
    log_trace(logger_mongo, "Creando estructuras para el tripulante.");

	char* path_tripulante = fpath_tripulante(tcb);

	// Se crea el archivo del tripulante y se lo abre
	FILE* file_tripulante = fopen(path_tripulante, "w+");
	
	//Se inicializan los datos del tripulante
	escribir_archivo_tripulante(path_tripulante, 0, NULL);

	// Se lo guarda en la bitacora
	acomodar_bitacora(file_tripulante, path_tripulante, tcb);

}

void acomodar_bitacora(FILE* file_tripulante, char* path_tripulante, t_TCB* tcb) {
	// Se utiliza un struct que conoce al tripulante y a su archivo, para luego saber donde se realizan los cambios pedidos por el mismo
    log_trace(logger_mongo, "Acomodando bitacora.");
	t_bitacora* nueva_bitacora = malloc(sizeof(t_bitacora));

	nueva_bitacora->bitacora_asociada = file_tripulante;
	nueva_bitacora->path = path_tripulante;
	nueva_bitacora->bloques = list_create();
	nueva_bitacora->tripulante = tcb;

	monitor_lista(sem_bitacoras, (void*) list_add, bitacoras, nueva_bitacora);

	asignar_nuevo_bloque(nueva_bitacora->path, 0);
}

void modificar_bitacora(t_estructura* mensaje, char** posicion, int socket) {

	t_bitacora* bitacora = obtener_bitacora(mensaje->tcb->TID);
	char* pos_inicial;
	char* pos_final;
	char* nombre_tarea;
	char* cadenita;
	int largo_cadenita;
	t_estructura* mensaje_tarea;

	switch (mensaje->codigo_operacion) {
		case MOVIMIENTO:
			log_trace(logger_mongo, "Me llega un movimiento de %i", mensaje->tcb->TID);
			pos_inicial = malloc(sizeof(char)*3 + 1);
			strcpy(pos_inicial, *posicion);
			pos_final = formatear_posicion(mensaje->tcb->coord_x, mensaje->tcb->coord_y);

			cadenita = malloc(strlen("Se mueve de ") + strlen(" a ") + 2*strlen(pos_final) + 1 + 1);
			strcpy(cadenita, "Se mueve de ");
			strcat(cadenita, pos_inicial);
			strcat(cadenita, " a ");
			strcat(cadenita, pos_final);
			strcat(cadenita, ".");

			escribir_bitacora(bitacora, cadenita);
			largo_cadenita = strlen(cadenita);
			free(pos_inicial);
			free(pos_final);
			free(mensaje->tcb);

			break;

		case INICIO_TAREA:
			log_trace(logger_mongo, "Inicio de tarea de %i", mensaje->tcb->TID);
			free(mensaje->tcb);
			mensaje_tarea = recepcion_y_deserializacion(socket);
			nombre_tarea = malloc(strlen(mensaje_tarea->tarea->nombre) + 1);
			strcpy(nombre_tarea, mensaje_tarea->tarea->nombre);

			cadenita = malloc(strlen("Comienza ejecucion de tarea ") + strlen(nombre_tarea) + 1 + 1);
			strcpy(cadenita, "Comienza ejecucion de tarea ");
			strcat(cadenita, nombre_tarea);
			strcat(cadenita, ".");

			escribir_bitacora(bitacora, cadenita);
			largo_cadenita = strlen(cadenita);
			free(nombre_tarea);
			free(mensaje_tarea->tarea->nombre);
			free(mensaje_tarea->tarea);
			free(mensaje_tarea);

			break;

		case FIN_TAREA:
			log_trace(logger_mongo, "Fin de tarea de %i", mensaje->tcb->TID);
			free(mensaje->tcb);
			mensaje_tarea = recepcion_y_deserializacion(socket);
			nombre_tarea = malloc(strlen(mensaje_tarea->tarea->nombre) + 1);
			strcpy(nombre_tarea, mensaje_tarea->tarea->nombre);

			cadenita = malloc(strlen("Se finaliza la tarea ") + strlen(nombre_tarea) + 1 + 1);
			strcpy(cadenita, "Se finaliza la tarea ");
			strcat(cadenita, nombre_tarea);
			strcat(cadenita, ".");

			escribir_bitacora(bitacora, cadenita);
			largo_cadenita = strlen(cadenita);
			free(nombre_tarea);
			free(mensaje_tarea->tarea->nombre);
			free(mensaje_tarea->tarea);
			free(mensaje_tarea);

			break;

		case CORRE_SABOTAJE:
			log_trace(logger_mongo, "%i corre hacia el sabotaje.", mensaje->tcb->TID);
			cadenita = malloc(strlen("Se corre en panico a la ubicacion del sabotaje.") + 1);
			strcpy(cadenita, "Se corre en panico a la ubicacion del sabotaje.");

			largo_cadenita = strlen(cadenita);
			escribir_bitacora(bitacora, cadenita);
			free(mensaje->tcb);

			break;

		case RESUELVE_SABOTAJE:
			log_trace(logger_mongo, "Resuelve sabotaje %i", mensaje->tcb->TID);
			cadenita = malloc(strlen("Se resuelve el sabotaje.") + 1);
			strcpy(cadenita, "Se resuelve el sabotaje.");

			largo_cadenita = strlen(cadenita);
			escribir_bitacora(bitacora, cadenita);
			free(mensaje->tcb);

			break;
	}

	log_trace(logger_mongo, "Mensaje agregado a bitacora: %s", cadenita);
	free(cadenita);

	//Actualizo struct bitacora
	uint32_t tamanio = tamanio_archivo(bitacora->path);

	// bitacora->bloques; Realmente no la usamos nunca, mejor dejarlo asi
	bitacora->tamanio = tamanio + largo_cadenita;

	set_tam(bitacora->path, tamanio + largo_cadenita);
}

void escribir_bitacora(t_bitacora* bitacora, char* mensaje) {

	t_list* lista_bloques = get_lista_bloques(bitacora->path);

	if (list_is_empty(lista_bloques)){
		log_trace(logger_mongo, "La lista de bloques esta vacia.");
		log_trace(logger_mongo, "Se asigna un nuevo bloque..");
		list_destroy(lista_bloques);
		asignar_nuevo_bloque(bitacora->path, strlen(mensaje));
		lista_bloques = get_lista_bloques(bitacora->path);
	}

	escribir_bloque_bitacora(mensaje, bitacora);

	matar_lista(lista_bloques);
}

void escribir_bloque_bitacora(char* mensaje, t_bitacora* bitacora) {

	int cantidad_alcanzada = 0;
	t_list* lista_bloques = get_lista_bloques(bitacora->path);

	int* aux;
	lockearLectura(path_blocks);

	for(int i = 0; i < list_size(lista_bloques); i++){

		aux = list_get(lista_bloques, i);

		for(int j = 0; j < TAMANIO_BLOQUE; j++){

			if (cantidad_alcanzada == strlen(mensaje)) {
				break;
			}

			if (*(directorio.mapa_blocks + *aux * TAMANIO_BLOQUE + j) == ',') {
				*(directorio.mapa_blocks + *aux * TAMANIO_BLOQUE + j) = mensaje[cantidad_alcanzada];

				cantidad_alcanzada++;
			}
		}
	}

	unlockear(path_blocks);

	matar_lista(lista_bloques);

	if (cantidad_alcanzada != strlen(mensaje)) {
		log_trace(logger_mongo, "Falta escribir parte del mensaje. ");
		log_trace(logger_mongo, "Alcance %i bytes de %i bytes, ", cantidad_alcanzada, strlen(mensaje));
		// el size lo podria dejar aca, y no pasar por param
		asignar_nuevo_bloque(bitacora->path, cantidad_alcanzada);
		char* resto_mensaje = malloc(strlen(mensaje + cantidad_alcanzada) + 1);
		strcpy(resto_mensaje, (mensaje + cantidad_alcanzada));
		// log_trace(logger_mongo, "El resto del mensaje sera: %s", resto_mensaje);
		escribir_bitacora(bitacora, resto_mensaje);
		free(resto_mensaje);
	}
}

char* formatear_posicion(int coord_x, int coord_y) {

	char* posicion_formateada = malloc(sizeof(char)*3 + 1);

	char* aux = string_itoa(coord_x);
	strcpy(posicion_formateada, aux);
	free(aux);
	strcat(posicion_formateada, "|");
	aux = string_itoa(coord_y);
	strcat(posicion_formateada, aux);
	free(aux);

	log_trace(logger_mongo, "La posicion formateada resulta %s", posicion_formateada);

	return posicion_formateada;
}

void borrar_bitacora(t_TCB* tcb) {

	t_bitacora* bitacora = quitar_bitacora_lista(tcb);
	remove(bitacora->path);
	fclose(bitacora->bitacora_asociada);
	free(bitacora->tripulante);
	free(bitacora->path);
	free(bitacora);
}

t_bitacora* quitar_bitacora_lista(t_TCB* tcb) {

	bool contains(void* bitacora) {
		return (tcb->TID == ((t_bitacora*) bitacora)->tripulante->TID);
	}

	t_bitacora* bitacora = list_remove_by_condition(bitacoras, contains);

	return bitacora;
}

t_bitacora* obtener_bitacora(int tid) {

	bool contains(void* bitacora) {
		return (tid == ((t_bitacora*) bitacora)->tripulante->TID);
	}

	t_bitacora* bitacora = list_find(bitacoras, contains);

	if(bitacora == NULL){
		log_warning(logger_mongo, "No se encontro la bitacora asociada al tripulante %i", tid);
		return NULL;
	}

	return bitacora;
}

char* fpath_tripulante(t_TCB* tcb) {
	char* string_aux = string_itoa((int) (tcb->TID));
	char* path_tripulante = malloc(strlen(path_bitacoras) + strlen("/Tripulante.ims") + strlen(string_aux) + 1);
	strcpy(path_tripulante, path_bitacoras);
	path_tripulante = strcat(path_tripulante, "/Tripulante");
	path_tripulante = strcat(path_tripulante, string_aux);
	path_tripulante = strcat(path_tripulante, ".ims");
	free(string_aux);

	return path_tripulante;
}
