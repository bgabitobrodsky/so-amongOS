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

typedef struct marco {
    int base;
    bool libre;
    //uint64_t ultimo_uso; // para LRU
} marco;
t_list* marcos;


typedef struct pagina {
    marco* puntero_marco;
    int tamaño_ocupado;
} pagina;



typedef struct tabla_paginas {
    t_list* paginas;
} tabla_paginas;

typedef struct indice_tabla {
    uint32_t pid;
    void* tabla;
} indice_tabla;


t_list* indices;

void iniciar_memoria();
void gestionar_tareas (t_archivo_tareas*);
void gestionar_tcb(t_TCB*);

tabla_paginas* crear_tabla_paginas(uint32_t pid);
marco* crear_marco(int base, bool libre);
void liberar_marco(int base);
marco* buscar_marco_libre();
marco* asignar_marco();

indice_tabla* crear_indice(int pid, void* tabla);
void* buscar_tabla(int pid);
t_TCB* buscar_tcb(int tid);


#endif /* MI_RAM_HQ_H_ */
