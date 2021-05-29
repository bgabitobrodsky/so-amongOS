#include "loggers.h"


t_log* crear_logger(char* path_file, char* name, int* semaforo){
    if( (semaforo=semget(IPC_PRIVATE,1,IPC_CREAT | 0700)) < 0) {
        printf("Error al crear semaforo para loggers\n");
    }
    initSem(semaforo, 0, 1);
    return log_create(path_file, name, true, LOG_LEVEL_INFO);
}

void loggear(t_log* logger, int* semaforo, logger_cod tipo, char* string){

    doWait(semaforo,0);

    switch(tipo){
        case INFO:
            log_info(logger,string);
            break;
        case WARNING:
            log_warning(logger,string);
            break;
        default:
            log_info(logger,string);
            break;
    }
    
    doSignal(semaforo,0);
}

void liberar_logger(t_log* logger, int* semaforo){
    log_destroy(logger);
    freeSem(semaforo);
}


