/*
 * i_mongo_store.h
 *
 *  Created on: 9 may. 2021
 *      Author: utnso
 */

#ifndef I_MONGO_STORE_H_
#define I_MONGO_STORE_H_

#include "mongo_tripulantes.h"

typedef struct {
	int socket;
	void (*atender)(char*);
} hilo_tripulante;

typedef struct {
	int socket;
	void (*atender)(char*);
} hilo_discordiador;

extern t_log* logger_mongo;
extern t_directorio directorio;
extern t_recurso recurso;
extern t_config* config_mongo;
extern t_list* bitacoras;

char* path_directorio;
char* path_files;
char* path_bitacoras;
char* path_oxigeno;
char* path_comida;
char* path_basura;
char* path_superbloque;
char* path_blocks;


void escuchar_mongo(void* args);
//void escuchar_mongo(int args);
void sabotaje(int socket_discordiador);
void iniciar_file_system();
void cerrar_archivos();
void cerrar_mutexs();

#endif /* I_MONGO_STORE_H_ */
