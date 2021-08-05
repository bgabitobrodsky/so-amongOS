#ifndef I_MONGO_ARCHIVOS_H_
#define I_MONGO_ARCHIVOS_H_

#include "mongo_blocks.h"
#include <dirent.h>
#include <errno.h>
#include <semaphore.h>


/* ESTRUCTURAS PROPIAS */

void iniciar_paths();
void inicializar_archivos();
void inicializar_archivos_preexistentes();
void asignar_nuevo_bloque(char* path);
int quitar_ultimo_bloque_libre(int cantidad_deseada, char tipo);
int existe_archivo(int codigo_archivo);
int llenar_bloque_recurso(int cantidad_deseada, char tipo, char* path);
void alterar(int codigo_archivo, int cantidad);
void descartar_basura();
void agregar(int codigo_archivo, int cantidad);
void quitar(int codigo_archivo, int cantidad);
char* conseguir_tipo(char tipo);
char conseguir_char(int codigo_archivo);
char* tipo_a_path(char tipo);
FILE* conseguir_archivo_recurso(int codigo);
char* conseguir_path_recurso_codigo(int codigo_archivo);
char* conseguir_path_recurso_archivo(FILE* archivo);
int es_recurso(char* path);
void asignar_bloque_recurso(char* archivo, int* bit_libre);
void asignar_bloque_tripulante(char* archivo, int* bit_libre);
FILE* conseguir_archivo(char* path);
void limpiar_cuerpos();
void limpiar_metadata(char* path);
void liberar_bloques(char* path);
void liberar_bloque(char* path, uint32_t nro_bloque);
void blanquear_bloque(int bloque);
uint32_t obtener_cantidad_bloques(char* path);
char* concatenar_numeros(char* cadena);
void iniciar_archivo_recurso2(char* path, int tamanio);

// devuelven la metadata del archivo
t_list* get_lista_bloques(char* path); // TESTEADA
int tamanio_archivo(char* path);
uint32_t cantidad_bloques_recurso(char* path);
char caracter_llenado_archivo(char* path);
char* md5_archivo(char* path);
uint32_t cantidad_bloques_tripulante(char* path);
void escribir_archivo_tripulante(char* path, uint32_t tamanio, t_list* lista_bloques);
uint32_t bloques_contar(char caracter);

void agregar_tam(char* path, int tamanio);
void quitar_tam(char* path, int tamanio);
void set_tam(char* path, int tamanio);
void set_bloq(char* path, t_list* lista);
void set_cant_bloques(char* path, int cant);
void set_caracter_llenado(char* path, char caracter);
void set_md5(char* path, char* md5);
void lockear_recurso_lectura(char* path);
void unlockear_recurso(char* path);
void lockear_recurso_escritura(char* path);

extern t_log* logger_mongo;
extern t_config* config_mongo;
extern t_directorio directorio;
extern t_recurso recurso;
extern t_list* bitacoras;
extern sem_t sem_llenar_bloque_recurso;
extern sem_t sem_quitar_ultimo_bloque_libre;

extern char* path_directorio;
extern char* path_files;
extern char* path_bitacoras;
extern char* path_oxigeno;
extern char* path_comida;
extern char* path_basura;
extern char* path_superbloque;
extern char* path_blocks;

#endif
