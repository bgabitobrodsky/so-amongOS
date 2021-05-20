/*
 * mi_ram_hq.c
 *
 *  Created on: 9 may. 2021
 *      Author: utnso
 */

#include "mi_ram_hq.h"

int main(int argc, char** argv){

	logger = log_create("mi_ram_hq.log", "MI_RAM_HQ", 1, LOG_LEVEL_DEBUG);
	config = config_create("mi_ram_hq.config");
	//config_discordiador = config_create("../Discordiador/discordiador.config");

	int tamanio_memoria = config_get_string_value(config, "TAMANIO_MEMORIA");
	char* memoria = malloc(tamanio_memoria);
	
    int mi_ram_fd = iniciar_servidor();

    int discordiador_fd = esperar_discordiador(mi_ram_fd);

    while(1){
		int cod_op = leer_operacion(discordiador_fd);
		t_tarea* tarea;
		
		switch(cod_op){
			case MENSAJE:
				log_info(logger, "MENSAJE RECIBIDO");
				break;
			case PEDIR_TAREA:
				log_info(logger, "PEDIDO DE TAREA RECIBIDO");
				break;
			case COD_TAREA:
				printf("recibo una tarea");				
				break;
			case -1:
				log_info(logger, "Murio el discordiador");
				return EXIT_FAILURE;
			default:
				log_warning(logger, "Operacion desconocida");
				break;
		}
	}

    close(discordiador_fd);
    log_destroy(logger);
    config_destroy(config);
    config_destroy(config_discordiador);
    free(memoria);
	return EXIT_SUCCESS;

    /*pthread_t hilo_escucha;
	pthread_create(&hilo_escucha, NULL, (void*) escuchar_alos_cliente, NULL);

	pthread_join(hilo_escucha, NULL);*/
}

/*
void escuchar_alos_cliente(){
    int socket_oyente = crear_socket_oyente("127.0.0.2", "4000");
    
    escuchar(socket_oyente, (void*) hola);

}

void hola(){
    printf("Hola");
}
*/


int iniciar_patota(FILE* archivo){
	t_PCB pcb;
	pcb->PID = nuevo_pid();//TODO A discusión de como sacar el pid
	pcb->direccion_tareas = &archivo;

	t_patota patota;
	patota->archivo_de_tareas = archivo;
	patota->pcb = pcb;

	//cargar_en_Mongo(archivo);

	return pcb->PID;
}

int nuevo_pid(){
	int id_patota = 1;
	while(1){
		if(!existe(id_patota)) {
	    	return id_patota;
	    }
	    id_patota++;
	}
}

//Iniciar tripulante: será el encargado de crear la o las estructuras
//administrativas necesarias para que un tripulante pueda ejecutar.

iniciar_tripulante(char* posicion){
	t_TCB tcb;
	tcb->TID = nuevo_tid()
	tcb->coord_x = posicion[0];
	tcb->coord_y = posicion[2];
	//tcb->siguiente_instruccion = ;//Ni idea de que va acá
	tcb->puntero_a_pcb =

	t_tripulante tripulante;
	//tripulante->codigo = ;//Ni idea de que va acá
	tripulante->estado = LLEGADA; //Supongo que se inicializa en LLEGADA por defecto
}
