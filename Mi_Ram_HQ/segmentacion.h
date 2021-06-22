#ifndef SEGMENTACION_H_
#define SEGMENTACION_H_

#include "mi_ram_hq.h"

extern t_list* indices;

typedef struct segmento {
    int base;
    int tam;
    bool libre;
} segmento;
t_list* segmentos;

typedef struct tabla_segmentos {
    segmento* segmento_pcb;
    segmento* segmento_tareas;
    t_list* segmentos_tcb;
} tabla_segmentos;

tabla_segmentos* crear_tabla_segmentos(uint32_t pid);
segmento* asignar_segmento(int tam);
segmento* buscar_segmento_libre(int tam);
segmento* best_fit(int tam);
segmento* first_fit(int tam);
segmento* crear_segmento(int base,int tam,bool libre);
void liberar_segmento(int base);
void unificar_segmentos_libres();

void print_segmentos_info();
void print_tablas_info();
void test_segmentos();
void test_tabla_segmentos();
void test_gestionar_tarea(int pid);
void test_gestionar_tcb();

#endif /* SEGMENTACION_H_ */
