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
//#include <comms/estructuras.h>
//#include <comms/socketes.h>

void atender_clientes(int socket_hijo);
void escuchar_miram(void* args);
t_PCB* crear_pcb(char* path);
t_TCB crear_tcb(t_PCB* pcb, int tid, char* posicion);

/**
 * MANEJO DE MEMORIA
**/

typedef struct pagina {
    int base;
    bool libre;
    //uint64_t ultimo_uso; // para LRU
} pagina;
t_list* paginas;

typedef struct segmento {
    int base;
    int tam;
    bool libre;
} segmento;
t_list* segmentos;


typedef struct tabla_paginas {
    uint32_t pid;
    t_list* paginas;
} tabla_paginas;

typedef struct tabla_segmentos {
    uint32_t pid;
    t_list* segmentos;
} tabla_segmentos;

void* memoria_principal;

segmento* crear_segmento(int tam);
tabla_segmentos* crear_tabla_segmentos(uint32_t pid);
pagina* crear_pagina();
tabla_paginas* crear_tabla_paginas(uint32_t pid);


#endif /* MI_RAM_HQ_H_ */
