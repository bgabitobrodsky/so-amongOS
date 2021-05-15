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
	PAQUETE
}op_code;

t_log* logger;
t_config* config, config_discordiador;

int iniciar_servidor(void);
int esperar_discordiador(int socket_servidor);
int leer_operacion(int socket_cliente);
void* recibir_buffer(int* size, int socket_cliente);
void recibir_mensaje(int socket_cliente);

#endif /* MI_RAM_HQ_UTILS_H_ */
