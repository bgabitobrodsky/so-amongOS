/*
 * utils.c
 *
 *  Created on: 8 may. 2021
 *      Author: utnso
 */

#include "utils.h"

int reconocer_comando(char* str){

	char** palabras = string_split(str, " ");
	int contador = 0; //contador de palabras

	while(palabras[contador] != NULL){ // la ultima palabra es NULL
		contador++;
	}

	if(comparar_strings(palabras[0],"INICIAR_PATOTA")){
		if(contador >= 3){
			free(palabras);
			return INICIAR_PATOTA;
		}else{
			printf("Error de parametros: INICIAR_PATOTA <cantidad_de_tripulantes> <path>(<pos1> ... <posn>)\n");
		}
	}

	if(comparar_strings(palabras[0],"LISTAR_TRIPULANTES")){
		if(contador == 1){
			free(palabras);
			return LISTAR_TRIPULANTES;
		}else{
			printf("Error de parametros: LISTAR_TRIPULANTES\n");
		}
	}

	if(comparar_strings(palabras[0],"EXPULSAR_TRIPULANTE")){
		if(contador == 2){
			free(palabras);
			return EXPULSAR_TRIPULANTE;
		}else{
			printf("Error de parametros: EXPULSAR_TRIPULANTE <codigo_de_tripulante>\n");
		}
	}

	if(comparar_strings(palabras[0],"INICIAR_PLANIFICACION")){
		if(contador == 1){
			free(palabras);
			return INICIAR_PLANIFICACION;
		}else{
			printf("Error de parametros: INICIAR_PLANIFICACION\n");
		}
	}

	if(comparar_strings(palabras[0],"PAUSAR_PLANIFICACION")){
		if(contador == 1){
			free(palabras);
			return PAUSAR_PLANIFICACION;
		}else{
			printf("Error de parametros: PAUSAR_PLANIFICACION\n");
		}
	}

	if(comparar_strings(palabras[0],"OBTENER_BITACORA")){
		if(contador == 1){
			free(palabras);
			return OBTENER_BITACORA;
		}else{
			printf("Error de parametros: OBTENER_BITACORA <codigo_de_tripulante>\n");
		}
	}

	if(comparar_strings(palabras[0],"HELP")){
		if(contador == 1){
			free(palabras);
			return HELP;
		}else{
			printf("Error de parametros: HELP\n");
		}
	}

	if(comparar_strings(palabras[0],"EXIT")){
		if(contador == 1){
			free(palabras);
			return EXIT;
		}else{
			printf("Error de parametros: EXIT\n");
		}
	}

	free(palabras);
	printf("Comando desconocido, escribe HELP para obtener la lista de comandos\n");
	return NO_CONOCIDO;
}


int comparar_strings(char* str, char* str2){
	return !strncmp(str, str2, strlen(str2));
}

void help_comandos(){
	printf("Lista de comandos:\n");
	printf("- INICIAR_PATOTA <cantidad_de_tripulantes> <path>(<pos1> ... <posn>)\n");
	printf("- INICIAR_PLANIFICACION\n");
	printf("- PAUSAR_PLANIFICACION\n");
	printf("- LISTAR_TRIPULANTES\n");
	printf("- EXPULSAR_TRIPULANTE <codigo_de_tripulante>\n");
	printf("- OBTENER_BITACORA <codigo_de_tripulante>\n");
}


// Funcion para crear un socket modalidad cliente
int conectar_a_mi_ram_hq() { 
	struct addrinfo datos_para_server, *informacion_server;
	int estado;
	int socket_cliente;
	char* ip = config_get_string_value(config, "IP_MI_RAM_HQ");
	char* puerto = config_get_string_value(config, "PUERTO_MI_RAM_HQ");

	memset(&datos_para_server, 0, sizeof(datos_para_server));
	datos_para_server.ai_family = AF_UNSPEC;
	datos_para_server.ai_socktype = SOCK_STREAM;
	datos_para_server.ai_flags = AI_PASSIVE;

	// Obtengo la informacion del server y la alojo en informacion_server, utilizando los datos predefinidos arriba para settear
	if((estado = getaddrinfo(ip, puerto, &datos_para_server, &informacion_server)) != 0)
		log_warning(logger, "Error al conseguir informacion de MI RAM HQ\n");

	if((socket_cliente = socket(informacion_server -> ai_family, informacion_server -> ai_socktype, informacion_server -> ai_protocol)) != -1){
		log_warning(logger, "Error al crear socket cliente a MI RAM HQ\n");
	}

	if(connect(socket_cliente, informacion_server -> ai_addr, informacion_server -> ai_addrlen) == -1) { 
		log_warning(logger, "Error al conectar a MI RAM HQ\n");
	}
	freeaddrinfo(informacion_server);

	return socket_cliente;
}

void* serializar_paquete(t_paquete* paquete, int bytes){
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}
