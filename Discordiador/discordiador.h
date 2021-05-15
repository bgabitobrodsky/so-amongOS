/*
 * Discordiador.h
 *
 *  Created on: 25 abr. 2021
 *      Author: utnso
 */

#ifndef DISCORDIADOR_H_
#define DISCORDIADOR_H_

#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<commons/log.h>
#include<commons/config.h>
#include<commons/string.h>
#include<readline/readline.h>
#include"utils.h"
#include <time.h>

t_config* config;
t_log* logger;

void leer_consola();
void iniciar_patota(char* leido);
void listar_tripulantes();
void iniciar_planificacion();
void pausar_planificacion();
void obtener_bitacora(char* leido);
void expulsar_tripulante(char* leido);
char* fecha_y_hora()

#endif /* DISCORDIADOR_H_ */
