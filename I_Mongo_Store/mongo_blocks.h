#ifndef I_MONGO_BLOCKS_H_
#define I_MONGO_BLOCKS_H_

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
#include <comms/generales.h>
#include <pthread.h>
#include <readline/readline.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <time.h>
#include <openssl/md5.h>

// #define TAMANIO_BLOQUE obtener_tamanio_bloque()
// #define CANTIDAD_BLOQUES obtener_cantidad_bloques()

#define TAMANIO_BLOQUE 64
#define CANTIDAD_BLOQUES 64
#define PUNTO_MONTAJE config_get_string_value(config_mongo, "PUNTO_MONTAJE");
#define POSICIONES_SABOTAJE config_get_array_value(config_mongo, "POSICIONES_SABOTAJE")
#define TIEMPO_SINCRONIZACION config_get_int_value(config_mongo, "TIEMPO_SINCRONIZACION")
#define	IP_MONGO_STORE config_get_string_value(config_mongo, "IP")
#define PUERTO_MONGO_STORE config_get_string_value(config_mongo, "PUERTO")
#define LIMIT_CONNECTIONS 10

/* ESTRUCTURAS PROPIAS */

typedef struct{

    char* punto_montaje;
    int puerto;
    int tiempo_sincronizacion;
    char** posiciones_sabotaje;

} config_mongo_t;

typedef struct {
    FILE* superbloque;
    FILE* blocks;
    char* mapa_blocks;
} t_directorio;

typedef struct {
    FILE* oxigeno;
    FILE* comida;
    FILE* basura;
} t_recurso;

typedef struct {

    t_TCB* tripulante;
    FILE* bitacora_asociada;
    int tamanio;
    int* bloques;

} t_bitacora;

/* SEMAFOROS PROPIOS */

pthread_mutex_t mutex_oxigeno;
pthread_mutex_t mutex_comida;
pthread_mutex_t mutex_basura;
pthread_mutex_t mutex_blocks;

extern t_log* logger_mongo;
extern t_config* config_mongo;
extern t_list* bitacoras;
extern char* mapa;

void iniciar_superbloque(FILE* archivo);
void iniciar_blocks(int filedescriptor_blocks);
void inicializar_mapa();
uint32_t obtener_tamanio_bloque();
uint32_t obtener_cantidad_bloques();
t_bitarray* obtener_bitmap();
char* crear_puntero_a_bitmap();
void reescribir_superbloque(uint32_t tamanio, uint32_t cantidad, t_bitarray* bitmap);
t_bitarray* actualizar_bitmap(int* lista_bloques_ocupados);
int contiene_generico(int* lista, int valor);
char devolver_byte(int numero_de_byte);

#endif
