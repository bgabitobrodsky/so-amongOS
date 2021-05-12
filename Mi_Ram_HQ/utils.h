/*
 * utils.h
 *
 *  Created on: 8 may. 2021
 *      Author: utnso
 */

#ifndef MI_RAM_HQ_UTILS_H_
#define MI_RAM_HQ_UTILS_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>

typedef enum{
	MENSAJE
}op_code;

t_log* logger;

int iniciar_servidor(void);
int esperar_discordiador(int socket_servidor);
int leer_operacion(int socket_cliente);

#endif /* MI_RAM_HQ_UTILS_H_ */
