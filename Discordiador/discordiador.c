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

t_config* config;
t_log* logger;
int socket_mi_ram_hq;

int main() {
	logger = log_create("discordiador.log", "discordiador", true, LOG_LEVEL_INFO);
	config = config_create("discordiador.config");

	socket_mi_ram_hq = conectar_a_mi_ram_hq();

	if (socket_mi_ram_hq != -1) {
		pthread_t hiloConsola;
		pthread_create(&hiloConsola, NULL, (void*) leer_consola, NULL);
		pthread_join(hiloConsola, NULL);
	}

	close(socket_mi_ram_hq);
	config_destroy(config);
	log_destroy(logger);

	return EXIT_SUCCESS;
}

void leer_consola() {
	char* leido;
	int comando;

	do {		
		leido = readline(">>>");

		if(strlen(leido) > 0) {
			comando = reconocer_comando(leido);

			switch (comando) {
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
	} while (comando != EXIT);

	free(leido);
}

void iniciar_patota(char* leido) {
	char** palabras = string_split(leido, " ");
	int cantidadTripulantes = atoi(palabras[1]);
	char* path = palabras[2];

	printf("PATOTA: cantidad de tripulantes %d, url: %s \n", cantidadTripulantes, path);

	int i = 0;

	while (palabras[i+3] != NULL){
		printf("POSICION %d: %s \n", i+1, palabras[i+3]);
		i++;
	}
	for(int j = i+1; j <= cantidadTripulantes; j++){
		printf("POSICION %d: 0|0 \n", j);
	}
}

void iniciar_planificacion() {
	printf("iniciarPlanificacion");
}

void listar_tripulantes() {

	tripulante();

	/*
	int i = 0;
    int argc; // No sÃ© como serÃ­a, pero vamos a recibir a los tripulantes activos de alguna manera
    char* fechaHora = fecha_y_hora();
    
    printf("Estado de la nave: %s\n",fechaHora);
    for(i; i<argc; i++){
        printf("Tripulante: %d\tPatota: %d\tStatus: %s", i, patota, status)
    }
	*/
	printf("listarTripulantes");

}

void pausar_planificacion() {
	
}

void obtener_bitacora(char* leido) {
	printf("obtenerBitacora");
}

void expulsar_tripulante(char* leido) {
	printf("expulsarTripulante");
}


void tripulante() {
	int id_tripulante = 2;
	int id_patota;
	int cord_x;
	int cord_y;
	char* status;
	// avisar a miram que va a iniciar

	int tarea = pedir_tarea(id_tripulante);
	printf("%d",tarea);
/*
	while(1){
		realizar_tarea(tarea);
		tarea = pedir_tarea(id_tripulante);
	}
	
*/
	//informar ram movimiento



}




int pedir_tarea(int id_tripulante){
	t_paquete* paquete = crear_paquete(PEDIR_TAREA);
	agregar_a_paquete(paquete, (void*) id_tripulante, sizeof(int));
	enviar_paquete(paquete,socket_mi_ram_hq);
	return 1;
}


void realizar_tarea(t_tarea tarea){ // TODO 

}

void instanciar_tripulante(char* str_posicion){ // TODO
	int cord_x = atoi(str_posicion[0]);
	int cord_y = atoi(str_posicion[2]);
}

char* fecha_y_hora() { // Creo que las commons ya tienen una funcion que hace esto
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
