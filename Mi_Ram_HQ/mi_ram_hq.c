/*
 * mi_ram_hq.c
 *
 *  Created on: 9 may. 2021
 *      Author: utnso
 */

#include "mi_ram_hq.h"

#define	IP_MI_RAM_HQ config_get_string_value(config_miramhq, "IP_MI_RAM_HQ")
#define PUERTO_MI_RAM_HQ config_get_string_value(config_miramhq, "PUERTO_MI_RAM_HQ")

// Vars globales
t_log* logger_miramhq;
t_config* config_miramhq;

int main(int argc, char** argv) {
	logger_miramhq = log_create("mi_ram_hq.log", "MI_RAM_HQ", 1, LOG_LEVEL_DEBUG);
	config_miramhq = config_create("mi_ram_hq.config");
	char* tamanio_memoria = config_get_string_value(config_miramhq, "TAMANIO_MEMORIA");
	char* memoria = malloc(atoi(tamanio_memoria));
	free(tamanio_memoria);
    	
	//int socket_oyente = crear_socket_oyente("127.0.0.1", "25430"); // Se podria delegar a un hilo
    	int socket_oyente = crear_socket_oyente("127.0.0.1", "25430"); // TODO: HARCODEADO HASTA CAMBIARR EL CONFIG
	args_escuchar_miram args_miram;
	args_miram.socket_oyente = socket_oyente;

	pthread_t hilo_escucha;
	pthread_create(&hilo_escucha, NULL, (void*) escuchar_miram, (void*) &args_miram);
	pthread_join(hilo_escucha, NULL);

	close(socket_oyente);
	log_destroy(logger_miramhq);
	config_destroy(config_miramhq);
	free(memoria);

	return EXIT_SUCCESS;
}

void atender_clientes(int socket_hijo) {
	    while(1) { //TODO cambiar por do while  y liberar la estructura
			t_estructura* mensaje_recibido = recepcion_y_deserializacion(socket_hijo); // Hay que pasarle en func hijos dentro de socketes.c al socket hijo, y actualizar los distintos punteros a funcion
		
			switch(mensaje_recibido->codigo_operacion) {
				case MENSAJE:
					log_info(logger_miramhq, "Mensaje recibido");
					break;

				case PEDIR_TAREA:
					log_info(logger_miramhq, "Pedido de tarea recibido");
					break;

				case COD_TAREA:
					printf("Recibo una tarea");
					break;

				case -1: // Puede que rompa si discordiador no envia mensajes, mejor hacerlo un codigo de por si
					log_info(logger_miramhq, "Se desconecto el modulo Discordiador");
		}
			free(mensaje_recibido);
	}
}

void escuchar_miram(void* args) { // No se libera args, ver donde liberar
	args_escuchar_miram* p = malloc(sizeof(args_escuchar_miram));
	p = args;
	int socket_escucha = p->socket_oyente;

	struct sockaddr_storage direccion_a_escuchar;
	socklen_t tamanio_direccion;
	int socket_especifico; // Sera el socket hijo que hara la conexion con el cliente

	if (listen(socket_escucha, 10) == -1) // Se pone el socket a esperar llamados, con una cola maxima dada por el 2do parametro, se eligio 10 arbitrariamente //TODO esto esta hardcodeado
		printf("Error al configurar recepcion de mensajes\n"); // Se verifica

	while (1) { // Loop infinito donde aceptara clientes
			tamanio_direccion = sizeof(direccion_a_escuchar);
			socket_especifico = accept(socket_escucha, (struct sockaddr*) &direccion_a_escuchar, &tamanio_direccion); // Se acepta (por FIFO si no me equivoco) el llamado entrante a socket escucha

			if (!fork()) { // Se crea un proceso hijo si se pudo forkear correctamente
				close(socket_escucha); // Cierro escucha en este hilo, total no sirve mas
				atender_clientes(socket_especifico); // Funcion enviada por parametro con puntero para que ejecuten los hijos del proceso
				close(socket_especifico); // Cumple proposito, se cierra socket hijo
				exit(0); // Returnea
			}

			close(socket_especifico); // En hilo padre se cierra el socket hijo, total al arrancar el while se vuelve a settear, evita "port leaks" supongo
		}
}

t_patota* iniciar_patota(FILE* archivo){
	t_PCB* pcb = malloc(sizeof(t_PCB));
	//pcb->PID = nuevo_pid();//TODO A discusión de como sacar el pid
	pcb->direccion_tareas = &archivo;

	t_patota* patota = malloc(sizeof(t_patota));
	patota->archivo_de_tareas = archivo;
	patota->pcb = pcb;

	//cargar_en_Mongo(archivo);

	return patota;
}
/*

}*/

//Iniciar tripulante: será el encargado de crear la o las estructuras
//administrativas necesarias para que un tripulante pueda ejecutar.

t_tripulante* iniciar_tripulante(char* posicion, t_PCB* puntero_pcb, int tid){
	t_TCB* tcb = malloc(sizeof(t_TCB));
	tcb->TID = tid;
	tcb->estado_tripulante = LLEGADA; //Supongo que se inicializa en LLEGADA por defecto
	tcb->coord_x = posicion[0];
	tcb->coord_y = posicion[2];
	//tcb->siguiente_instruccion = ;//Ni idea de que va acá
	tcb->puntero_a_pcb = puntero_pcb;

	t_tripulante* tripulante = malloc(sizeof(t_tripulante));
	//tripulante->codigo = ;//Ni idea de que va acá
	tripulante->tcb = tcb;
	return tripulante;
}
