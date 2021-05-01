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
	log_info(logger,"Hola");

	leer_config();


	pthread_t hiloConsola;
	pthread_create(&hiloConsola, NULL, (void*) leer_consola, NULL);

	pthread_join(hiloConsola, NULL);

	log_destroy(logger);
	return EXIT_SUCCESS;
}

void leer_consola(){

	char* leido;
	char** comando;

	while(1){
		leido = readline(">>>");
		comando= string_split(leido, " ");
		int cont = 0;

			while(1){
				if(comando[cont] != NULL)
					cont++;
				else
					break;
			}

		if(!strncmp(comando[0], "INICIAR_PATOTA", 14)){
			iniciar_patota(cont, comando);
		}else if(!strncmp(comando[0], "EXIT", 4)){
			break;
		}else{
			log_warning(logger, "Comando '%s' no conocido", comando[0]);
		}

	}
	free(leido);
	free(comando);

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


	log_info(logger, "IP MI RAM HQ: %s", config.ip_mi_ram_hq);

	config_destroy(cfg);
}

void iniciar_patota(int argc, char* argv[]){
	printf("PATOTA: cantidad de tripulantes %s, url: %s \n", argv[1], argv[2]);
	if(argc > 2){
		printf("Posiciones\n");
		for(int i = 3; i <= argc; i++){
			if(argv[i]==NULL)
				argv[i] = "0|0";
			printf("POSICION %d: %s \n", i-2, argv[i]);
		}
	}
}
