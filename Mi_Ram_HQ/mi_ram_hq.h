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
#include<pthread.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<comms/paquetes.h>
#include<comms/estructuras.h>
#include<comms/socketes.h>
#include"utils.h"

void atender_clientes(int socket_hijo);

#endif /* MI_RAM_HQ_H_ */
