/*
 * utils.h
 *
 *  Created on: 8 may. 2021
 *      Author: utnso
 */

#ifndef DISCORDIADOR_UTILS_H_
#define DISCORDIADOR_UTILS_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<commons/string.h>

enum{
	NO_CONOCIDO,
	LISTAR_TRIPULANTES,
	INICIAR_PLANIFICACION,
	PAUSAR_PLANIFICACION,
	OBTENER_BITACORA,
	EXPULSAR_TRIPULANTE,
	INICIAR_PATOTA,
	EXIT,
	HELP
}comando_cod;

int reconocer_comando(char* str);
int comparar_strings(char* str, char* str2);
void help_comandos();

#endif /* DISCORDIADOR_UTILS_H_ */
