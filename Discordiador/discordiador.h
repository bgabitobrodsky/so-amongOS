/*
 * Discordiador.h
 *
 *  Created on: 25 abr. 2021
 *      Author: utnso
 */

#ifndef DISCORDIADOR_H_
#define DISCORDIADOR_H_

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
	HELP
	
} comando_cod;

extern t_config* config;
extern t_log* logger;

int reconocer_comando(char* str);
int comparar_strings(char* str, char* str2);
void help_comandos();
void leer_consola();
void iniciar_patota(char* leido);
void iniciar_planificacion();
void listar_tripulantes();
void pausar_planificacion();
void obtener_bitacora(char* leido);
int pedir_tarea(int id_tripulante);
void realizar_tarea(t_tarea tarea);
char* fecha_y_hora();



//HILOS
void expulsar_tripulante(char* leido);
void iniciar_hilo_tripulante(void* funcion);
t_TCB* crear_tcb(t_PCB* pcb, int tid, char* posicion);
t_tripulante* crear_tripulante(t_TCB* un_tcb);
t_tripulante* iniciar_tripulante(void* funcion, t_PCB* pcb, int tid, char* posicion);

//PROCESOS
t_patota* crear_patota(t_PCB* un_pcb);
t_PCB* crear_pcb(char* path);
int nuevo_pid();
//int pids_contiene(int valor); // se puede eliminar, ya es lista

int esta_en_lista(t_list* lista, int elemento);
int sonIguales(int elemento1, int elemento2);



#endif /* DISCORDIADOR_H_ */
