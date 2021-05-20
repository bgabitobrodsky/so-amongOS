
#include "utils.h"

int iniciar_servidor(void){
	int socket_servidor;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo("127.0.0.1", "25430", &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next){
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1){
            close(socket_servidor);
            continue;
        }
        break;
    }

	listen(socket_servidor, SOMAXCONN);
    freeaddrinfo(servinfo);
    log_info(logger, "Mi ram hq lista en el puerto 4444");
    return socket_servidor;
}

int esperar_discordiador(int socket_servidor){
	struct sockaddr_in dir_cliente;
	int tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (struct sockaddr*) &dir_cliente, &tam_direccion);

	return socket_cliente;
}

int leer_operacion(int socket_discordiador){

	op_code cod_op;

	if(recv(socket_discordiador, &cod_op, sizeof(int), MSG_WAITALL) != 0)
		return cod_op;
	else{
		close(socket_discordiador);
		return -1;
	}
}