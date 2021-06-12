/*
 * utils.h
 *
 *  Created on: 8 may. 2021
 *      Author: utnso
 */

#ifndef DISCORDIADOR_UTILS_H_
#define DISCORDIADOR_UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <comms/estructuras.h>
#include <comms/paquetes.h>
#include <comms/socketes.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <readline/readline.h>
#include <time.h>
#include <stddef.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>

typedef enum{

	NO_CONOCIDO,
	LISTAR_TRIPULANTES,
	INICIAR_PLANIFICACION,
	PAUSAR_PLANIFICACION,
	OBTENER_BITACORA,
	EXPULSAR_TRIPULANTE,
	INICIAR_PATOTA,
	EXIT,
	HELP,
	APAGAR_SISTEMA
	
} comando_cod;

extern t_config* config;
extern t_log* logger;

extern t_list* lista_tripulantes_new;
extern t_list* lista_tripulantes_exec;
extern t_list* lista_pids;
extern t_list* lista_patotas;

extern t_queue* cola_tripulantes_new;
extern t_queue* cola_tripulantes_ready;

int reconocer_comando(char* str);
int comparar_strings(char* str, char* str2);
void help_comandos();
void iniciar_listas();
void iniciar_colas();


#endif /* DISCORDIADOR_UTILS_H_ */
