#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

/* ENUMS */
//                      				ESTRUCTURAS                          		COSAS FILESYSTEM            ACCIONES BITACORA                                                           CODIGOS UNICOS: MONGO           	DISCORDIADOR                    GENERALES
enum codigo_operacion { RECIBIR_PCB, RECIBIR_TCB, TAREA, ARCHIVO_TAREAS,     OXIGENO, COMIDA, BASURA,    MOVIMIENTO, INICIO_TAREA, FIN_TAREA, CORRE_SABOTAJE, RESUELVE_SABOTAJE,     SABOTAJE, PRIMERA_CONEXION,     MENSAJE, PEDIR_TAREA, COD_TAREA,     RECEPCION, DESCONEXION };

enum estados { NEW, READY, EXCECUTING, BLOCKED};

/* ESTRUCTURAS */

typedef struct {

    uint32_t PID;
    uint32_t direccion_tareas;

} t_PCB;

/*
typedef struct {
    t_PCB* pcb;
} t_patota;
*/

typedef struct {

    uint32_t TID;
    char estado_tripulante;
    uint32_t coord_x;
    uint32_t coord_y;
    uint32_t siguiente_instruccion;
    uint32_t puntero_a_pcb;

} t_TCB;

/*
typedef struct { // Puede estar de mas
    t_TCB* tcb;
} t_tripulante;
*/

typedef struct {

    uint32_t largo_nombre;
    char* nombre;
    uint32_t parametro; // Siempre es un int, a menos que sea DESCARTAR_BASURA que no lleva nada
    uint32_t coord_x;
    uint32_t coord_y;
    uint32_t duracion; // En ciclos de CPU

} t_tarea;

typedef struct {

	uint32_t largo_texto;
    char* texto;
    uint32_t pid;

} t_archivo_tareas;

typedef struct { // Solucion nefasta a no poder retornar varios tipos de struct de una funcion

    t_TCB* tcb;
    t_PCB* pcb;
    t_tarea* tarea;
    t_archivo_tareas* archivo_tareas;
    int codigo_operacion;
    int cantidad; // Revisar funcs paquetes

} t_estructura;

typedef struct {

    int socket_oyente;

} args_escuchar;


#endif
