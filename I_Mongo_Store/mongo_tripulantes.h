#ifndef I_MONGO_TRIPULANTES_H_
#define I_MONGO_TRIPULANTES_H_

#include "mongo_archivos.h"

extern t_log* logger_mongo;
extern t_config* config_mongo;
extern t_directorio directorio;
extern t_recurso recurso;
extern t_list* bitacoras;

void manejo_tripulante(int socket_tripulante);
void crear_estructuras_tripulante(t_TCB* tcb, int socket_tripulante);
void acomodar_bitacora(FILE* file_tripulante, t_TCB* tcb);
void modificar_bitacora(t_estructura* mensaje);
void escribir_bitacora(t_bitacora* bitacora, int largo_strings, int cant_strings, ...);
void escribir_bloque_bitacora(int bloque, char* mensaje);
char* formatear_posicion(int coord_x, int coord_y);
void borrar_bitacora(t_TCB* tcb);
t_bitacora* quitar_bitacora_lista(t_TCB* tcb);
t_bitacora* obtener_bitacora(t_TCB* tcb);
char* fpath_tripulante(t_TCB* tcb);

#endif
