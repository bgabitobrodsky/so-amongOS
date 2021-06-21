/*
 * Discordiador.h
 *
 *  Created on: 25 abr. 2021
 *      Author: utnso
 */

#ifndef DISCORDIADOR_H_
#define DISCORDIADOR_H_

#include "utils.h"

//FUnciones de testeo
void funcion_hilo();
void iniciar_hilo();

// Funciones PRINCIPALES
void iniciar_patota(char* leido);
void listar_tripulantes();
void expulsar_tripulante(char* leido);
void iniciar_planificacion();
void pausar_planificacion();
void obtener_bitacora(char* leido);

// HILOS
t_TCB* crear_puntero_tcb(t_PCB* pcb, int tid, char* posicion);
t_TCB crear_tcb(t_PCB* pcb, int tid, char* posicion);
void enlistar_algun_tripulante();
void tripulante(t_tripulante* un_tripulante);
void iniciar_tripulante(t_tripulante* un_tripulante);
void enlistarse(t_tripulante* un_tripulante);
void realizar_tarea(t_tripulante* un_tripulante);
void llegar_a_destino(t_tripulante* un_tripulante);
void no_me_despierten_estoy_trabajando(t_tripulante* un_tripulante);
int identificar_tarea(char* nombre_recibido);

// PROCESOS
t_patota* crear_patota(uint32_t un_pid);
t_PCB* crear_pcb(char* path);
int nuevo_pid();
t_list* lista_tripulantes_patota(uint32_t pid);
t_tripulante* crear_tripulante(int tid, int x, int y, char estado);

// FUNCIONES AUXILIARES
void leer_consola();

// TESTS
void test_serializar_tcb();
void test_iniciar_patota();
void test_listar_tripulantes();
void test_nuevo_pid();
void test_enlistar_algun_tripulante();
void test_serializar_tarea();

#endif /* DISCORDIADOR_H_ */
