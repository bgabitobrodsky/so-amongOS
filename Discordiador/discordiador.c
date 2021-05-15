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

	close(socket_mi_ram_hq);

	/*
	int i = 0;
    int argc; // No sÃ© como serÃ­a, pero vamos a recibir a los tripulantes activos de alguna manera
    char* fechaHora = fecha_y_hora();
    
    printf("Estado de la nave: %s\n",fechaHora);
    for(i; i<argc; i++){
        printf("Tripulante: %d\tPatota: %d\tStatus: %s", i, patota, status)
    }
	*/

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


char* fecha_y_hora() { 
  time_t tiempo = time(NULL);
  struct tm tiempoLocal = *localtime(&tiempo); // Tiempo actual
  static char fecha_Hora[70]; // El lugar en donde se pondrÃ¡ la fecha y hora formateadas
  char *formato = "%d-%m-%Y %H:%M:%S";  // El formato. Mira mÃ¡s en https://en.cppreference.com/w/c/chrono/strftime
  int bytesEscritos = strftime(fecha_Hora, sizeof fecha_Hora, formato, &tiempoLocal);  // Intentar formatear
  if (bytesEscritos != 0) { // Si no hay error, los bytesEscritos no son 0
   return fecha_Hora;
  } else {
    return "Error formateando fecha";
  }
}