/*
 ============================================================================
 Name        : Discordiador.c
 Author      : Rey de fuego
 Version     : 1
 Copyright   : Mala yuyu
 Description : El discordiador
 ============================================================================
 */

#define	IP_MI_RAM_HQ config_get_string_value(config, "IP_MI_RAM_HQ");
#define PUERTO_MI_RAM_HQ config_get_string_value(config, "PUERTO_MI_RAM_HQ");
#define	IP_MONGO_STORE config_get_string_value(config, "IP_MONGO_STORE"); // Verificar sintaxis
#define PUERTO_MONGO_STORE config_get_string_value(config, "PUERTO_MONGO_STORE");

#include "discordiador.h"

// Vars globales
t_config* config_discordiador;
t_log* logger_discordiador;
int socket_a_mi_ram_hq;
int socket_a_mongo_store;

int main(int argc, char *argv[]) {
	logger_discordiador = log_create("discordiador.log", "discordiador", true, LOG_LEVEL_INFO);
	config_discordiador = config_create("discordiador.config");

	socket_a_mi_ram_hq = crear_socket_cliente(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
	socket_a_mongo_store = crear_socket_cliente(IP_MONGO_STORE, PUERTO_MONGO_STORE);

	if (socket_a_mi_ram_hq != -1 && socket_a_mongo_store != -1) {

		pthread_t hiloConsola;
		void* p_leer_consola = &leer_consola(); // Verificar sintaxis
		pthread_create(&hiloConsola, NULL, p_leer_consola, NULL);
		pthread_join(hiloConsola, NULL);

	}

	close(socket_a_mi_ram_hq);
	close(socket_a_mongo_store);

	// Liberar punteros a hilos
	free(p_leer_consola);
	
	config_destroy(config);
	log_destroy(logger_discordiador);

	return EXIT_SUCCESS;
}

void leer_consola() {
	char* leido;
	int comando;

	do {

		leido = readline(">>>");

		if (strlen(leido) > 0) {
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

		free(leido);

	} while (comando != EXIT);

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
	enviar_paquete(paquete,socket_a_mi_ram_hq);
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
  	} 
	else {
    	return "Error formateando fecha";
  	}
}
