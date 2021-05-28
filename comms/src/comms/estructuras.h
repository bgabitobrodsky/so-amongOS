#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
//#include "stdbool.h"

enum codigo_operacion { TRIPULANTE, TAREA, SABOTAJE, MENSAJE, PEDIR_TAREA, COD_TAREA };
enum estado_tripulante { LLEGADA, LISTO, TRABAJANDO, BLOQUEADO, FINALIZADO };


typedef struct { // Debe estar de mas, es lo mismo hacer varios structs de tripu y tareas, y es mas lindo asi, la dejo como vestigio porlas

//uint32_t cantidad_integrantes; //NO IRIA, se actualiza cada vez que se finaliza un tripulante
    FILE* archivo_de_tareas;
    t_PCB* pcb;
} t_patota;

typedef struct {

    uint32_t PID;
    uint32_t direccion_tareas;

} t_PCB;

typedef struct { // Puede estar de mas

    uint32_t codigo;
    uint32_t coord_x;
    uint32_t coord_y;
    uint32_t estado;
    t_TCB* tcb;

} t_tripulante;

typedef struct {

    uint32_t TID;
    char estado_tripulante;
    uint32_t coord_x;
    uint32_t coord_y;
    uint32_t siguiente_instruccion;
    uint32_t puntero_a_pcb;

} t_TCB;


typedef struct {

    uint32_t nombre_largo;
    char* nombre;
    uint32_t parametro; // Siempre es un int, a menos que sea DESCARTAR_BASURA que no lleva nada
    uint32_t coord_x;
    uint32_t coord_y;
    uint32_t duracion; // En ciclos de CPU

} t_tarea;


typedef struct { // Solucion nefasta a no poder retornar varios tipos de struct de una funcion

    t_tripulante* tripulante;
    t_tarea* tarea;
    int codigo_operacion;

} t_estructura;

#endif
