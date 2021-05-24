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

    char* tamanio_memoria = config_get_string_value(config, "TAMANIO_MEMORIA");
    char* memoria = malloc(atoi(tamanio_memoria));
    free(tamanio_memoria);
	
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
int nuevo_pid(){
	int id_patota = 1;
	while(1){
		if(!existe(id_patota)) {
	    	return id_patota;
	    }
	    id_patota++;
	}
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



/*
void escuchar_alos_cliente(){
    int socket_oyente = crear_socket_oyente("127.0.0.2", "4000");
    
    escuchar(socket_oyente, (void*) hola);

}

void hola(){
    printf("Hola");
}
*/
