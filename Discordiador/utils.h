/*
 * utils.h
 *
 *  Created on: 8 may. 2021
 *      Author: utnso
 */

#ifndef DISCORDIADOR_UTILS_H_
#define DISCORDIADOR_UTILS_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<commons/string.h>
#include<commons/log.h>
#include<commons/config.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

typedef enum{
	MENSAJE,
	PEDIR_TAREA
}op_code;

typedef struct{
	int size;
	void* stream;
} t_buffer;


typedef struct{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

typedef enum{
	NO_CONOCIDO,
	LISTAR_TRIPULANTES,
	INICIAR_PLANIFICACION,
	PAUSAR_PLANIFICACION,
	OBTENER_BITACORA,
	EXPULSAR_TRIPULANTE,
	INICIAR_PATOTA,
	EXIT,
	HELP
}comando_cod;

extern t_config* config;
extern t_log* logger;

int reconocer_comando(char* str);
int comparar_strings(char* str, char* str2);
void help_comandos();
int conectar_a_mi_ram_hq();
void* serializar_paquete(t_paquete* paquete, int bytes);

#endif /* DISCORDIADOR_UTILS_H_ */
