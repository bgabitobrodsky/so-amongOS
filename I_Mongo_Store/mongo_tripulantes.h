#ifndef I_MONGO_TRIPULANTES_H_
#define I_MONGO_TRIPULANTES_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <string.h>
#include <comms/estructuras.h>
#include <comms/paquetes.h>
#include <comms/socketes.h>
#include <pthread.h>
#include <readline/readline.h>
#include <sys/types.h>
#include <sys/stat.h>

extern t_log* logger_mongo;
extern t_config* config_mongo;
extern t_archivos archivos;

void manejo_tripulante(int socket_tripulante);
void crear_estructuras_tripulante(t_tripulante* tripulante, int socket_tripulante);
void modificar_bitacora(int codigo_operacion, t_tripulante* tripulante);

#endif