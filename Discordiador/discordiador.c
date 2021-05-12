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

int main(int argc, char *argv[]){

	logger = log_create("discordiador.log", "discordiador", true, LOG_LEVEL_INFO);

	leer_config();

	pthread_t hiloConsola;
	pthread_create(&hiloConsola, NULL, (void*) leer_consola, NULL);

	pthread_join(hiloConsola, NULL);

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

	free(leido);
}

void leer_config(){
	t_config* cfg = config_create("discordiador.config");

	config.ip_mi_ram_hq = config_get_string_value(cfg, "IP_MI_RAM_HQ");
	config.puerto_mi_ram_hq = config_get_int_value(cfg, "PUERTO_MI_RAM_HQ");
	config.ip_i_mongo_store = config_get_string_value(cfg, "IP_I_MONGO_STORE");
	config.puerto_i_mongo_store = config_get_int_value(cfg, "PUERTO_I_MONGO_STORE");
	config.grado_multitarea = config_get_int_value(cfg, "GRADO_MULTITAREA");
	config.algoritmo = config_get_string_value(cfg, "ALGORITMO");
	config.quantum = config_get_int_value(cfg, "QUANTUM");
	config.duracion_sabotaje = config_get_int_value(cfg, "DURACION_SABOTAJE");
	config.retardo_ciclo_cpu = config_get_int_value(cfg, "RETARDO_CICLO_CPU");

	config_destroy(cfg);
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

	int socket_mi_ram_hq = conectar_a_mi_ram_hq();
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
	close(socket_mi_ram_hq);

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
