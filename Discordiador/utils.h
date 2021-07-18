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
#include <comms/generales.h>
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

enum tareas {
    GENERAR_OXIGENO, CONSUMIR_OXIGENO, GENERAR_COMIDA, CONSUMIR_COMIDA, GENERAR_BASURA, DESCARTAR_BASURA, OTRA_TAREA
};

enum estados { NEW, READY, EXEC, BLOCK, EXIT, PANIK};

typedef enum{

	NO_CONOCIDO,
	LISTAR_TRIPULANTES,
	INICIAR_PLANIFICACION,
	PAUSAR_PLANIFICACION,
	OBTENER_BITACORA,
	EXPULSAR_TRIPULANTE,
	INICIAR_PATOTA,
	HELP,
	APAGAR_SISTEMA
	
} comando_cod;

extern t_config* config;
extern t_log* logger;

extern char estado_tripulante[6];

extern t_list* lista_pids;
extern t_list* lista_patotas;
extern t_list* lista_tripulantes;
extern t_list* lista_tripulantes_new;
extern t_list* lista_tripulantes_exec;

extern t_queue* cola_tripulantes_ready;
extern t_queue* cola_tripulantes_block;
extern t_queue* cola_tripulantes_block_emergencia;

extern pthread_mutex_t sem_lista_tripulantes;
extern pthread_mutex_t sem_lista_new;
extern pthread_mutex_t sem_cola_ready;
extern pthread_mutex_t sem_lista_exec;
extern pthread_mutex_t sem_cola_block;
extern pthread_mutex_t sem_cola_block_emergencia;

// Funciones PRINCIPALES
int iniciar_patota(char* leido);
void listar_tripulantes();
void expulsar_tripulante(char* leido);
void iniciar_planificacion();
void pausar_planificacion();
void obtener_bitacora(char* leido);

// PROCESOS
t_patota* crear_patota(uint32_t un_pid);
int nuevo_pid();
void* eliminar_patota_de_lista(t_list* lista, int elemento);

// AUXILIARES
int reconocer_comando(char* str);
void help_comandos();
void iniciar_listas();
void iniciar_colas();
void iniciar_listas_();
void iniciar_colas_();
void iniciar_semaforos();
void enviar_archivo_tareas(char* archivo_tareas, int pid, int socket);
void pedir_tarea_a_mi_ram_hq(uint32_t tid, int socket);
void enviar_pid_a_ram(uint32_t pid, int socket);
int esta_pid_en_lista(t_list* lista, int elemento);

// SABOTAJES
void peligro(t_posicion* pos_sabotaje, int socket_ram);
void resolver_sabotaje(t_tripulante* un_tripulante, int socket_ram, int socket_mongo);
t_tripulante* tripulante_mas_cercano_a(t_posicion* posicion);
void guardian_sabotaje();

// TRIPULANTES
int esta_tripulante_en_lista(t_list* lista, int elemento);
void* eliminar_tripulante_de_lista(t_list* lista, int elemento);
void enviar_tripulante_a_ram (t_tripulante un_tripulante, int socket);
t_tripulante* crear_tripulante(int tid, int x, int y, char estado);
t_tripulante* crear_puntero_tripulante(uint32_t tid, char* posicion);
int soy_el_ultimo_de_mi_especie(int tid);
int verificacion_tcb(int socket);
int verificacion_archivo_tareas(int socket);
void cambiar_estado(t_tripulante* un_tripulante, char estado, int socket_ram);
void esperar_entrada_salida(t_tripulante* un_tripulante, int st_ram, int st_mongo);
int es_mi_turno(t_tripulante* un_tripulante);
void ciclo_de_vida_rr(t_tripulante* un_tripulante, int st_ram, int st_mongo, char* estado_guardado);
void ciclo_de_vida_fifo(t_tripulante* un_tripulante, int st_ram, int st_mongo, char* estado_guardado);

#endif /* DISCORDIADOR_UTILS_H_ */
