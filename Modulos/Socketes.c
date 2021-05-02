/*

Funcionalidad de sockets, cortesia de Nico

*/

#include "Socketes.h"


void sigchld_handler(int s)
{
 	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

int crear_socket_cliente(char* ip_del_servidor_a_conectar, char* puerto_del_servidor)
{
	struct addrinfo datos_para_server, *informacion_server;

	memset(&datos_para_server, 0, sizeof(datos_para_server));
	datos_para_server.ai_family = AF_UNSPEC; // Posible IPv4
	datos_para_server.ai_socktype = SOCK_STREAM;
	datos_para_server.ai_flags = AI_PASSIVE;

	if (int estado = getaddrinfo(ip_del_servidor_a_conectar, puerto_del_servidor, &datos_para_server, &informacion_server) != 0)
		printf("Error al conseguir informacion del servidor\n");

	int socket_cliente = socket(informacion_server -> ai_family, informacion_server -> ai_socktype, informacion_server -> ai_protocol);

	if (socket_cliente == -1) 
		printf("Error al crear socket\n");

	if (connect(socket_cliente, informacion_server -> ai_addr, informacion_server -> ai_addrlen) == -1)
		printf("Error al conectar cliente\n");

	freeaddrinfo(informacion_server);

	return socket_cliente;
}

int crear_socket_oyente(char *ip_del_servidor_a_conectar, char* puerto_del_servidor) { // Puerto debera ser definido de antemano
	struct addrinfo datos_para_server, *informacion_server;
	int socket_escucha;

	memset(&datos_para_server, 0, sizeof(datos_para_server));
	datos_para_server.ai_family = AF_UNSPEC;
	datos_para_server.ai_socktype = SOCK_STREAM;
	datos_para_server.ai_flags = AI_PASSIVE;

	if (int estado = getaddrinfo(ip_del_servidor_a_conectar, puerto_del_servidor, &datos_para_server, &informacion_server) != 0)
		printf("Error al conseguir informacion del servidor\n");

	socket_escucha = socket(informacion_server -> ai_family, informacion_server -> ai_socktype, informacion_server -> ai_protocol);

	if (socket_escucha == -1) 
		printf("Error al crear socket\n");

	if (bind(socket_escucha, informacion_server -> ai_addr, informacion_server -> ai_addrlen) == -1)
		printf("Error al conectar con el servidor\n");

	freeaddrinfo(informacion_server);

	return socket_escucha;
}

void escuchar(int socket_escucha) {
	struct sockaddr_storage direccion_a_escuchar;
	sockelen_t tamanio_direccion;
	int socket_especifico;

	if (listen(socket_escucha, 10) == -1) // Cola de aceptacion para clientear es de 10 - Nico
		printf("Error al configurar recepcion de mensajes\n");

	sa.sa_handler = sigchld_handler; // Limpieza de procesos muertos 
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		printf("Error al limpiar procesos\n");
		exit(1);
	}
	
	while(1) { // Loop infinito donde aceptara clientes
		tamanio_direccion = sizeof(direccion_a_escuchar);
		socket_especifico = accept(socket_escucha, (struct sockaddr*) &direccion_a_escuchar, &tamanio_direccion);

		if (!fork()) { // Proceso de hijo (al aceptar un socket se hace uno)
			close(socket_escucha);
			acciones_hijo(); // Indefinidas por ahora, podria pasarse la funcion por parametro acorde al modulo
			close(socket_especifico);
			exit(0);
		}

		close(socket_especifico);
	}
}



int enviar_mensaje(int socket, char* mensaje, int largo) { // Se podria definir largo en base al tipo mensaje y hardcodear parametro
	int bytes_enviados;
	
	bytes_enviados = send(socket, mensaje, largo, 0);

	if (bytes_enviados == -1)
		printf("No se envio el mensaje\n");
	else if (bytes_enviados /= largo) 
		printf("No se envio todo el mensaje\n");
	
	return bytes_enviados;
}

int recibir_mensaje(int socket, char* buffer, int largo) { 
	int bytes_recibidos;
	
	bytes_recibidos = recv(socket, buffer, largo, 0);

	switch(bytes_recibidos) {
		case (-1):
			printf("Error al recibir mensaje\n");
			break;
		case 0: 
			printf("El remoto ha cerrado la conexion\n");
			break;
	}

	return bytes_recibidos;
}