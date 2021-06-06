/*
 * i_mongo_store.h
 *
 *  Created on: 9 may. 2021
 *      Author: utnso
 */

#ifndef I_MONGO_STORE_H_
#define I_MONGO_STORE_H_

#include "utils.h"
//#include <pthread.h>
//#include <readline/readline.h>
//#include<comms/paquetes.h>
//#include<comms/estructuras.h>
//#include<unistd.h>
//#include<stdio.h>
//#include<stdlib.h>
//#include<commons/log.h>
//#include<commons/string.h>
//#include<commons/config.h>
//#include<comms/socketes.h>

typedef struct{

    char* punto_montaje;
    int puerto;
    int tiempo_sincronizacion;
    char** posiciones_sabotaje;
    
} config_mongo_t;

typedef struct {
    int socket_oyente;
} args_escuchar_mongo;

void escuchar_mongo(void* args);

#endif /* I_MONGO_STORE_H_ */
