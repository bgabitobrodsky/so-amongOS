#ifndef I_MONGO_ARCHIVOS_H_
#define I_MONGO_ARCHIVOS_H_

#include "mongo_blocks.h"

/* ESTRUCTURAS PROPIAS */

void inicializar_archivos();
void inicializar_archivos_preexistentes();
void asignar_nuevo_bloque(char* path);
int asignar_primer_bloque_libre(t_list* lista_bloques, int cantidad_deseada, char tipo,  char* path);
int quitar_ultimo_bloque_libre(t_list* lista_bloques, uint32_t cant_bloques, int cantidad_deseada, char tipo);
void alterar(int codigo_archivo, int cantidad);
void agregar(int codigo_archivo, int cantidad);
void quitar(int codigo_archivo, int cantidad);
char* conseguir_tipo(char tipo);
char conseguir_char(int codigo_archivo);
FILE* conseguir_archivo_char(char tipo);
FILE* conseguir_archivo_recurso(int codigo);
char* conseguir_path_recurso_codigo(int codigo_archivo);
char* conseguir_path_recurso_archivo(FILE* archivo);
void crear_md5(char *str, unsigned char digest[16]);
int max(int a, int b);
int es_recurso(char* path);
void asignar_bloque_recurso(char* archivo, int bit_libre);
void asignar_bloque_tripulante(char* archivo, int bit_libre);
FILE* conseguir_archivo(char* path);

// devuelven la metadata del archivo
t_list* obtener_lista_bloques(char* path); // TESTEADA
uint32_t tamanio_archivo(char* path);
uint32_t cantidad_bloques_recurso(char* path);
char caracter_llenado_archivo(char* path);
char* md5_archivo(char* path);
uint32_t cantidad_bloques_tripulante(char* path);
void escribir_archivo_tripulante(char* path, uint32_t tamanio, t_list* lista_bloques);
void escribir_tamanio(char* path, uint32_t tamanio);
uint32_t bloques_contar(char caracter);
void imprimir_bitmap(); // TESTEADO

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
