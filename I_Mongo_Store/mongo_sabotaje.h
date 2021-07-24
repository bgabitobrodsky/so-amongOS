#ifndef I_MONGO_SABOTAJE_H_
#define I_MONGO_SABOTAJE_H_

#include "mongo_tripulantes.h"

void enviar_posicion_sabotaje(int socket_discordiador);
void reparar();
int verificar_cant_bloques();
int verificar_bitmap();
int verificar_sizes(); // TESTEADO
int verificar_size_recurso(char* path);
int verificar_block_counts(); // TESTEADO
int verificar_block_counts_recurso(char* path);
int verificar_blocks();
int lista_blocks_saboteada(FILE* archivo);
void recorrer_recursos(t_list* lista_bloques_ocupados);
void recorrer_bitacoras(t_list* lista_bloques_ocupados);
void sortear(t_list* lista_bloques_ocupados);
int bloques_ocupados_difieren(t_list* lista_bloques_ocupados);
int md5_no_concuerda(); // TESTEADO
int md5_no_concuerda_recurso(char* path_recurso);
int bitmap_no_concuerda();
void restaurar_blocks(int codigo);

extern int existe_oxigeno;
extern int existe_comida;
extern int existe_basura;

#endif
