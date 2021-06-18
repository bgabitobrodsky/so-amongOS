#include "mongo_tripulantes.h"

void manejo_tripulante(int socket_tripulante) { // TODO: Ver si le agrada al enunciado la implementacion
	while(1) {
		t_estructura* mensaje = recepcion_y_deserializacion(socket_tripulante);

		if (mensaje->codigo_operacion == PRIMERA_CONEXION) { // TODO: Agregar codigo
			crear_estructuras_tripulante(mensaje->tcb, socket_tripulante); // TODO: Ver como se mandan tripulantes
			log_info(logger_mongo, "Se creo la bitacora del tripulante %s.\n", string_itoa(mensaje->tcb->TID));
			free(mensaje->tcb);
		} 	
		else {
			if (mensaje->codigo_operacion > BASURA && mensaje->codigo_operacion <= SABOTAJE) {
				modificar_bitacora(mensaje->codigo_operacion, mensaje->tcb);
				log_info(logger_mongo, "Se modifico la bitacora del tripulante %s.\n", string_itoa(mensaje->tcb->TID));
				free(mensaje->tcb);
			}
			else {
				alterar(mensaje->codigo_operacion, mensaje->cantidad); 
			}
		}

		if (mensaje->codigo_operacion == DESCONEXION) { // Tripulante avisa desconexion para finalizar proceso
			borrar_bitacora(mensaje->tcb);
			log_info(logger_mongo, "Se desconecto el tripulante %s.\n", string_itoa(mensaje->tcb->TID));
			free(mensaje);
			exit(0);
		}

		free(mensaje);		
	}
}

void crear_estructuras_tripulante(t_TCB* tcb, int socket_tripulante) { // TODO: Verificar estructura, funcion boceto
	char* path_bitacoras = config_get_string_value(config_mongo, "PUNTO_MONTAJE");
	sprintf(path_bitacoras, "/Files/Bitacoras");
	
	char* path_tripulante = malloc(strlen(path_bitacoras) + strlen("/Tripulante%s.ims") + 1);
	sprintf(path_tripulante, "%s/Tripulante%s.ims", path_bitacoras, string_itoa(tcb->TID)); // TODO: Revisar funcionamiento de esta linea y ver identificador

	int file_descriptor_tripulante = open(path_tripulante, O_RDWR | O_APPEND | O_CREAT);

	FILE* file_tripulante = fdopen(file_descriptor_tripulante, "r+");
	
	acomodar_bitacora(file_tripulante, tcb);
}

void acomodar_bitacora(FILE* file_tripulante, t_TCB* tcb) {
	t_bitacora* nueva_bitacora = malloc(sizeof(t_bitacora));
	nueva_bitacora->bitacora_asociada = file_tripulante;
	nueva_bitacora->tripulante = tcb;

	list_add(bitacoras, nueva_bitacora);
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
