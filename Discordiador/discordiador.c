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

config_t config;
t_log* logger;

int main(int argc, char *argv[]) {

	logger = log_create("discordiador.log", "discordiador", true, LOG_LEVEL_INFO);

	leerConfig();

	pthread_t hiloConsola;
	pthread_create(&hiloConsola, NULL, (void*) leerConsola, NULL);

	pthread_join(hiloConsola, NULL);

	log_destroy(logger);
	return EXIT_SUCCESS;
}

void leerConsola(){

	char* leido;
	int comando;
	do{
		leido = readline(">>>");
		if(strlen(leido) > 0){
			comando = reconocerComando(leido);

			switch(comando){
				case INICIAR_PATOTA:
					iniciarPatota(leido);
					break;

				case INICIAR_PLANIFICACION:
					iniciarPlanificacion();
					break;

				case LISTAR_TRIPULANTES:
					listarTripulantes();
					break;

				case PAUSAR_PLANIFICACION:
					pausarPlanificacion();
					break;

				case OBTENER_BITACORA:
					obtenerBitacora(leido);
					break;
				
				case EXPULSAR_TRIPULANTE:
					expulsarTripulante(leido);
					break;
				
				case HELP:
					helpComandos();
					break;
				
				case NO_CONOCIDO:
					printf("Comando desconocido, escribe HELP para obtener la lista de comandos\n");
					break;

				default:
					break;
			}
		}		
	}while(comando != EXIT);

	free(leido);
}

void leerConfig() {
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

void iniciarPatota(char* leido){
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
void iniciarPlanificacion(){
	printf("iniciarPlanificacion");
}
void listarTripulantes(){
	printf("listarTripulantes soy el cambio de juli");
}
void pausarPlanificacion(){
	printf("pausarPlanificacion");
}
void obtenerBitacora(char* leido){
	printf("obtenerBitacora");
}
void expulsarTripulante(char* leido){
	printf("expulsarTripulante");
}
