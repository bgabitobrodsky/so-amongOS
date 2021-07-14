#include "mongo_tripulantes.h"

void manejo_tripulante(void* socket) {
	int socket_tripulante = ((hilo_tripulante*) socket)->socket;

	while(1) {
		// Se espera a ver que manda el tripulante
		log_error(logger_mongo, "espero mensaje ");

		t_estructura* mensaje = recepcion_y_deserializacion(socket_tripulante);

		// Si es primera conexion, se crea la bitacora y se asigna a la lista
		if (mensaje->codigo_operacion == RECIBIR_TCB) {
		    log_trace(logger_mongo, "entro en el if crear tripu");

			crear_estructuras_tripulante(mensaje->tcb, socket_tripulante);
			log_info(logger_mongo, "Se creo la bitacora del tripulante %i.\n", mensaje->tcb->TID);
			free(mensaje->tcb);
		}
		// Si no lo es, puede ser o agregar/quitar recursos o cambiar informacion en la bitacora
		else {
			// Codigos mayores a Basura y menores a Sabotaje corresponden a asignaciones de bitacora
			if (mensaje->codigo_operacion > BASURA && mensaje->codigo_operacion < SABOTAJE) {

				log_error(logger_mongo, "IF ENTRE ");
				t_estructura* mensaje2 = recepcion_y_deserializacion(socket_tripulante); // Aca recibe la info adicional
				log_error(logger_mongo, "recibo");
				modificar_bitacora(mensaje, mensaje2);
				log_info(logger_mongo, "Se modifico la bitacora del tripulante %s.\n", string_itoa(mensaje->tcb->TID));
				free(mensaje->tcb);
			}

			// Si es otro codigo
			else if(mensaje->codigo_operacion > TAREA && mensaje->codigo_operacion < MOVIMIENTO){
				log_error(logger_mongo, "Toy por alterar");
				alterar(mensaje->codigo_operacion, mensaje->cantidad); 
			}
		}

		// Ultimo mensaje del tripulante, al morir o algo, sera la desconexion, lo cual borra la bitacora y libera los recursos
		if (mensaje->codigo_operacion == DESCONEXION) { // Tripulante avisa desconexion para finalizar proceso
			borrar_bitacora(mensaje->tcb);
			log_info(logger_mongo, "Se desconecto un tripulante.\n");
			free(mensaje);

			// Aca finalizaria el hilo creado por el tripulante al conectarse a Mongo
			pthread_exit(0);
		}

		free(mensaje);		
	}
}

void crear_estructuras_tripulante(t_TCB* tcb, int socket_tripulante) {
	// Se obtiene el path particular del tripulante, identificado con su TID
    log_trace(logger_mongo, "-1");

	char* path_tripulante = fpath_tripulante(tcb);
    log_trace(logger_mongo, "0");

	// Se crea el archivo del tripulante y se lo abre
	FILE* file_tripulante = fopen(path_tripulante, "w+");
    log_trace(logger_mongo, "1");
	
	//Se inicializan los datos del tripulante

	escribir_archivo_tripulante(file_tripulante, 0, NULL);

    log_trace(logger_mongo, "2");

	// Se lo guarda en la bitacora
	acomodar_bitacora(file_tripulante, tcb);
}

void acomodar_bitacora(FILE* file_tripulante, t_TCB* tcb) {
	// Se utiliza un struct que conoce al tripulante y a su archivo, para luego saber donde se realizan los cambios pedidos por el mismo
    log_trace(logger_mongo, "Acomodando bitacora");
	t_bitacora* nueva_bitacora = malloc(sizeof(t_bitacora));
	nueva_bitacora->bitacora_asociada = file_tripulante;
	nueva_bitacora->tripulante = tcb;

	list_add(bitacoras, nueva_bitacora);

	asignar_nuevo_bloque(nueva_bitacora->bitacora_asociada);
    log_trace(logger_mongo, "Acomodada bitacora");
}

