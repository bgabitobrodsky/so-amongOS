#ifndef MI_RAM_HQ_H_
#define MI_RAM_HQ_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <time.h>

#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/temporal.h>
#include <commons/config.h>
#include <commons/txt.h>

#include <comms/estructuras.h>
#include <comms/paquetes.h>
#include <comms/socketes.h>
#include <comms/generales.h>

#include <nivel-gui/nivel-gui.h>
#include <nivel-gui/tad_nivel.h>
#include <curses.h>

#include "segmentacion.h"
#include "paginacion.h"


/**
 * FUNCIONES PRINCIPALES
**/

void atender_clientes(void*);
void proceso_handler(void* args);


/**
 * MAPA
**/
/*
NIVEL* nivel;
int cols = 9, rows = 9;
int err;
char last_key = 0;
*/
t_dictionary* mapa_indices;

void iniciar_mapa();
char mapa_iniciar_tcb(t_TCB* tcb);
void matar_mapa();

/**
 * MANEJO DE MEMORIA
**/


void* memoria_principal;

t_dictionary* tablas;
t_log* logger;
t_config* config;

pthread_mutex_t m_compactacion;
pthread_mutex_t m_swap;
pthread_mutex_t asignacion_marco;
//memo virtual
FILE* disco;
int marcos_disco_size;
bool* bitmap_disco;

void iniciar_memoria();
void iniciar_mapa();
int gestionar_tareas (t_archivo_tareas*);
int gestionar_tcb(t_TCB*);

void* buscar_tabla(int pid);
t_tarea* buscar_siguiente_tarea(int tid);
t_list* buscar_tcbs_por_pid(int);
t_TCB* buscar_tcb_por_tid(int);
int actualizar_tcb(t_TCB*);
int eliminar_tcb(int tid); // devuelve 1 si todo ok, 0 si falló algo
int tamanio_tarea(t_tarea* tarea);

#endif /* MI_RAM_HQ_H_ */
