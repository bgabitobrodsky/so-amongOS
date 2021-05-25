
#include "semaforos.h"

void doSignal(int semid, int numSem) {
    struct sembuf sops; //Signal
    sops.sem_num = numSem;
    sops.sem_op = 1;
    sops.sem_flg = 0;

    if (semop(semid, &sops, 1) == -1) {
        printf("Error al hacer Signal\n");
    }
}

void doWait(int semid, int numSem) {
    struct sembuf sops;
    sops.sem_num = numSem; /* Sobre el primero, ... */
    sops.sem_op = -1; /* ... un wait (resto 1) */
    sops.sem_flg = 0;

    if (semop(semid, &sops, 1) == -1) {
        printf("Error al hacer el wait\n");
    }
}

void initSem(int semid, int numSem, int valor) { //iniciar un semaforo
  
    if (semctl(semid, numSem, SETVAL, valor) < 0) {        
        printf("Error iniciando semaforo\n");
    }
}

void freeSem(int semid){
    if ((semctl(semid, 0, IPC_RMID)) == -1) {
        printf("Error al liberar el semaforo");
    }
}