void modificar_bitacora(t_estructura* mensaje1, t_estructura* mensaje2) { //TODO Necesitara recibir dos mensajes consecutivos, para volver a como estaba antes borrar 2do parametro
	t_bitacora* bitacora = obtener_bitacora(mensaje1->tcb);
	char* pos_inicial = NULL;
	char* pos_final = NULL;
	char* nombre_tarea;
	char* cadenita;
	
	switch (mensaje1->codigo_operacion) {
		case MOVIMIENTO:
			pos_inicial = formatear_posicion(mensaje2->posicion->coord_x, mensaje2->posicion->coord_y); // Ver como manejar pos inicial
			pos_final = formatear_posicion(mensaje1->tcb->coord_x, mensaje1->tcb->coord_y);
			cadenita = malloc(strlen("se mueve de ") + strlen(" a ") + 2*strlen(pos_final) + 1);
			strcpy(cadenita, "se mueve de ");
			strcat(cadenita, pos_inicial);
			strcat(cadenita, " a ");
			strcat(cadenita, pos_final);

			escribir_bitacora(bitacora, cadenita); // Implementar en t_estructura y crear posicion
			free(pos_inicial);
			free(pos_final);
			free(cadenita);
			break;
		case INICIO_TAREA:
			nombre_tarea = malloc(strlen(mensaje2->tarea->nombre) + 1);
			strcpy(nombre_tarea, mensaje2->tarea->nombre);

			cadenita = malloc(strlen("comienza ejecucion de tarea ") + strlen(nombre_tarea) + 1);
			strcpy(cadenita, "comienza ejecucion de tarea ");
			strcat(cadenita, nombre_tarea);

			escribir_bitacora(bitacora, cadenita);
			free(cadenita);
			free(nombre_tarea);
			break;
		case FIN_TAREA:
			nombre_tarea = malloc(strlen(mensaje2->tarea->nombre) + 1);
			strcpy(nombre_tarea, mensaje2->tarea->nombre);

			cadenita = malloc(strlen("se finaliza la tarea ") + strlen(nombre_tarea) + 1);
			strcpy(cadenita, "se finaliza la tarea ");
			strcat(cadenita, nombre_tarea);

			escribir_bitacora(bitacora, cadenita);
			free(cadenita);
			free(nombre_tarea);
			break;
		case CORRE_SABOTAJE:

			escribir_bitacora(bitacora, "se corre en panico a la ubicacion del sabotaje");
			break;
		case RESUELVE_SABOTAJE:
			escribir_bitacora(bitacora, "se resuelve el sabotaje");
			break;
	}

	//Actualizo struct bitacora
	t_list* lista_bloques = lista_bloques_tripulante(bitacora->bitacora_asociada);
	int tamanio = (int) tamanio_archivo(bitacora->bitacora_asociada);
	bitacora->bloques = lista_bloques;
	bitacora->tamanio = tamanio;
}

void escribir_bitacora(t_bitacora* bitacora, char* mensaje) {

	int size_lista_bloques = list_size(bitacora->bloques);
	int ultimo_bloque = (int) list_get(bitacora->bloques, size_lista_bloques); // TODO restar 1 o esta bien?

	escribir_bloque_bitacora(ultimo_bloque, mensaje, bitacora);

	free(mensaje);
}

void escribir_bloque_bitacora(int bloque, char* mensaje, t_bitacora* bitacora) {
	// TODO REVISAR
	int cantidad_alcanzada = 0;
	int i, j, t;
	j = 0; // TODO como funciona esto?
	t_list* lista_bloques = lista_bloques_tripulante(bitacora->bitacora_asociada);

	log_trace(logger_mongo, "Lista bloque primer elemento %i", list_get(lista_bloques, 0));

	for (i = 0; *(directorio.mapa_blocks + bloque * TAMANIO_BLOQUE + i + 1) != '\t'; i++) {

		if (*(directorio.mapa_blocks + (int)list_get(lista_bloques, j) * TAMANIO_BLOQUE + i) == '\t') {
			*(directorio.mapa_blocks + (int)list_get(lista_bloques, j) * TAMANIO_BLOQUE + i) = mensaje[i];
			cantidad_alcanzada++;
		}
	}

	if (cantidad_alcanzada != strlen(mensaje)) {
		asignar_nuevo_bloque(bitacora->bitacora_asociada);
		char* resto_mensaje = malloc(strlen(mensaje) - cantidad_alcanzada);

		for (j = cantidad_alcanzada, t = 0; j < strlen(mensaje); j++, t++) {
			resto_mensaje[t] = mensaje[j];
		}

		escribir_bitacora(bitacora, resto_mensaje);
	}
}

char* formatear_posicion(int coord_x, int coord_y) { // Puede generar memory leaks
	char* posicion_formateada = malloc(sizeof(char) * 3); // Ejemplo: 1|2, 3 chars
	strcat(posicion_formateada, string_itoa(coord_x));
	strcat(posicion_formateada, "|");
	strcat(posicion_formateada, string_itoa(coord_y));

	return posicion_formateada;
}

void borrar_bitacora(t_TCB* tcb) {
	t_bitacora* bitacora = quitar_bitacora_lista(tcb);

	fclose(bitacora->bitacora_asociada);
	free(bitacora->tripulante);
	free(bitacora);
}

t_bitacora* quitar_bitacora_lista(t_TCB* tcb) {

	bool contains(void* tcb1) {
		return (tcb == ((t_bitacora*) tcb1)->tripulante);
	}

	t_bitacora* bitacora = list_remove_by_condition(bitacoras, contains);
	return bitacora;
}

t_bitacora* obtener_bitacora(t_TCB* tcb) {

	bool contains(void* tcb1) {
		return (tcb == ((t_bitacora*) tcb1)->tripulante);
	}

	t_bitacora* bitacora = list_remove_by_condition(bitacoras, contains);
	list_add(bitacoras, bitacora);
	return bitacora;
}

char* fpath_tripulante(t_TCB* tcb) {

	char* path_tripulante = malloc(strlen(path_bitacoras) + strlen("/Tripulante.ims") + strlen(string_itoa((int) (tcb->TID))) + 1);
	strcpy(path_tripulante, path_bitacoras);
	path_tripulante = strcat(path_tripulante, "/Tripulante");
	path_tripulante = strcat(path_tripulante, string_itoa((int) (tcb->TID)));
	path_tripulante = strcat(path_tripulante, ".ims");
	log_debug(logger_mongo, "Nuevo path de bitacora: %s", path_tripulante);

	return path_tripulante;
}
