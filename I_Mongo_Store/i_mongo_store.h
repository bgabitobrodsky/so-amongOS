/*
 * i_mongo_store.h
 *
 *  Created on: 9 may. 2021
 *      Author: utnso
 */

#ifndef I_MONGO_STORE_H_
#define I_MONGO_STORE_H_

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
void sabotaje(int socket_discordiador);
void manejo_tripulante(int socket_tripulante);
void iniciar_file_system();
int file_system_existente(char* punto_montaje, stat dir);
void inicializar_archivos(char* path_files);
void alterar(int codigo_archivo, int cantidad);
void agregar(FILE* archivo, int cantidad, char tipo);
void quitar(FILE* archivo, int cantidad, char tipo);
void cerrar_archivos();

#endif /* I_MONGO_STORE_H_ */
