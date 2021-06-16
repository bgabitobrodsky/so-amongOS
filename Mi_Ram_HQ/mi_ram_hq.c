/*
 * mi_ram_hq.c
 *
 *  Created on: 9 may. 2021
 *      Author: utnso
 */

//TODO: por alguna razon, no funcionan los printf en este modulo.

#include "mi_ram_hq.h"
#include <comms/generales.h>


#define	IP_MI_RAM_HQ config_get_string_value(config_miramhq, "IP")
#define PUERTO_MI_RAM_HQ config_get_string_value(config_miramhq, "PUERTO")
#define TAMANIO_MEMORIA config_get_int_value(config_miramhq, "TAMANIO_MEMORIA")

// Vars globales
t_log* logger_miramhq;
t_config* config_miramhq;

t_list* lista_tcb;
t_list* lista_pcb;

char estado_tripulante[4] = {'N', 'R', 'E', 'B'};

int main(int argc, char** argv) {
	logger_miramhq = log_create("mi_ram_hq.log", "MI_RAM_HQ", 1, LOG_LEVEL_DEBUG);
	config_miramhq = config_create("mi_ram_hq.config");

	//char* memoria = malloc(TAMANIO_MEMORIA);
	char* memoria = malloc(2048);

	lista_tcb = list_create();
	lista_pcb = list_create();

    //int socket_oyente = crear_socket_oyente(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
    int socket_oyente = crear_socket_oyente("127.0.0.1", "25430");
    //escuchar_miram((void*) socket_oyente);//debug

    args_escuchar args_miram;
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

void atender_clientes(int socket_hijo) { // TODO miram no termina ni siquiera si muere discordiador. una forma de arreglarlo es hacer que estas funciones devuelvan valores.
	int flag = 1;

	t_tarea* t = crear_tarea("GENERAR_OXIGENO 12;2;3;5");
	t_buffer* b = serializar_tarea(*t);
	empaquetar_y_enviar(b, 2, socket_hijo);

	log_info(logger_miramhq, "Atendiendo.\n");

	    while(flag) {
	    	t_estructura* mensaje_recibido = recepcion_y_deserializacion(socket_hijo); // Hay que pasarle en func hijos dentro de socketes.c al socket hijo, y actualizar los distintos punteros a funcion
	    	sleep(1); //para que no se rompa en casos de bug o tiempos de espera

			switch(mensaje_recibido->codigo_operacion) {

				case MENSAJE:
					//log_info(logger_miramhq, "Mensaje recibido\n");
					break;

				case PEDIR_TAREA:
					log_info(logger_miramhq, "Pedido de tarea recibido\n");
					break;

				case TAREA:
                    printf("Tarea recibida! ");
                    printf("Largo nombre: %i\n", mensaje_recibido->tarea->largo_nombre);
                    printf("Nombre: %s\n", mensaje_recibido->tarea->nombre);
                    printf("Parametros: %i\n", mensaje_recibido->tarea->parametro);
                    printf("Cordenada en X: %i\n", mensaje_recibido->tarea->coord_x);
                    printf("Cordenada en Y: %i\n", mensaje_recibido->tarea->coord_y);
                    printf("Duracion: %i\n", mensaje_recibido->tarea->duracion);
					break;

				case COD_TAREA:
					log_info(logger_miramhq, "Recibo una tarea\n");
					//free(mensaje_recibido->tarea);
					break;

				case RECIBIR_PCB:
					//log_info(logger_miramhq, "Recibo una pcb\n");
					//list_add(lista_pcb, (void*) mensaje_recibido->pcb);
					//almacenar_pcb(mensaje_recibido); //TODO en un futuro, capaz no podamos recibir el PCB por quedarnos sin memoria
					free(mensaje_recibido->pcb);
					break;

				case RECIBIR_TCB:
					// log_info(logger_miramhq, "Recibo una tcb\n");
					// log_info(logger_miramhq, "Tripulante %i, estado: %c, pos: %i %i, puntero_pcb: %i, sig_ins %i\n", (int) mensaje_recibido->tcb->TID, (char) mensaje_recibido->tcb->estado_tripulante, (int) mensaje_recibido->tcb->coord_x, (int) mensaje_recibido->tcb->coord_y, (int) mensaje_recibido->tcb->puntero_a_pcb, (int) mensaje_recibido->tcb->siguiente_instruccion);
					list_add(lista_tcb, (void*) mensaje_recibido->tcb);
					free(mensaje_recibido->tcb);
					//printf("Tripulante 1 pos: %c %c\n", (int) mensaje_recibido->tcb->coord_x, (int) mensaje_recibido->tcb->coord_y);
					break;

				case DESCONEXION:
					log_info(logger_miramhq, "Se desconecto el modulo Discordiador");
					flag = 0;
					break;

				default:
					log_info(logger_miramhq, "Se recibio un codigo invalido.\n");
					//printf("El codigo es %d\n", mensaje_recibido->codigo_operacion);
					break;
			}
			free(mensaje_recibido);
	    }

}

void escuchar_miram(void* args) { // No se libera args, ver donde liberar
	args_escuchar* p = malloc(sizeof(args_escuchar));
	p = args;
	int socket_escucha = p->socket_oyente;
	//int socket_escucha = (int) args; //Tema de testeos, no borrar

	struct sockaddr_storage direccion_a_escuchar;
	socklen_t tamanio_direccion;
	int socket_especifico; // Sera el socket hijo que hara la conexion con el cliente

	if (listen(socket_escucha, 10) == -1) // Se pone el socket a esperar llamados, con una cola maxima dada por el 2do parametro, se eligio 10 arbitrariamente //TODO esto esta hardcodeado
		printf("Error al configurar recepcion de mensajes\n");

	struct sigaction sa;
		sa.sa_handler = sigchld_handler;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = SA_RESTART;
		if (sigaction(SIGCHLD, &sa, NULL) == -1) {
			printf("Error al limpiar procesos\n");
			exit(1);
		}

	while (1) { // Loop infinito donde aceptara clientes
			tamanio_direccion = sizeof(direccion_a_escuchar);
			socket_especifico = accept(socket_escucha, (struct sockaddr*) &direccion_a_escuchar, &tamanio_direccion); // Se acepta (por FIFO si no me equivoco) el llamado entrante a socket escucha

			if (!fork()) { // Se crea un proceso hijo si se pudo forkear correctamente
				close(socket_escucha); // Cierro escucha en este hilo, total no sirve mas
				atender_clientes(socket_especifico); // Funcion enviada por parametro con puntero para que ejecuten los hijos del proceso
				close(socket_especifico); // Cumple proposito, se cierra socket hijo
				exit(0); // Returnea
			} //comentar fork para debuggeo

			close(socket_especifico); // En hilo padre se cierra el socket hijo, total al arrancar el while se vuelve a settear, evita "port leaks" supongo
		}
}


t_PCB* crear_pcb(char* path){
	t_PCB* pcb = malloc(sizeof(t_PCB));
	pcb -> PID =1;
	pcb -> direccion_tareas = (uint32_t) path;
	return pcb;
}


t_TCB crear_tcb(t_PCB* pcb, int tid, char* posicion){
	t_TCB tcb;
	tcb.TID = tid;
	tcb.estado_tripulante = estado_tripulante[NEW];
	tcb.coord_x = posicion[0];
	tcb.coord_y = posicion[2];
	tcb.siguiente_instruccion = 5; //TODO
	tcb.puntero_a_pcb = 7;

	return tcb;
}
