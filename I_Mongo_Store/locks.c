#include "locks.h"
extern pthread_mutex_t sem_diccionario;


void crearDiccionarioLocks() {
	diccionarioLocks = dictionary_create();
}

void lockearLectura(char* path) {
	verificarExistencia(path);
	pthread_rwlock_rdlock(dictionary_get(diccionarioLocks,path));
}

void lockearEscritura(char* path) {
	verificarExistencia(path);
	pthread_rwlock_wrlock(dictionary_get(diccionarioLocks,path));
}

void unlockear(char* path) {
	verificarExistencia(path);
	pthread_rwlock_unlock(dictionary_get(diccionarioLocks,path));
}

void  verificarExistencia(char* path) {
	pthread_mutex_lock(&sem_diccionario);
	if(!dictionary_has_key(diccionarioLocks,path)){
		log_trace(logger_mongo, "No existia el lock");
		pthread_rwlock_t* unLock = malloc(sizeof(pthread_rwlock_t));
		pthread_rwlock_init(unLock,NULL);
		dictionary_put(diccionarioLocks,path,unLock);
	}
	pthread_mutex_unlock(&sem_diccionario);
}
