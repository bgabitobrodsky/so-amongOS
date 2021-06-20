#ifndef I_MONGO_ARCHIVOS_H_
#define I_MONGO_ARCHIVOS_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/bitarray.h>
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
#include <signal.h>
#include <sys/mman.h>

#define TAMANIO_BLOQUE 64
#define CANTIDAD_BLOQUES 64 
#define PUNTO_MONTAJE config_get_string_value(config_mongo, "PUNTO_MONTAJE");

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

    t_TCB* tripulante;
    FILE* bitacora_asociada;
    // ¿Porqué no le pusiste a las bitácoras su tamanio y bloques?
    int tamanio;
    int* bloques;

} t_bitacora;

typedef struct {

    FILE* oxigeno;
    FILE* comida;
    FILE* basura;
    FILE* superbloque;
    FILE* blocks;
    char* path_blocks;
    void* mapa_blocks;

} t_archivos;

/* SEMAFOROS PROPIOS */

pthread_mutex_t mutex_oxigeno;
pthread_mutex_t mutex_comida;
pthread_mutex_t mutex_basura;
pthread_mutex_t mutex_blocks;

void inicializar_archivos(char* path_files);
void inicializar_archivos_preexistentes(char* path_files);
void asignar_nuevo_bloque(FILE* archivo);
void asignar_bloque(FILE* archivo, int bit_libre); //TODO
int asignar_primer_bloque_libre(uint32_t* lista_bloques, uint32_t cant_bloques, int cantidad_deseada, char tipo);
int quitar_ultimo_bloque_libre(uint32_t* lista_bloques, uint32_t cant_bloques, int cantidad_deseada, char tipo);
void actualizar_MD5(FILE* archivo);
void alterar(int codigo_archivo, int cantidad);
void agregar(FILE* archivo, int cantidad);
void quitar(FILE* archivo, char* path, int cantidad, char tipo);
char* conseguir_tipo(char tipo);
char conseguir_char(int codigo_archivo);
FILE* conseguir_archivo_char(char tipo);
FILE* conseguir_archivo(int codigo);
char* conseguir_path(int codigo_archivo);
char* conseguir_nombre(FILE* archivo);
int max(int a, int b);
char char_random();

// devuelven la metadata del archivo
uint32_t tamanio_archivo(FILE* archivo);
uint32_t cantidad_bloques_archvio(FILE* archivo);
uint32_t* lista_bloques_archvio(FILE* archivo);
char caracter_llenado_archivo(FILE* archivo);
char* md5_archivo(FILE* archivo);
void escribir_archivo(FILE* archivo, uint32_t tamanio, uint32_t cantidad_bloques, uint32_t* list_bloques, char caracter_llenado, char* md_5);



extern t_log* logger_mongo;
extern t_config* config_mongo;
extern t_archivos archivos;
extern t_list* bitacoras;
extern int* posiciciones_bitacora;

#endif
