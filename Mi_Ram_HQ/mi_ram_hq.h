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


typedef struct indice_tabla {
    uint32_t pid;
    void* tabla;
} indice_tabla;


t_list* indices;

void iniciar_memoria();
void gestionar_tareas (t_archivo_tareas*);
void gestionar_tcb(t_TCB*);


indice_tabla* crear_indice(int pid, void* tabla);
void* buscar_tabla(int pid);
t_TCB* buscar_tcb(int tid);


#endif /* MI_RAM_HQ_H_ */
