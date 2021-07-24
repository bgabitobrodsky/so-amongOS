/*
 * i_mongo_store.h
 *
 *  Created on: 9 may. 2021
 *      Author: utnso
 */

#ifndef I_MONGO_STORE_H_
#define I_MONGO_STORE_H_

#include "mongo_sabotaje.h"

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
void sabotaje(int n);
void iniciar_file_system();
void sincronizar_blocks();
void cerrar_archivos();
void cerrar_mutexs();
void manejo_discordiador();
void matar_bitacora(void* una_bitacora);

#endif /* I_MONGO_STORE_H_ */
