#ifndef SEGMENTACION_H_
#define SEGMENTACION_H_

#include "mi_ram_hq.h"

/*
    ESTRUCTURAS
*/
typedef enum tipo_segmento{
    S_PCB,
    S_TAREAS,
    S_TCB
}tipo_segmento;

typedef struct segmento {
    int base;
    int tam;
    bool libre;
    tipo_segmento tipo;
    pthread_mutex_t mutex;
} segmento;
t_list* segmentos;

typedef struct tabla_segmentos {
    segmento* segmento_pcb;
    segmento* segmento_tareas;
    t_list* segmentos_tcb;
    pthread_mutex_t mutex;
} tabla_segmentos;

typedef struct segmento_dump_wrapper{
    segmento* segmento;
    int pid;
    int num;
} segmento_dump_wrapper;

/*
    VARIABLES GLOBALES UTILES PARA SEGMENTACION
*/
int intento_asignar_segmento;

/*
    FUNCIONES DE SEGMENTACION
*/
tabla_segmentos* crear_tabla_segmentos(int pid);
segmento* asignar_segmento(int tam);
segmento* buscar_segmento_libre(int tam);
segmento* best_fit(int tam);
segmento* first_fit(int tam);
segmento* crear_segmento(int base,int tam,bool libre);
void liberar_segmento(segmento*);
void unificar_segmentos_libres();
void matar_tabla_segmentos(int pid);
void dump_segmentacion();

/*
    FUNCIONES DE SEMAFORO
*/
void bloquear_lista_segmentos();
void desbloquear_lista_segmentos();
void liberar_segmento(segmento* segmento);
segmento* buscar_segmento_por_tid(int tid);
void bloquear_segmento(segmento* segmento);
void desbloquear_segmento(segmento* segmento);
void bloquear_segmento_por_tid(int tid);
void desbloquear_segmento_por_tid(int tid);

/*
    TESTS
*/
/*
void print_segmentos_info();
void print_tablas_segmentos_info();
void test_segmentos();
void test_tabla_segmentos();
void test_gestionar_tarea(int pid);
void test_gestionar_tcb();
void test_compactacion();
*/

#endif /* SEGMENTACION_H_ */
