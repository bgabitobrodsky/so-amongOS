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




void gestionar_tareas (t_archivo_tareas*);
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

typedef struct segmento {
    int base;
    int tam;
    bool libre;
} segmento;
t_list* segmentos;


typedef struct tabla_paginas {
    t_list* paginas;
} tabla_paginas;

typedef struct tabla_segmentos {
    segmento* segmento_pcb;
    segmento* segmento_tareas;
    t_list* segmentos_tcb;
} tabla_segmentos;

typedef struct indice_tabla {
    uint32_t pid;
    void* tabla;
} indice_tabla;
t_list* tablas;


segmento* asignar_segmento(int tam);
segmento* buscar_segmento_libre(int tam);
segmento* best_fit(int tam);
segmento* first_fit(int tam);
segmento* crear_segmento(int base,int tam,bool libre);
void liberar_segmento(int base);

tabla_segmentos* crear_tabla_segmentos(uint32_t pid);
tabla_paginas* crear_tabla_paginas(uint32_t pid);
indice_tabla* crear_indice(int pid, void* tabla);
pagina* crear_pagina();

void* buscar_tabla(int pid);

void iniciar_memoria();
void print_segmentos_info();
void print_tablas_info();

void test_segmentos();
void test_tabla_segmentos();
#endif /* MI_RAM_HQ_H_ */
