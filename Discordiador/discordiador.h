/*
 * Discordiador.h
 *
 *  Created on: 25 abr. 2021
 *      Author: utnso
 */

#ifndef DISCORDIADOR_H_
#define DISCORDIADOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <readline/readline.h>
#include <time.h>
//#include <comms/loggers.h>
#include <stddef.h>
#include "utils.h"


void leer_consola();
void iniciar_patota(char* leido);
void iniciar_planificacion();
void listar_tripulantes();
void pausar_planificacion();
void obtener_bitacora(char* leido);
void expulsar_tripulante(char* leido);
void tripulante();
int pedir_tarea(int id_tripulante);
void realizar_tarea(t_tarea tarea);
void instanciar_tripulante(char* str_posicion);
char* fecha_y_hora();
void iniciar_hilo_tripulante(void* funcion);
t_tripulante* crear_tripulante(char* posicion);
int nuevo_pid();
int pids_contiene(int valor);

#endif /* DISCORDIADOR_H_ */
