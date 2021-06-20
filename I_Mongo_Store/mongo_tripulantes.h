#ifndef I_MONGO_TRIPULANTES_H_
#define I_MONGO_TRIPULANTES_H_

#include "mongo_blocks.h"

extern t_log* logger_mongo;
extern t_config* config_mongo;
extern t_archivos archivos;
extern t_list* bitacoras;

void manejo_tripulante(int socket_tripulante);
void crear_estructuras_tripulante(t_TCB* tcb, int socket_tripulante);
void acomodar_bitacora(FILE* file_tripulante, t_TCB* tcb);
void modificar_bitacora(int codigo_operacion, t_TCB* tcb);
void borrar_bitacora(t_TCB* tcb);
int obtener_indice_bitacora(t_TCB* tcb);
char* fpath_bitacoras();
char* fpath_tripulante(char* path_directorio, t_TCB* tcb);

#endif
