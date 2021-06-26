/*
 * Discordiador.h
 *
 *  Created on: 25 abr. 2021
 *      Author: utnso
 */

#ifndef DISCORDIADOR_H_
#define DISCORDIADOR_H_

#include "test_discordiador.h"
#include "../comms/src/comms/envio_y_recepcion.h"

//FUnciones de testeo
enum {DISCORDIADOR, TEST_SERIALIZACION, TEST_ENVIO_Y_RECEPCION, TEST_DISCORDIADOR};
int correr_tests(int);
void funcion_hilo();
void iniciar_hilo();

// Funciones PRINCIPALES
int iniciar_patota(char* leido);
void listar_tripulantes();
void expulsar_tripulante(char* leido);
void iniciar_planificacion();
void pausar_planificacion();
void obtener_bitacora(char* leido);

// HILOS
void tripulante(t_tripulante* un_tripulante);
void iniciar_tripulante(t_tripulante* un_tripulante, int socket);
void enlistarse(t_tripulante* un_tripulante, int socket);
void realizar_tarea(t_tripulante* un_tripulante, int socket);
int identificar_tarea(char* nombre_recibido);
void crear_hilo_tripulante(t_tripulante* un_tripulante);
t_list* lista_tripulantes_patota(uint32_t pid);
void morir(t_tripulante* un_tripulante);
void conseguir_siguiente_tarea(t_tripulante* un_tripulante, int socket);
int llegue(t_tripulante* un_tripulante);
void atomic_llegar_a_destino(t_tripulante* un_tripulante, int socket);
void atomic_no_me_despierten_estoy_trabajando(t_tripulante* un_tripulante, int socket);
void quitar_tripulante_de_listas(t_tripulante* un_tripulante);
void verificar_cambio_estado(char* estado_guardado, t_tripulante* un_tripulante, int socket);
void actualizar_tripulante(t_tripulante* un_tripulante, int socket);

// FUNCIONES AUXILIARES
void leer_consola();
void planificador();

#endif /* DISCORDIADOR_H_ */
