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
#include <fts.h>

#if defined(__APPLE__)
#  define COMMON_DIGEST_FOR_OPENSSL
#  include <CommonCrypto/CommonDigest.h>
#  define SHA1 CC_SHA1
#else
#  include <openssl/md5.h>
#endif

// #define TAMANIO_BLOQUE obtener_tamanio_bloque()
// #define CANTIDAD_BLOQUES obtener_cantidad_bloques()

#define TAMANIO_BLOQUE config_get_int_value(config_superbloque, "TAMANIO_BLOQUE")
#define CANTIDAD_BLOQUES config_get_int_value(config_superbloque, "CANTIDAD_BLOQUES")
#define PUNTO_MONTAJE config_get_string_value(config_mongo, "PUNTO_MONTAJE");
#define POSICIONES_SABOTAJE config_get_array_value(config_mongo, "POSICIONES_SABOTAJE")
#define TIEMPO_SINCRONIZACION config_get_int_value(config_mongo, "TIEMPO_SINCRONIZACION")
#define	IP_MONGO_STORE config_get_string_value(config_mongo, "IP")
#define PUERTO_MONGO_STORE config_get_string_value(config_mongo, "PUERTO")
#define LIMIT_CONNECTIONS 10

/* ESTRUCTURAS PROPIAS */

typedef struct {

    FILE* superbloque;
    FILE* blocks;
    char* mapa_blocks;
    char* supermapa;
} t_directorio;

typedef struct {

    FILE* oxigeno;
    FILE* comida;
    FILE* basura;

} t_recurso;

typedef struct {

    t_TCB* tripulante;
    FILE* bitacora_asociada;
    uint32_t tamanio;
    t_list* bloques;
    char* path;

} t_bitacora;

/* SEMAFOROS PROPIOS */

pthread_mutex_t mutex_oxigeno;
pthread_mutex_t mutex_comida;
pthread_mutex_t mutex_basura;
pthread_mutex_t mutex_blocks;
extern pthread_mutex_t sem_lista_bloques_ocupados;

extern t_log* logger_mongo;
extern t_config* config_mongo;
extern t_config* config_superbloque;
extern t_list* bitacoras;
extern t_list* lista_bloques_ocupados;
extern char* mapa;

void iniciar_superbloque(FILE* archivo); // testeado
void iniciar_superbloque_fd(int filedescriptor_superbloque);
void iniciar_blocks(int filedescriptor_blocks); // testeado
void inicializar_mapa(); // testeado
uint32_t obtener_tamanio_bloque_superbloque(); // testeado
uint32_t obtener_cantidad_bloques_superbloque(); // testeado
t_bitarray* obtener_bitmap(); // TESTEADO
char* crear_puntero_a_bitmap(); // TESTEADO
char* crear_puntero_a_bitmap_fd();
void reescribir_superbloque(uint32_t tamanio, uint32_t cantidad, t_bitarray* bitmap); // TESTEADO
void reescribir_superbloque_fd(uint32_t tamanio, uint32_t cantidad, t_bitarray* bitmap);
void actualizar_bitmap(t_list* bloques_ocupados); // TESTEADO
void reemplazar(t_list* lista, int index, void* elemento); // TESTEADO y patear a generales
void reescribir_bitmap(t_bitarray* bitmap);
void reescribir_bitmap_fd(t_bitarray* bitmap);
void sincronizar_map();

void set_bloq(char* path, t_list* lista); // TESTEADO
void set_tam(char* path, int tamanio); // TESTEADO
void set_md5(char* path, char* md5); // TESTEADO
void set_caracter_llenado(char* path, char caracter); // TESTEADO
void set_cant_bloques(char* path, int cant); // TESTEADO
void iniciar_archivo_recurso(char* path, int tamanio, int cant_bloques, t_list* lista_bloques); // TESTEADO
void cargar_bitmap();
int max(int a, int b);
int min(int a, int b);

// Locks
void crearDiccionarioLocks();
void lockearLectura(char*);
void lockearEscritura(char*);
void unlockear(char*);
void  verificarExistencia(char*);
void liberar_lista(t_list* lista);
void matar_lista(t_list* lista);

#endif
