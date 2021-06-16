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
			if (mensaje->codigo_operacion >= BASURA && mensaje->codigo_operacion <= SABOTAJE) {
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
	
	char* path_tripulante;
	sprintf(path_tripulante, "%s/Tripulante%s.ims", path_bitacoras, string_itoa(tcb->TID)); // TODO: Revisar funcionamiento de esta linea y ver identificador

	int file_descriptor_tripulante = open(path_tripulante, O_RDWR | O_APPEND | O_CREAT);

	FILE* file_tripulante = fdopen(file_descriptor_tripulante, "r+");
	
	acomodar_bitacora(file_tripulante, tcb);
}

void acomodar_bitacora(FILE* file_tripulante, t_TCB* tcb) {
	int posicion = encontrar_posicion_libre();
	t_bitacora* bitacora_tripulante;
	bitacora_tripulante->bitacora_asociada = file_tripulante;
	bitacora_tripulante->tripulante = tcb;
	bitacoras[posicion] = bitacora_tripulante;
	posiciciones_bitacora[posicion] = 1;
}

void modificar_bitacora(int codigo_operacion, t_TCB* tcb) { // TODO: Definir comportamiento
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
	int posicion = encontrar_posicion_dado_tripulante(tcb);
	fclose(bitacoras[posicion]->bitacora_asociada);
	free(bitacoras[posicion]->tripulante);
	free(bitacoras[posicion]);
	posiciciones_bitacora[posicion] = 0;
}

int encontrar_posicion_libre() {
	for(int i; i < sizeof(posiciciones_bitacora)/sizeof(int); i++) {
		if (posiciciones_bitacora[i] == 0) {
			return i;
		}
	}
	return -1;
}

int encontrar_posicion_dado_tripulante(t_TCB* tcb) {
	for(int i; i < sizeof(bitacoras)/sizeof(t_bitacora); i++) {
		if (bitacoras[i]->tripulante->TID == tcb->TID) {
			return i;
		}
	}
	return -1;
}
