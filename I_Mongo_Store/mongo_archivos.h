#ifndef I_MONGO_ARCHIVOS_H_
#define I_MONGO_ARCHIVOS_H_

#include "mongo_blocks.h"

/* ESTRUCTURAS PROPIAS */

void inicializar_archivos();
void inicializar_archivos_preexistentes();
void asignar_nuevo_bloque(FILE* archivo);
int asignar_primer_bloque_libre(uint32_t* lista_bloques, uint32_t cant_bloques, int cantidad_deseada, char tipo);
int quitar_ultimo_bloque_libre(uint32_t* lista_bloques, uint32_t cant_bloques, int cantidad_deseada, char tipo);
void alterar(int codigo_archivo, int cantidad);
void agregar(int codigo_archivo, int cantidad);
void quitar(int codigo_archivo, int cantidad);
char* conseguir_tipo(char tipo);
char conseguir_char(int codigo_archivo);
FILE* conseguir_archivo_char(char tipo);
FILE* conseguir_archivo_recurso(int codigo);
char* conseguir_path_recurso_codigo(int codigo_archivo);
char* conseguir_path_recurso_archivo(FILE* archivo);
char* crear_md5();
char char_random();
int max(int a, int b);
int es_recurso(FILE* archivo);
void asignar_bloque_recurso(FILE* archivo, int bit_libre);
void asignar_bloque_tripulante(FILE* archivo, int bit_libre);

// devuelven la metadata del archivo
uint32_t tamanio_archivo(FILE* archivo);
uint32_t cantidad_bloques_recurso(FILE* archivo);
uint32_t* lista_bloques_recurso(FILE* archivo);
char caracter_llenado_archivo(FILE* archivo);
char* md5_archivo(FILE* archivo);
uint32_t cantidad_bloques_tripulante(FILE* archivo);
uint32_t* lista_bloques_tripulante(FILE* archivo);
void escribir_archivo_recurso(FILE* archivo, uint32_t tamanio, uint32_t cantidad_bloques, uint32_t* list_bloques);
void escribir_archivo_tripulante(FILE* archivo, uint32_t tamanio, uint32_t* lista_bloques);
void escribir_tamanio(FILE* archivo, uint32_t tamanio);
int bloques_contar(uint32_t* lista_bloques, char caracter);

extern t_log* logger_mongo;
extern t_config* config_mongo;
extern t_directorio directorio;
extern t_recurso recurso;
extern t_list* bitacoras;

extern char* path_directorio;
extern char* path_files;
extern char* path_bitacoras;
extern char* path_oxigeno;
extern char* path_comida;
extern char* path_basura;
extern char* path_superbloque;
extern char* path_blocks;

#endif
