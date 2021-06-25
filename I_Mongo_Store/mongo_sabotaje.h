#ifndef I_MONGO_SABOTAJE_H_
#define I_MONGO_SABOTAJE_H_

#include "mongo_tripulantes.h"

void enviar_posiciones_sabotaje(int socket_discordiador);
int reparar(t_TCB* tripulante);
int verificar_cant_bloques(t_TCB* tripulante);
int verificar_bitmap(t_TCB* tripulante);
int verificar_sizes(t_TCB* tripulante);
int verificar_block_counts(t_TCB* tripulante);
int verificar_blocks(t_TCB* tripulante);
void recorrer_recursos(int* lista_bloques_ocupados);
void recorrer_bitacoras(int* lista_bloques_ocupados);
void sortear(int* lista_bloques_ocupados);
int bloques_ocupados_difieren(int* lista_bloques_ocupados);
int reasignar_tamanios_archivo();
int revisar_block_count_recursos();
char* rompio(int codigo);

#endif
