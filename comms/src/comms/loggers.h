#ifndef LOGGERS_H_
#define LOGGERS_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include "semaforos.h"

typedef enum{
    INFO,
    WARNING
} logger_cod;

t_log* crear_logger(char* path_file, char* name, int* semaforo);

void loggear(t_log* logger,  int* semaforo, logger_cod tipo, char* string); // TODO, implementar el format de strings como en el logger de las commons

void liberar_logger(t_log* logger, int* semaforo);

#endif
