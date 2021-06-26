#ifndef MI_RAM_HQ_H_
#define MI_RAM_HQ_H_

#include "utils.h"

//#include <stdio.h>
//#include <stdlib.h>
//#include <pthread.h>
//#include <commons/string.h>
//#include <commons/log.h>
//#include <commons/config.h>
//#include <comms/paquetes.h>
#include <comms/estructuras.h>
//#include <comms/socketes.h>




void atender_clientes(void*);
void proceso_handler(void* args);
t_PCB* crear_pcb(char* path);
t_TCB crear_tcb(t_PCB* pcb, int tid, char* posicion);

/**
 * MANEJO DE MEMORIA
**/

void* memoria_principal;

t_dictionary* tablas;
t_log* logger;
t_config* config;

void iniciar_memoria();
int gestionar_tareas (t_archivo_tareas*);
int gestionar_tcb(t_TCB*);


void* buscar_tabla(int pid);
t_TCB* buscar_tcb(int tid);
t_tarea* buscar_siguiente_tarea(int tid);
t_list* buscar_tcbs_por_pid(int);
t_TCB* buscar_tcb_por_tid(int);
int actualizar_tcb(t_TCB*);
int eliminar_tcb(int tid); // devuelve 1 si todo ok, 0 si falló algo

#endif /* MI_RAM_HQ_H_ */
