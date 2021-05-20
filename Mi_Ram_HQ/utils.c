
#include "utils.h"

int leer_operacion(int socket_discordiador){

	op_code cod_op;

	if(recv(socket_discordiador, &cod_op, sizeof(int), MSG_WAITALL) != 0)
		return cod_op;
	else{
		close(socket_discordiador);
		return -1;
	}
}