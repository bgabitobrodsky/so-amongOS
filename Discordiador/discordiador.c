/*
 ============================================================================
 Name        : Discordiador.c
 Author      : Rey de fuego
 Version     : 1
 Copyright   : Mala yuyu
 Description : El discordiador
 ============================================================================
 */

//TODO en crear_tcb, estamos asignando a las posiciones el ASCII, no el numero (lo cual esta bien?)
//TODO cambiar las LISTAS DE TRIPULANTES por COLAS DE TRIPULANTES
#define	IP_MI_RAM_HQ config_get_string_value(config, "IP_MI_RAM_HQ")
#define PUERTO_MI_RAM_HQ config_get_string_value(config, "PUERTO_MI_RAM_HQ")
#define	IP_I_MONGO_STORE config_get_string_value(config, "IP_I_MONGO_STORE")
#define PUERTO_I_MONGO_STORE config_get_string_value(config, "PUERTO_I_MONGO_STORE")
#define	ALGORITMO config_get_string_value(config, "ALGORITMO")
#define	GRADO_MULTITAREA config_get_string_value(config, "GRADO_MULTITAREA")

#include "discordiador.h"


// Vars globales
t_config* config;
t_log* logger;
int socket_a_mi_ram_hq;
int socket_a_mongo_store;
int pids[100]; //TODO lista
char estado_tripulante[4] = {'N', 'R', 'E', 'B'};

t_list *lista_pids;
t_list *lista_tripulantes_new;
t_list *lista_tripulantes_ready;
t_list *lista_tripulantes_exec;

