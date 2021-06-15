#include "mongo_tripulantes.h"

void manejo_tripulante(int socket_tripulante) { // TODO: Ver si le agrada al enunciado la implementacion
	while(1) {
		t_estructura* mensaje = recepcion_y_deserializacion(socket_discordiador);

		if (mensaje->codigo_operacion == PRIMERA_CONEXION) { // TODO: Agregar codigo
			crear_estructuras_tripulante(mensaje->tripulante, socket_tripulante); // TODO: Ver como se mandan tripulantes	
			free(mensaje->tripulante);
		} 	
		else {
			if (mensaje->codigo_operacion >= BASURA && mensaje->codigo_operacion <= SABOTAJE) {
				modificar_bitacora(mensaje->codigo_operacion, mensaje->tripulante);
				free(mensaje->tripulante);
			}
			else {
				alterar(mensaje->codigo_operacion, mensaje->cantidad); 
			}
		}

		if (mensaje->codigo_operacion == DESCONEXION) { // Tripulante avisa desconexion para finalizar proceso
			free(mensaje);
			exit(0);
		}

		free(mensaje);		
	}
}

void crear_estructuras_tripulante(t_tripulante* tripulante, int socket_tripulante) { // TODO: Verificar estructura, funcion boceto
	char* path_bitacoras = config_get_string_value(config_mongo, "PUNTO_MONTAJE");
	sprintf(path_bitacoras, "/Files/Bitacoras");
	
	char* path_tripulante;
	sprintf(path_tripulante, "%s/Triupulante%s.ims", path_bitacoras, tripulante.identifiacor); // TODO: Revisar funcionamiento de esta linea y ver identificador

	int file_descriptor_tripulante = open(path_tripulante, O_RDWR | O_APPEND | O_CREAT);

	FILE* file_tripulante = fdopen(file_descriptor_tripulante, "r+");

	enviar_y_empaquetar(serializar_file(file_tripulante), ARCHIVO, socket_tripulante); // TODO: Ver si tiene sentido que tripulante conozca su bitacora, agregar todas las cosas de esta linea
}

void modificar_bitacora(int codigo_operacion, t_tripulante* tripulante) { // TODO: Definir comportamiento
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