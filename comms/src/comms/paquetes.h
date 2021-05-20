#ifndef PAQUETES_H_
#define PAQUETES_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "estructuras.h"
#include "socketes.h"

typedef enum{
	MENSAJE,
	PEDIR_TAREA,
    COD_TAREA
}op_code;


typedef struct {
    uint32_t tamanio_estructura; 
    void* estructura;
} t_buffer;

typedef struct {
    op_code codigo_operacion;
    t_buffer* buffer;
} t_paquete;

t_buffer* serializar_tripulante(t_tripulante tripulante);
t_buffer* serializar_tarea(t_tarea tarea);
t_buffer* serializar_sabotaje();
void empaquetar(t_buffer buffer, int codigo_operacion, int socket_receptor);
t_estructura* recepcion_y_deserializacion(int socket_receptor);
t_tripulante* desserializar_tripulante(t_buffer* buffer);
t_tarea* desserializar_tarea(t_buffer* buffer);

/*
    Crea un paquete vacío con el código de operación pasado por parametro
*/
t_paquete* crear_paquete(op_code codigo);


/*
    Agrega lo que se le pase por el parametro valor a un paquete
*/
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);

/*
    Envia un paquete a un socket especifico
*/
void enviar_paquete(t_paquete* paquete, int socket_servidor);

/*
    Hace el free de memoria de un paquete
*/
void eliminar_paquete(t_paquete* paquete);

/*
    Crea el buffer para un paquete
*/
void crear_buffer(t_paquete* paquete);

void* recibir_paquete(int socket_cliente);

/*
    Serializa un paquete genérico
*/
void* serializar_paquete(t_paquete* paquete, int bytes);

#endif
