#ifndef I_MONGO_ARCHIVOS_H_
#define I_MONGO_ARCHIVOS_H_

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

void inicializar_archivos(char* path_files);
void alterar(int codigo_archivo, int cantidad);
void agregar(FILE* archivo, int cantidad, char tipo);
FILE* conseguir_archivo(int codigo);
char conseguir_char(int codigo);
void quitar(FILE* archivo, int cantidad, char tipo);

#endif