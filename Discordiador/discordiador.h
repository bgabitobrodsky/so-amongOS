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
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include"utils.h"
#include"../Modulos/socketes.h"

typedef struct{
    char* ip_mi_ram_hq;
    int puerto_mi_ram_hq;
    char* ip_i_mongo_store;
    int puerto_i_mongo_store;
    int grado_multitarea;
    char* algoritmo;
    int quantum;
    int duracion_sabotaje;
    int retardo_ciclo_cpu;
} config_discordiador_t;

void leer_config();
void leer_consola();
void iniciar_patota(char* leido);
void listar_tripulantes();
void iniciar_planificacion();
void pausar_planificacion();
void obtener_bitacora(char* leido);
void expulsar_tripulante(char* leido);
void help_comandos();

#endif /* DISCORDIADOR_H_ */
