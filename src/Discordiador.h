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

/*
enum comandos{
	INICIAR_PLANIFICACION,
	PAUSAR_PLANIFICACION,
	INICIAR_PATOTA,
	LISTAR_TRIPULANTES,
	EXPULSAR_TRIPULANTE,
	OBTENER_BITACORA
};


#define PARAMETROS_INICIAR_PLANIFICACION 2;
*/
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
} config_t;

void leer_config();
void leer_consola();
void iniciar_patota();

#endif /* DISCORDIADOR_H_ */
