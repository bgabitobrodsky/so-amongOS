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
#include <readline/readline.h>
#include <commons/string.h>

enum comandos{
	INICIAR_PLANIFICACION,
	PAUSAR_PLANIFICACION,
	INICIAR_PATOTA,
	LISTAR_TRIPULANTES,
	EXPULSAR_TRIPULANTE,
	OBTENER_BITACORA
};

#define PARAMETROS_INICIAR_PLANIFICACION 2;

#endif /* DISCORDIADOR_H_ */
