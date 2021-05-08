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

int reconocerComando(char* str);
int compararString(char* str, char* str2);
void helpComandos();

#endif /* DISCORDIADOR_UTILS_H_ */
