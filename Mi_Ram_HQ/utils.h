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
#include<commons/config.h>
#include<commons/collections/list.h>
#include<string.h>

typedef enum{
	MENSAJE,
	PEDIR_TAREA
}op_code;

extern t_log* logger;
extern t_config* config;
extern t_config* config_discordiador;

int iniciar_servidor(void);
int esperar_discordiador(int socket_servidor);
int leer_operacion(int socket_cliente);
void* recibir_buffer(int* size, int socket_cliente);
void recibir_mensaje(int socket_cliente);

#endif /* MI_RAM_HQ_UTILS_H_ */
