#ifndef LOCKS_H_
#define LOCKS_H_

#include "mongo_blocks.h"

t_dictionary* diccionarioLocks;

void crearDiccionarioLocks();
void lockearLectura(char*);
void lockearEscritura(char*);
void unlockear(char*);
void  verificarExistencia(char*);

#endif /* LOCKS_H_ */
