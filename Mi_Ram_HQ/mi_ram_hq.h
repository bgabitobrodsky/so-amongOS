/*
 * i_mongo_store.h
 *
 *  Created on: 9 may. 2021
 *      Author: utnso
 */

#ifndef MI_RAM_HQ_H_
#define MI_RAM_HQ_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/config.h>
#include<commons/collections/list.h>
#include<string.h>
#include<comms/estructuras.h>
#include<comms/paquetes.h>
#include<comms/socketes.h>
#include <pthread.h>
#include <commons/string.h>

extern t_log* logger_miramhq;
extern t_config* config_miramhq;

typedef struct {
    int socket_oyente;
} args_escuchar_miram;

void atender_clientes(int socket_hijo);
void escuchar_miram(void* args);

#endif /* MI_RAM_HQ_H_ */
