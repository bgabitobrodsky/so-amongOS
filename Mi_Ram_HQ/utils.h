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
#include<comms/estructuras.h>
#include<comms/paquetes.h>
#include<comms/socketes.h>
#include <pthread.h>
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <math.h>

extern t_log* logger;
extern t_config* config;

size_t tamanio_tarea(t_tarea* tarea);

#endif /* MI_RAM_HQ_UTILS_H_ */
