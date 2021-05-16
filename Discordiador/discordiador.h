/*
 * Discordiador.h
 *
 *  Created on: 25 abr. 2021
 *      Author: utnso
 */

#ifndef DISCORDIADOR_H_
#define DISCORDIADOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <readline/readline.h>
#include "utils.h"
#include <time.h>
#include <Libreria_Modulos/paquetes.h>
#include <Libreria_Modulos/socketes.h>
#include <Libreria_Modulos/estructuras.h>

void leer_consola();
void iniciar_patota(char* leido);
void listar_tripulantes();
void iniciar_planificacion();
void pausar_planificacion();
void obtener_bitacora(char* leido);
void expulsar_tripulante(char* leido);
void tripulante();
int pedir_tarea(int id_tripulante);
t_paquete* crear_paquete(op_code codigo);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_servidor);
void eliminar_paquete(t_paquete* paquete);
void crear_buffer(t_paquete* paquete);
char* fecha_y_hora();

#endif /* DISCORDIADOR_H_ */
