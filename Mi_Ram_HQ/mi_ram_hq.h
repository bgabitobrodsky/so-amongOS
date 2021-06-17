/*
 * i_mongo_store.h
 *
 *  Created on: 9 may. 2021
 *      Author: utnso
 */

#ifndef MI_RAM_HQ_H_
#define MI_RAM_HQ_H_

#include "utils.h"

//#include <stdio.h>
//#include <stdlib.h>
//#include <pthread.h>
//#include <commons/string.h>
//#include <commons/log.h>
//#include <commons/config.h>
//#include <comms/paquetes.h>
//#include <comms/estructuras.h>
//#include <comms/socketes.h>


void atender_clientes(void*);
void proceso_handler(void* args);
t_PCB* crear_pcb(char* path);
t_TCB crear_tcb(t_PCB* pcb, int tid, char* posicion);


#endif /* MI_RAM_HQ_H_ */
