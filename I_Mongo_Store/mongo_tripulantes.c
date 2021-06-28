#include "mongo_tripulantes.h"

void manejo_tripulante(int socket_tripulante) {
	while(1) {
		// Se espera a ver que manda el tripulante
		t_estructura* mensaje = recepcion_y_deserializacion(socket_tripulante);

		// Si es primera conexion, se crea la bitacora y se asigna a la lista
		if (mensaje->codigo_operacion == RECIBIR_TCB) {
			crear_estructuras_tripulante(mensaje->tcb, socket_tripulante);
			log_info(logger_mongo, "Se creo la bitacora del tripulante %s.\n", string_itoa(mensaje->tcb->TID));
			free(mensaje->tcb);
		}
		// Si no lo es, puede ser o agregar/quitar recursos o cambiar informacion en la bitacora
		else {
			// Codigos mayores a Basura y menores a Sabotaje corresponden a asignaciones de bitacora
			if (mensaje->codigo_operacion > BASURA && mensaje->codigo_operacion < SABOTAJE) {
				// t_estructura* mensaje2 = recepcion_y_deserializacion(socket_tripulante); // Aca recibe la info adicional
				modificar_bitacora(mensaje); // modificar_bitacora(mensaje, mensaje2);
				log_info(logger_mongo, "Se modifico la bitacora del tripulante %s.\n", string_itoa(mensaje->tcb->TID));
				free(mensaje->tcb);
			}

			// Si es otro codigo
			else if(mensaje->codigo_operacion > TAREA && mensaje->codigo_operacion < MOVIMIENTO){
				alterar(mensaje->codigo_operacion, mensaje->cantidad); 
			}
		}

		// Ultimo mensaje del tripulante, al morir o algo, sera la desconexion, lo cual borra la bitacora y libera los recursos
		if (mensaje->codigo_operacion == DESCONEXION) { // Tripulante avisa desconexion para finalizar proceso
			borrar_bitacora(mensaje->tcb);
			log_info(logger_mongo, "Se desconecto el tripulante %s.\n", string_itoa(mensaje->tcb->TID));
			free(mensaje);

			// Aca finalizaria el hilo creado por el tripulante al conectarse a Mongo
			exit(0);
		}

		free(mensaje);		
	}
}

void crear_estructuras_tripulante(t_TCB* tcb, int socket_tripulante) { // TODO: Verificar estructura, funcion boceto.
	// Se obtiene el path particular del tripulante, identificado con su TID
	char* path_tripulante = fpath_tripulante(tcb);
	
	// Se crea el archivo del tripulante y se lo abre
	FILE* file_tripulante = fopen(path_tripulante, "w+");
	
	//Se inicializan los datos del tripulante
	escribir_archivo_tripulante(file_tripulante, 0, NULL);

	// Se lo guarda en la bitacora
	acomodar_bitacora(file_tripulante, tcb);
}

void acomodar_bitacora(FILE* file_tripulante, t_TCB* tcb) {
	// Se utiliza un struct que conoce al tripulante y a su archivo, para luego saber donde se realizan los cambios pedidos por el mismo
	t_bitacora* nueva_bitacora = malloc(sizeof(t_bitacora));
	nueva_bitacora->bitacora_asociada = file_tripulante;
	nueva_bitacora->tripulante = tcb;

	list_add(bitacoras, nueva_bitacora);

	asignar_nuevo_bloque(file_tripulante);

	// TODO: Asignar cosas en Struct bitacora
}

void modificar_bitacora(t_estructura* mensaje) { // Necesitara recibir dos mensajes consecutivos, parametros serian (t_estructura* mensaje1, t_estructura* mensaje2)
	t_bitacora* bitacora = obtener_bitacora(mensaje->tcb);
	char* pos_inicial = NULL;
	char* pos_final = NULL;
	char* nombre_tarea = NULL;
	
	switch (codigo_operacion) {
		case MOVIMIENTO:
			pos_inicial = formatear_posicion(mensaje->posicion->coord_x, mensaje->posicion->coord_y); // Ver como manejar pos inicial
			pos_final = formatear_posicion(mensaje->tcb->coord_x, mensaje->tcb->coord_y);
			escribir_bitacora(bitacora, strlen("Se mueve de a ") + sizeof(char) * 6, "Se mueve de %s a %s", pos_inicial, pos_final); // Implementar en t_estructura y crear posicion
			free(pos_inicial);
			free(pos_final)
			break;
		case INICIO_TAREA:
			char* nombre_tarea = mensaje->tarea->nombre;
			escribir_bitacora(bitacora, strlen("Comienza ejecucion de tarea ") + stlen(nombre_tarea), "Comienza ejecucion de tarea %s", nombre_tarea);
			free(nombre_tarea);
			break;
		case FIN_TAREA:
			char* nombre_tarea = mensaje->tarea->nombre;
			escribir_bitacora(bitacora, strlen("Se finaliza la tarea ") + stlen(nombre_tarea), "Se finaliza la tarea %s", nombre_tarea);
			free(nombre_tarea);
			break;
		case CORRE_SABOTAJE:
			escribir_bitacora(bitacora, strlen("Se corre en panico a la ubicacion del sabotaje"), "Se corre en panico a la ubicacion del sabotaje");
			break;
		case RESUELVE_SABOTAJE:
			escribir_bitacora(bitacora, strlen("Se resuelve el sabotaje"), "Se resuelve el sabotaje");
			break;
	}

	// TODO: Actualizar struct bitacora
}

void escribir_bitacora(t_bitacora* bitacora, int largo_strings, int cant_strings, ...) {
	va_list lista_argumentos;
	va_start(lista_argumentos, cant_strings);

	char* mensaje = malloc(largo_strings);

	for (int i; i < cant_strings; i++) {
		strcat(mensaje, va_arg(lista_argumentos, char*));
	}

	int size_lista_bloques = sizeof(bitacora->bloques)/sizeof(int); // Revisar
	int ultimo_bloque = bitacora->bloques[size_lista_bloques];

	escribir_bloque_bitacora(ultimo_bloque, mensaje);

	va_end(lista_argumentos);
	free(mensaje);
}

void escribir_bloque_bitacora(int bloque, char* mensaje) {
	// TODO: Implementar
}

char* formatear_posicion(int coord_x, int coord_y) { // Puede generar memory leaks
	char* posicion_formateada = malloc(sizeof(char) * 3); // Ejemplo: 1|2, 3 chars
	strcat(posicion_formateada, itoa(coord_x));
	strcat(posicion_formateada, '|');
	strcat(posicion_formateada, itoa(coord_y));

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
	char* path_tripulante = malloc(strlen(path_bitacoras) + strlen("/Tripulante.ims") + sizeof(string_itoa(tcb->TID)) + 1);
	char* path_bitacoras_aux = malloc(strlen(path_bitacoras) + 1);
	strcpy(path_bitacoras_aux, path_bitacoras);
	path_tripulante = strcat(path_bitacoras_aux, "/Tripulante");
	path_tripulante = strcat(path_tripulante, string_itoa(tcb->TID));
	path_tripulante = strcat(path_tripulante, ".ims");
	return path_tripulante;
}
