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
				modificar_bitacora(mensaje->codigo_operacion, mensaje->tcb);
				log_info(logger_mongo, "Se modifico la bitacora del tripulante %s.\n", string_itoa(mensaje->tcb->TID));
				free(mensaje->tcb);
			}
			// Si es otro codigo, debera ser un cambio (tal vez agregar comprobacion para evitar errores de los que codean el Discordiador)
			else {
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
	// Se obtiene el path donde se crean las bitacoras
	char* path_bitacoras = fpath_bitacoras();
	
	// Se obtiene el path particular del tripulante, identificado con su TID
	char* path_tripulante = fpath_tripulante(path_bitacoras, tcb);
	
	// Se crea el archivo del tripulante y se lo abre
	FILE* file_tripulante = fopen(path_tripulante, "w+");
	
	// Se lo guarda en la bitacora
	acomodar_bitacora(file_tripulante, tcb);
}

void acomodar_bitacora(FILE* file_tripulante, t_TCB* tcb) {
	// Se utiliza un struct que conoce al tripulante y a su archivo, para luego saber donde se realizan los cambios pedidos por el mismo
	t_bitacora* nueva_bitacora = malloc(sizeof(t_bitacora));
	nueva_bitacora->bitacora_asociada = file_tripulante;
	nueva_bitacora->tripulante = tcb;

	list_add(bitacoras, nueva_bitacora);

	// TODO: Darle al menos un bloque a bitacora
}

void modificar_bitacora(int codigo_operacion, t_TCB* tcb) { // TODO: Definir comportamiento
	int indice = obtener_indice_bitacora(tcb);
	t_bitacora* bitacora = list_get(bitacoras, indice);
	
	switch (codigo_operacion) {
		case MOVIMIENTO:
			break;
		case INICIO_TAREA:
			break;
		case FIN_TAREA:
			break;
		case CORRE_SABOTAJE:
			break;
		case RESUELVE_SABOTAJE:
			break;
	}
}

void borrar_bitacora(t_TCB* tcb) {
	int indice = obtener_indice_bitacora(tcb);

	t_bitacora* bitacora = list_remove(bitacoras, indice);

	fclose(bitacora->bitacora_asociada);
	free(bitacora->tripulante);
	free(bitacora);
}

int obtener_indice_bitacora(t_TCB* tcb) { // TODO: Implementar
	return 0;
}

char* fpath_bitacoras() {
	char* path_directorio = config_get_string_value(config_mongo, "PUNTO_MONTAJE");
	char* path_bitacoras = malloc((strlen(path_directorio)+1) + strlen("/Files/Bitacoras"));
	char* path_directorio_aux = malloc(strlen(path_directorio) + 1);
	strcpy(path_directorio_aux, path_directorio);
	path_bitacoras = strcat(path_directorio_aux, "/Files/Bitacoras");
	return path_bitacoras;
}

char* fpath_tripulante(char* path_bitacoras, t_TCB* tcb) {
	char* path_tripulante = malloc(strlen(path_bitacoras) + strlen("/Tripulante.ims") + sizeof(string_itoa(tcb->TID)) + 1);
	char* path_bitacoras_aux = malloc(strlen(path_bitacoras) + 1);
	strcpy(path_bitacoras_aux, path_bitacoras);
	path_tripulante = strcat(path_bitacoras_aux, "/Tripulante");
	path_tripulante = strcat(path_tripulante, string_itoa(tcb->TID));
	path_tripulante = strcat(path_tripulante, ".ims");
	return path_tripulante;
}
