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

//FUnciones de testeo
void funcion_hilo();
void iniciar_hilo();

// Funciones PRINCIPALES
void leer_consola();
void iniciar_patota(char* leido);
void listar_tripulantes();
void expulsar_tripulante(char* leido);
void iniciar_planificacion();
void pausar_planificacion();
void obtener_bitacora(char* leido);

// Funciones de conexiones y sockets
void enviar_tcb_a_ram(t_TCB un_tcb, int socket);


// HILOS
void iniciar_hilo_tripulante(void* funcion);
t_TCB* crear_puntero_tcb(t_PCB* pcb, int tid, char* posicion);
t_TCB crear_tcb(t_PCB* pcb, int tid, char* posicion);

t_TCB* iniciar_tcb(void* funcion, t_PCB* pcb, int tid, char* posicion);
void enlistar_algun_tripulante();

// PROCESOS
//t_patota* crear_patota(t_PCB* un_pcb);
t_PCB* crear_pcb(char* path);
int nuevo_pid();
t_list* lista_tripulantes_patota(uint32_t pid);

// FUNCIONES AUXILIARES
int esta_tcb_en_lista(t_list* lista, t_TCB* elemento);
void* eliminar_tcb_de_lista(t_list* lista, t_TCB* elemento);


#endif /* DISCORDIADOR_H_ */
