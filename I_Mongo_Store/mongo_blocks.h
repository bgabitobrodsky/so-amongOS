#ifndef I_MONGO_BLOCKS_H_
#define I_MONGO_BLOCKS_H_

#include "mongo_archivos.h"

extern t_log* logger_mongo;
extern t_config* config_mongo;
extern t_archivos archivos;
extern t_list* bitacoras;

void iniciar_superbloque(FILE* archivo);
void iniciar_blocks(FILE* archivo, int filedescriptor);

#endif