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
#include <commons/string.h>
#include <string.h>
#include <comms/estructuras.h>
#include <comms/paquetes.h>
#include <comms/socketes.h>
#include <pthread.h>
#include <readline/readline.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

extern t_log* logger_mongo;
extern t_config* config_mongo;

/* ESTRUCTURAS PROPIAS */

typedef struct{

    char* punto_montaje;
    int puerto;
    int tiempo_sincronizacion;
    char** posiciones_sabotaje;

} config_mongo_t;

typedef struct {
    int socket_oyente;
} args_escuchar_mongo;

typedef struct {

    FILE* oxigeno;
    FILE* comida;
    FILE* basura;
    FILE* superbloque;
    FILE* blocks;

} t_archivos;

/* SEMAFOROS PROPIOS */

pthread_mutex_t mutex_oxigeno;
pthread_mutex_t mutex_comida;
pthread_mutex_t mutex_basura;

void inicializar_archivos(char* path_files);
void alterar(int codigo_archivo, int cantidad);
void agregar(FILE* archivo, int cantidad, char tipo);
void agregar_unlocked(FILE* archivo, int cantidad, char tipo);
void quitar(FILE* archivo, int cantidad, char tipo);
char* conseguir_tipo(char tipo);
FILE* conseguir_archivo(int codigo);
char conseguir_char(int codigo);
pthread_mutex_t* conseguir_semaforo(char tipo);
int max(int a, int b);

#endif
