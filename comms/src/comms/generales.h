#ifndef GENERALES_H_
#define GENERALES_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/log.h>
#include "estructuras.h"

// Generales
void liberar_puntero_doble(char** palabra);
t_tarea* crear_tarea(char* string_tarea);
char* leer_archivo_entero(char* path);
int comparar_strings(char* str, char* str2);
int contar_palabras(char** palabras);
char* fecha_y_hora();
int esta_en_lista(t_list* lista, int elemento);
int sonIguales(int elemento1, int elemento2);

// Monitores
void monitor_cola_push(pthread_mutex_t semaforo, t_queue* cola, void* elemento_a_insertar);
void* monitor_cola_pop(pthread_mutex_t semaforo, t_queue* cola);
void* monitor_cola_pop_or_peek(pthread_mutex_t semaforo, void*(*operacion)(t_queue*), t_queue* cola);
void* monitor_lista_dos_parametros(pthread_mutex_t semaforo, void*(*operacion)(t_list*, void*), t_list* lista, void* elemento);

#endif
