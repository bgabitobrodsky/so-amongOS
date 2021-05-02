#ifndef SOCKETES_H_
#define SOCKETES_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<string.h>

void sigchld_handler(int s);
int crear_socket_cliente(char *ip_del_servidor_a_conectar, char* puerto_del_servidor);
int crear_socket_oyente(char *ip_del_servidor_a_conectar, char* puerto_del_servidor);
void escuchar(int socket_escucha);
int enviar_mensaje(int socket, char* mensaje, int largo);
int recibir_mensaje(int socket, char* buffer, int largo);

#endif
