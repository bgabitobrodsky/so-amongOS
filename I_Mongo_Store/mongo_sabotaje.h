#ifndef I_MONGO_SABOTAJE_H_
#define I_MONGO_SABOTAJE_H_

#include "mongo_tripulantes.h"

void enviar_posiciones_sabotaje(int socket_discordiador);
int reparar();
int verificar_cant_bloques();
int verificar_bitmap();
int verificar_sizes();
int verificar_block_counts();
int verificar_blocks();
void recorrer_recursos(int* lista_bloques_ocupados);
void recorrer_bitacoras(int* lista_bloques_ocupados);
void sortear(int* lista_bloques_ocupados);
int bloques_ocupados_difieren(int* lista_bloques_ocupados);
int contiene(int* lista, int valor);
char* rompio(int codigo);

#endif