int main() {
	logger = log_create("discordiador.log", "discordiador", true, LOG_LEVEL_INFO);
	config = config_create("discordiador.config");

	lista_tripulantes_ready = list_create();
	lista_tripulantes_exec = list_create();
	lista_tripulantes_new = list_create();
	lista_pids = list_create();

	for(int i = 0; i<10;i++){
		printf("%i", nuevo_pid());
	}

	socket_a_mi_ram_hq = crear_socket_cliente(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
	//socket_a_mi_ram_hq = crear_socket_cliente("127.0.0.1", "25430");
	socket_a_mongo_store = crear_socket_cliente(IP_I_MONGO_STORE, PUERTO_I_MONGO_STORE);
	//socket_a_mongo_store = crear_socket_cliente("127.0.0.1", "4000");

	if (socket_a_mi_ram_hq != -1 && socket_a_mongo_store != -1) {
		pthread_t hiloConsola;
		pthread_create(&hiloConsola, NULL, (void*)leer_consola, NULL);
		pthread_join(hiloConsola, NULL);
	}

	close(socket_a_mi_ram_hq);
	close(socket_a_mongo_store);
	
	config_destroy(config);
	log_destroy(logger);

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
	t_PCB* pcb = crear_pcb(path);
	t_patota* patota = crear_patota(pcb);

	t_tripulante* aux;
	while (palabras[i+3] != NULL){
		printf("POSICION %d: %s \n", i+1, palabras[i+3]);
		//void* funcion = pedir_funcion()
		aux = iniciar_tripulante(NULL, pcb, i+1, palabras[i+3]); //Le manda a RAM el tripulante

		list_add(lista_tripulantes_new, (void*) aux);
		i++;

	}
	for(int j = i+1; j <= cantidadTripulantes; j++){
		printf("POSICION %d: 0|0 \n", j);
		//void* funcion = pedir_funcion()
		aux = iniciar_tripulante(NULL, pcb, i+1, "0|0"); //Le manda a RAM el tripulante

		list_add(lista_tripulantes_new, (void*) aux);
	}
	free(aux);
}

t_patota* crear_patota(t_PCB* un_pcb){
	t_patota* patota = malloc(sizeof(t_patota));
	patota -> pcb = un_pcb;
	//patota -> archivo_de_tareas //TODO
	return patota;
}

t_PCB* crear_pcb(char* path){
	t_PCB* pcb = malloc(sizeof(t_PCB));
	pcb -> PID = nuevo_pid();
	pcb -> direccion_tareas = (uint32_t) path;
	return pcb;
}

int nuevo_pid(){
	int id_patota = 1;
	while(1){
		if(!esta_en_lista(lista_pids, id_patota)){
			list_add(lista_pids, (void*) id_patota);
			return id_patota;
		}
		id_patota++;
	}

	/*while(1){
		if(!pids_contiene(id_patota)){
			pids[id_patota] = id_patota;
	    	return id_patota;
		}
	    id_patota++;
	}*/
}
/*
int pids_contiene(int valor){
	for(int i = 1; i<100; i++){
		if(pids[i] == valor)
			return 1;
	}
	return 0;
}*/

void iniciar_hilo_tripulante(void* funcion){
	pthread_t hilo1;
	pthread_create(&hilo1, NULL, funcion, NULL);
}

t_TCB* crear_tcb(t_PCB* pcb, int tid, char* posicion){
	t_TCB* tcb = malloc(sizeof(t_TCB));
	tcb -> TID = tid;
	tcb -> estado_tripulante = estado_tripulante[NEW];
	tcb -> coord_x = posicion[0];
	tcb -> coord_y = posicion[2];
	//tcb -> siguiente_instruccion; //TODO
	tcb -> puntero_a_pcb = (uint32_t) pcb;

	return tcb;
}

t_tripulante* crear_tripulante(t_TCB* un_tcb){
	t_tripulante* tripulante = malloc(sizeof(t_tripulante));
	tripulante -> tcb = un_tcb;
	return tripulante;
}

t_tripulante* iniciar_tripulante(void* funcion, t_PCB* pcb, int tid, char* posicion){
	t_TCB* un_tcb = crear_tcb(pcb, tid, posicion);
	t_tripulante* nuestro_tripulante = crear_tripulante(un_tcb);
	iniciar_hilo_tripulante(funcion);
	return nuestro_tripulante;
}

void iniciar_planificacion() {
	printf("iniciarPlanificacion");
	//TODO NO TESTEADO
	if(!strcmp(ALGORITMO, "FIFO")){ //No sé dónde va esto, pero debería ser mientras una variable planificacion_activa = true;
		while(list_size(lista_tripulantes_exec) < atoi(GRADO_MULTITAREA)){
			list_add(lista_tripulantes_ready, list_remove(lista_tripulantes_ready, 0)); //List remove retorna el elemento eliminado
		}
	}
}

void listar_tripulantes() {

	//int i = 0;
    //int argc; tripulantes activos
    //char* fechaHora = fecha_y_hora();
    
    //printf("Estado de la nave: %s\n",fechaHora);
    //for(i; i<argc; i++){
    //    printf("Tripulante: %d\tPatota: %d\tStatus: %s", i, patota, status)
    //}

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

/*int pedir_tarea(int id_tripulante){
	t_paquete* paquete = crear_paquete(PEDIR_TAREA); // BORRAR ESTA COSA BONITA
	agregar_a_paquete(paquete, (void*) id_tripulante, sizeof(int)); // BORRAR ESTA COSA BONITA
	enviar_paquete(paquete,socket_a_mi_ram_hq); // BORRAR ESTA COSA BONITA
	return 1;
}

void realizar_tarea(t_tarea tarea){ // TODO 

}*/

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

int esta_en_lista(t_list* lista, int elemento){ //TODO no se si funca
	bool contiene(void* elemento1){
	return sonIguales(elemento, (int) elemento1);
	}
	int a = list_any_satisfy(lista, contiene);
	return a;
}

int sonIguales(int elemento1, int elemento2){
		return elemento1 == elemento2;
	}

int reconocer_comando(char* str) {

	char** palabras = string_split(str, " ");
	int contador = 0; //contador de palabras

	while (palabras[contador] != NULL) { // la ultima palabra es NULL
		contador++;
	}

	if (comparar_strings(palabras[0],"INICIAR_PATOTA")) {
		if (contador >= 3) {
			free(palabras);
			return INICIAR_PATOTA;
		}
		else {
			printf("Error de parametros: INICIAR_PATOTA <cantidad_de_tripulantes> <path>(<pos1> ... <posn>)\n");
		}
	}

	if (comparar_strings(palabras[0],"LISTAR_TRIPULANTES")) {
		if (contador == 1) {
			free(palabras);
			return LISTAR_TRIPULANTES;
		}
		else {
			printf("Error de parametros: LISTAR_TRIPULANTES\n");
		}
	}

	if (comparar_strings(palabras[0],"EXPULSAR_TRIPULANTE")) {
		if (contador == 2) {
			free(palabras);
			return EXPULSAR_TRIPULANTE;
		}
		else {
			printf("Error de parametros: EXPULSAR_TRIPULANTE <codigo_de_tripulante>\n");
		}
	}

	if (comparar_strings(palabras[0],"INICIAR_PLANIFICACION")) {
		if (contador == 1) {
			free(palabras);
			return INICIAR_PLANIFICACION;
		}
		else {
			printf("Error de parametros: INICIAR_PLANIFICACION\n");
		}
	}

	if (comparar_strings(palabras[0],"PAUSAR_PLANIFICACION")) {
		if (contador == 1) {
			free(palabras);
			return PAUSAR_PLANIFICACION;
		}
		else {
			printf("Error de parametros: PAUSAR_PLANIFICACION\n");
		}
	}

	if (comparar_strings(palabras[0],"OBTENER_BITACORA")) {
		if (contador == 1) {
			free(palabras);
			return OBTENER_BITACORA;
		}
		else {
			printf("Error de parametros: OBTENER_BITACORA <codigo_de_tripulante>\n");
		}
	}

	if (comparar_strings(palabras[0],"HELP")) {
		if (contador == 1) {
			free(palabras);
			return HELP;
		}
		else {
			printf("Error de parametros: HELP\n");
		}
	}

	if (comparar_strings(palabras[0],"EXIT")) {
		if (contador == 1) {
			free(palabras);
			return EXIT;
		}
		else {
			printf("Error de parametros: EXIT\n");
		}
	}

	free(palabras);
	printf("Comando desconocido, escribe HELP para obtener la lista de comandos\n");
	return NO_CONOCIDO;
}


int comparar_strings(char* str, char* str2) {
	return !strncmp(str, str2, strlen(str2));
}

void help_comandos() {
	printf("Lista de comandos:\n");
	printf("- INICIAR_PATOTA <cantidad_de_tripulantes> <path>(<pos1> ... <posn>)\n");
	printf("- INICIAR_PLANIFICACION\n");
	printf("- PAUSAR_PLANIFICACION\n");
	printf("- LISTAR_TRIPULANTES\n");
	printf("- EXPULSAR_TRIPULANTE <codigo_de_tripulante>\n");
	printf("- OBTENER_BITACORA <codigo_de_tripulante>\n");
}