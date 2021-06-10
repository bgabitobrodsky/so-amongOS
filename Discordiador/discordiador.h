/*
 * Discordiador.h
 *
 *  Created on: 25 abr. 2021
 *      Author: utnso
 */

#ifndef DISCORDIADOR_H_
#define DISCORDIADOR_H_

#include "utils.h"
//#include <stdio.h>
//#include <stdlib.h>
//#include <commons/log.h>
//#include <commons/config.h>
//#include <commons/string.h>
//#include <pthread.h>
//#include <readline/readline.h>
//#include <time.h>
//#include <stddef.h>
//#include <commons/collections/queue.h>
//#include <commons/collections/list.h>

void leer_consola();
void iniciar_patota(char* leido);
void iniciar_planificacion();
void listar_tripulantes();
void pausar_planificacion();
void obtener_bitacora(char* leido);
int pedir_tarea(int id_tripulante);
void realizar_tarea(t_tarea tarea);
char* fecha_y_hora();
void escuchar_discordiador(void* args);

//HILOS
void expulsar_tripulante(char* leido);
void iniciar_hilo_tripulante(void* funcion);
t_TCB* crear_tcb(t_PCB* pcb, int tid, char* posicion);
//t_tripulante* crear_tripulante(t_TCB* un_tcb);
t_TCB* iniciar_tcb(void* funcion, t_PCB* pcb, int tid, char* posicion);
void enlistar_tripulante();

//PROCESOS
//t_patota* crear_patota(t_PCB* un_pcb);
t_PCB* crear_pcb(char* path);
int nuevo_pid();
t_list* lista_tripulantes_patota(t_PCB* pcb);

//FUNCIONES AUXILIARES

int esta_en_lista(t_list* lista, int elemento);
int sonIguales(int elemento1, int elemento2);
void iniciar_listas();
void iniciar_colas();

#endif /* DISCORDIADOR_H_ */
