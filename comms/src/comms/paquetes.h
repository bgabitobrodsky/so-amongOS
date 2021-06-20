#ifndef PAQUETES_H_
#define PAQUETES_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "estructuras.h"
#include "socketes.h"

typedef struct {

    uint32_t tamanio_estructura; 
    void* estructura;

} t_buffer;

typedef struct {

    int codigo_operacion;
    t_buffer* buffer;

} t_paquete;

t_buffer* serializar_tcb(t_TCB tcb);
t_buffer* serializar_tarea(t_tarea tarea);
t_buffer* serializar_vacio();
void empaquetar_y_enviar(t_buffer* buffer, int codigo_operacion, int socket_receptor);
void enviar_codigo(int codigo_operacion, int socket_receptor);
t_estructura* recepcion_y_deserializacion(int socket_receptor);
t_TCB* deserializar_tcb(t_buffer* buffer);
t_tarea* deserializar_tarea(t_buffer* buffer);
void eliminar_paquete(t_paquete* paquete);
t_buffer* serializar_cantidad(int cantidad);
t_archivo_tareas* deserializar_archivo_tareas(t_buffer* buffer);
t_buffer* serializar_archivo_tareas(t_archivo_tareas texto_archivo);
t_buffer* serializar_entero(uint32_t numero);

#endif
