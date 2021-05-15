/*
 ============================================================================
 Name        : Discordiador.c
 Author      : Rey de fuego
 Version     : 1
 Copyright   : Mala yuyu
 Description : El discordiador
 ============================================================================
 */

#include "discordiador.h"

int socket_mi_ram_hq;

int main(int argc, char *argv[]){

	logger = log_create("discordiador.log", "discordiador", true, LOG_LEVEL_INFO);
	config = config_create("discordiador.config");

	printf("%s", config_get_string_value(config, "IP_MI_RAM_HQ"));
	socket_mi_ram_hq = conectar_a_mi_ram_hq();
	if(socket_mi_ram_hq != -1){
		pthread_t hiloConsola;
		pthread_create(&hiloConsola, NULL, (void*) leer_consola, NULL);

		pthread_join(hiloConsola, NULL);
	}else{
		log_warning(logger, "Error al conectar a MI_RAM_HQ");
	}


	close(socket_mi_ram_hq);
	config_destroy(config);
	log_destroy(logger);
	return EXIT_SUCCESS;
}

void leer_consola(){

	char* leido;
	int comando;
	do{
		leido = readline(">>>");
		if(strlen(leido) > 0){
			comando = reconocer_comando(leido);

			switch(comando){
				case INICIAR_PATOTA:
					iniciar_patota(leido);
					break;

				case INICIAR_PLANIFICACION:
					iniciar_planificacion();
					break;

				case LISTAR_TRIPULANTES:
					listar_tripulantes();
					break;

				case PAUSAR_PLANIFICACION:
					pausar_planificacion();
					break;

				case OBTENER_BITACORA:
					obtener_bitacora(leido);
					break;
				
				case EXPULSAR_TRIPULANTE:
					expulsar_tripulante(leido);
					break;
				
				case HELP:
					help_comandos();
					break;
				
				case NO_CONOCIDO:
					break;
			}
		}		
	}while(comando != EXIT);
	close(socket_mi_ram_hq);
	free(leido);
}

void iniciar_patota(char* leido){
	char** palabras = string_split(leido, " ");

	int cantidadTripulantes = atoi(palabras[1]);
	char* path = palabras[2];

	printf("PATOTA: cantidad de tripulantes %d, url: %s \n", cantidadTripulantes, path);

	// 4 es el offset de lo leído para acceder a las posiciones, (iniciar_patota cant path <posiciones...>)
	/*for(i = 4; i <= cantidadTripulantes + 4; i++){
		printf("Posiciones\n");
		for(int i = 3; i <= argc; i++){
			if(argv[i]==NULL)
				argv[i] = "0|0";
			printf("POSICION %d: %s \n", i-2, argv[i]);
		}
	}*/

}
void iniciar_planificacion(){
	printf("iniciarPlanificacion");
}
void listar_tripulantes(){

	char* mensaje = "holaaa";

	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);


	send(socket_mi_ram_hq, a_enviar, bytes, 0);

	free(a_enviar);	
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}
void pausar_planificacion(){
	printf("pausarPlanificacion");
}
void obtener_bitacora(char* leido){
	printf("obtenerBitacora");
}
void expulsar_tripulante(char* leido){
	printf("expulsarTripulante");
}


void tripulante(){
	int id_tripulante = 2;
	// avisar a miram que va a iniciar

	int tarea = pedir_tarea(id_tripulante);

/*
	while(1){
		realizar_tarea(tarea);
		tarea = pedir_tarea(id_tripulante);
	}
*/
}


int pedir_tarea(int id_tripulante){
	t_paquete* paquete = crear_paquete(PEDIR_TAREA);

}



t_paquete* crear_paquete(op_code codigo){
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigo;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio){
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}
