#ifndef SEMAFOROS_H_
#define SEMAFOROS_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>

void doSignal(int semid, int numSem);

void doWait(int semid, int numSem);

void initSem(int semid, int numSem, int valor);

void freeSem(int semid);



#endif
