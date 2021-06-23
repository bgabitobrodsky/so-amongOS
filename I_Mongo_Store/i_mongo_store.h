/*
 * i_mongo_store.h
 *
 *  Created on: 9 may. 2021
 *      Author: utnso
 */

#ifndef I_MONGO_STORE_H_
#define I_MONGO_STORE_H_

#include "mongo_tripulantes.h"

extern t_log* logger_mongo;
extern t_config* config_mongo;
extern t_archivos archivos;
extern t_list* bitacoras;

void escuchar_mongo(void* args);
void sabotaje(int socket_discordiador);
void iniciar_file_system();
void cerrar_archivos();
void cerrar_mutexs();

#endif /* I_MONGO_STORE_H_ */
