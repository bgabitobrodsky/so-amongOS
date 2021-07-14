#ifndef I_MONGO_SABOTAJE_H_
#define I_MONGO_SABOTAJE_H_

#include "mongo_tripulantes.h"

void enviar_posicion_sabotaje(int socket_discordiador);
char* reparar();
int verificar_cant_bloques();
int verificar_bitmap();
int verificar_sizes();
int verificar_block_counts();
int verificar_blocks();
int lista_blocks_saboteada(FILE* archivo);
char* reparar();
void recorrer_recursos(t_list* lista_bloques_ocupados);
void recorrer_bitacoras(t_list* lista_bloques_ocupados);
void sortear(t_list* lista_bloques_ocupados);
int bloques_ocupados_difieren(t_list* lista_bloques_ocupados);
char* rompio(int codigo);

#endif
