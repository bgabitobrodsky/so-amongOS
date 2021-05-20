/*
 * i_mongo_store.c
 *
 *  Created on: 9 may. 2021
 *      Author: utnso
 */

#include "i_mongo_store.h"

#define	IP_MONGO_STORE config_get_string_value(config, "IP_MONGO_STORE"); // Verificar sintaxis
#define PUERTO_MONGO_STORE config_get_string_value(config, "PUERTO_MONGO_STORE");

int main(int argc, char** argv){

    pthread_t hilo_escucha;
    (void*) p_escuchar_alos_cliente = &escuchar_alos_cliente();
	pthread_create(&hilo_escucha, NULL, p_escuchar_alos_cliente, NULL);
	pthread_join(hilo_escucha, NULL);

    free(p_escuchar_alos_cliente);
}


void escuchar_alos_cliente(){
    int socket_oyente = crear_socket_oyente(IP_MONGO_STORE, PUERTO_MONGO_STORE);
    (void*) p_hola = &hola();
    escuchar(socket_oyente, p_hola);
    free(p_hola);
}

void hola(){
    printf("Hola");
}
