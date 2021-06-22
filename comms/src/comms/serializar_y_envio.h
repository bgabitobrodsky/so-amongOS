#ifndef SERIALIZAR_Y_ENVIO_H_
#define SERIALIZAR_Y_ENVIO_H_

#include <CUnit/Basic.h>
#include "estructuras.h"
#include "paquetes.h"

int CUmain();
void test_serializar_tcb();
void test_serializar_tarea();
void test_serializar_vacio();

void test_desserializar_tcb();
void test_desserializar_tarea();
void test_desserializar_vacio();



//// GENERICOS////////////
t_PCB pcb_generico();
t_TCB tcb_generico();
t_tarea tarea_generica();

//////////////////////// UTILES ///////////////777777//////////////////
t_PCB crear_pcb_(int pid, char* direccion_tareas);
t_TCB crear_tcb_(int tid, int coord_x, int coord_y, char estado_tripulante, t_PCB* puntero_a_pcb);
t_tarea crear_tarea_(int coord_x, int coord_y, int duracion, char* nombre, int parametro);
void son_pcb_iguales(t_PCB pcb_1, t_PCB pcb_2); //TODO tipo de retorno
void son_tcb_iguales(t_TCB tcb_1, t_TCB tcb_2); //TODO tipo de retorno
void son_tareas_iguales(t_tarea tarea_1, t_tarea tarea_2); //TODO tipo de retorno

#endif /* SERIALIZAR_Y_ENVIO_H_ */
