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

typedef struct pagina {
    int base;
    bool libre;
    //uint64_t ultimo_uso; // para LRU
} pagina;
t_list* paginas;


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
pagina* crear_pagina(int base, bool libre);

indice_tabla* crear_indice(int pid, void* tabla);
void* buscar_tabla(int pid);


#endif /* MI_RAM_HQ_H_ */
