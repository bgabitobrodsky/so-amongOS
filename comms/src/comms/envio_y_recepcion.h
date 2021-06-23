#ifndef EMPAQUETADO_ENVIADO_RECEPCION_H_
#define EMPAQUETADO_ENVIADO_RECEPCION_H_

#include <CUnit/Basic.h>
#include "serializar_y_envio.h"

int socket_a_mi_ram_hq;
int socket_a_mongo_store;

int CUmain_envio_y_recepcion();
void test_empaquetar_y_enviar_tcb_ram();
void test_empaquetar_y_enviar_tarea();
void test_empaquetar_y_enviar_sabotaje();
void test_empaquetar_y_enviar_mensaje();
void test_empaquetar_y_enviar_pedir_tarea();
void test_empaquetar_y_enviar_codigo_tarea();
void test_empaquetar_y_enviar_recepcion();
void test_empaquetar_y_enviar_desconexion();

#endif /* EMPAQUETADO_ENVIADO_RECEPCION_H_ */
