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

/*
    NECESARIOS
*/
t_log* logger;
t_config* config;

/*
    MAPA
*/
NIVEL* nivel;
int cols, rows;
int err;
char ultima_clave_mapa;
t_dictionary* mapa_indices;
pthread_mutex_t m_mapa;

/*
    MANEJO DE MEMORIA
*/
void* memoria_principal;
t_dictionary* tablas;

/*
    SEMAFOROS GENERALES
*/
pthread_mutex_t m_tablas;

/*
    SEMÁFOROS SEGMENTACIÓN
*/
pthread_mutex_t asignacion_segmento;
pthread_mutex_t lista_segmentos;

/*
    SEMÁFOROS PAGINACIÓN
*/
pthread_mutex_t m_swap;
pthread_mutex_t asignacion_marco;

/*
    MEMORIA VIRTUAL
*/
FILE* disco;
int marcos_disco_size;
bool* bitmap_disco;

/*
    FUNCIONES PRINCIPALES
*/
void proceso_handler(void* args);
void atender_clientes(void*);

void iniciar_memoria();
void* buscar_tabla(int pid);
int gestionar_tareas (t_archivo_tareas*);
int gestionar_tcb(t_TCB*);
t_TCB* buscar_tcb_por_tid(int);
t_list* buscar_tcbs_por_pid(int);
t_tarea* buscar_siguiente_tarea(int tid);
int eliminar_tcb(int tid);
int actualizar_tcb(t_TCB*);
void dump(int n);
void signal_compactacion(int n);

/*
    FUNCIONES PARA EL MAPA
*/
void iniciar_mapa();
void mapa_iniciar_tcb(t_TCB* tcb);
char* get_clave_mapa_por_tid(int tid);
void matar_tcb_en_mapa(int tid);
void actualizar_tcb_en_mapa(t_TCB* tcb);
void matar_mapa();
void bloquear_mapa();
void desbloquear_mapa();

/*
    FUNCIONES AUXILIARES
*/
int tamanio_tarea(t_tarea* tarea);
void bloquear_tabla(void* una_tabla);
void desbloquear_tabla(void* una_tabla);
void bloquear_lista_tablas();
void desbloquear_lista_tablas();

#endif /* MI_RAM_HQ_H_ */
