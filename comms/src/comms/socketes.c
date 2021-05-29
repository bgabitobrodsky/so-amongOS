/*

 Funcionalidad de sockets, cortesia de Nico

*/

#include "socketes.h"


// Funcion extraida de la Guia Beej, para limpieza de procesos muertos
void sigchld_handler(int s) {
	int saved_errno = errno;

	while (waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

// Funcion para crear un socket modalidad cliente
int crear_socket_cliente(char* ip_del_servidor_a_conectar, char* puerto_del_servidor) { 
	printf("escuchar cliente");
	struct addrinfo datos_para_server, *informacion_server, *p;
	int socket_cliente;
	int estado;

	memset(&datos_para_server, 0, sizeof(datos_para_server)); // Se settea en 0 la var datos_para_server
	datos_para_server.ai_family = AF_UNSPEC; // 
	datos_para_server.ai_socktype = SOCK_STREAM; // Se indica la modalidad de la comunicacion: no declaro formato de ip, socket stream y que se rellene la IP propia del cliente 
	datos_para_server.ai_flags = AI_PASSIVE; //

	if ((estado = getaddrinfo(ip_del_servidor_a_conectar, puerto_del_servidor, &datos_para_server, &informacion_server)) != 0) // Obtengo la informacion del server y la alojo en informacion_server, utilizando los datos predefinidos arriba para settear
		printf("Error al conseguir informacion del servidor\n"); // Al mismo tiempo se verifica si el proceso funciono correctamente

	for (p = informacion_server; p != NULL; p = p->ai_next){ // Ciclo por todas las opciones hasta encontrar una
		
        if ((socket_cliente = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) // Si crear socket falla, sigue intentando
            continue;

        if (connect(socket_cliente, informacion_server -> ai_addr, informacion_server -> ai_addrlen) == -1) { // Si no conecta, cierra e intenta a otro
            close(socket_cliente);
            continue;
        }

        break;
    }


	freeaddrinfo(informacion_server); // Libero la informacion del server total ya no sirve

	return socket_cliente;
}

// Funcion para crear un socket que escuchara llamados de conexion en un servidor
int crear_socket_oyente(char *ip_del_servidor_a_conectar, char* puerto_del_servidor) { 
	struct addrinfo datos_para_server, *informacion_server, *p;
	int socket_escucha;
	int estado;

	memset(&datos_para_server, 0, sizeof(datos_para_server));
	datos_para_server.ai_family = AF_UNSPEC;
	datos_para_server.ai_socktype = SOCK_STREAM;
	datos_para_server.ai_flags = AI_PASSIVE;

	if ((estado = getaddrinfo(ip_del_servidor_a_conectar, puerto_del_servidor, &datos_para_server, &informacion_server)) != 0)
		printf("Error al conseguir informacion del servidor\n");


	for (p = informacion_server; p != NULL; p = p->ai_next){
		
        if ((socket_escucha = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        if (bind(socket_escucha, p->ai_addr, p->ai_addrlen) == -1){
            close(socket_escucha);
            continue;
        }

        break;
    }

	freeaddrinfo(informacion_server); // Libero la informacion del server total ya no sirve

	return socket_escucha;
}

// Funcion para escuchar llamados, con un socket de la funcion anterior, faltaria  //TODO
void escuchar(int socket_escucha) {
	struct sockaddr_storage direccion_a_escuchar;
	socklen_t tamanio_direccion;
	int socket_especifico; // Sera el socket hijo que hara la conexion con el cliente

	if (listen(socket_escucha, 10) == -1) // Se pone el socket a esperar llamados, con una cola maxima dada por el 2do parametro, se eligio 10 arbitrariamente //TODO esto esta hardcodeado
		printf("Error al configurar recepcion de mensajes\n"); // Se verifica

	/*sa.sa_handler = sigchld_handler; // Limpieza de procesos muertos, ctrl C ctrl V del Beej, porlas
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		printf("Error al limpiar procesos\n");
		exit(1);
	}*/

	while (1) { // Loop infinito donde aceptara clientes
		tamanio_direccion = sizeof(direccion_a_escuchar);
		socket_especifico = accept(socket_escucha, (struct sockaddr*) &direccion_a_escuchar, &tamanio_direccion); // Se acepta (por FIFO si no me equivoco) el llamado entrante a socket escucha

		if (!fork()) { // Se crea un proceso hijo si se pudo forkear correctamente
			close(socket_escucha); // Cierro escucha en este hilo, total no sirve mas
			// funcion_de_hijos(); // Funcion enviada por parametro con puntero para que ejecuten los hijos del proceso
			close(socket_especifico); // Cumple proposito, se cierra socket hijo
			exit(0); // Returnea
		}

		close(socket_especifico); // En hilo padre se cierra el socket hijo, total al arrancar el while se vuelve a settear, evita "port leaks" supongo
	}
}

// La funcion send pero con verificacion y simplificada
// Lo enviado en mensaje sera un buffer serializado previamente con las funciones de paquetes
int enviar_mensaje(int socket, void* mensaje, int largo) { // Se podria definir largo en base al tipo mensaje y hardcodear parametro
	int bytes_enviados;

	bytes_enviados = send(socket, mensaje, largo, 0); // Envio el mensaje dado, y recibo la cantidad de bytes que mande

	if (bytes_enviados == -1) // Pudieron no mandarse, verifico
		printf("No se envio el mensaje\n");
	else if (bytes_enviados /= largo) // Pueden ser menos de los que queria, verifico (hasta 1KB safe si no me equivoco)
		printf("No se envio todo el mensaje\n");

	return bytes_enviados;
}


// La funcion recv pero con verificacion y simplificada
// Lo importante va a estar en el buffer de todas formas, que luego se utilizara en conjunto con las funciones de paquetes
int recibir_mensaje(int socket, void* buffer, int largo) { 
	int bytes_recibidos;

	bytes_recibidos = recv(socket, buffer, largo, 0); // Recibo un mensaje, y recibo la cantidad de bytes que recibi

	switch (bytes_recibidos) {
	case (-1):
		printf("Error al recibir mensaje\n"); // Pudo no recibirse nada
		break;
	case 0:
		printf("El remoto ha cerrado la conexion\n"); // Pudo haberse cerrado el socket opuesto, forma de saberlo
		break;
	}

	return bytes_recibidos;
}
