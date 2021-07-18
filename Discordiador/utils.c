/*
 * utils.c
 *
 *  Created on: 8 may. 2021
 *      Author: utnso
 */

#include "utils.h"

int reconocer_comando(char* str) {

	char** palabras = string_split(str, " ");
	int contador = 0; //contador de palabras

	while (palabras[contador] != NULL) { // la ultima palabra es NULL
		contador++;
	}

	if (comparar_strings(palabras[0],"INICIAR_PATOTA")) {
		if (contador >= 3) {
			liberar_puntero_doble(palabras);
			return INICIAR_PATOTA;
		}
		else {
			printf("Error de parametros: INICIAR_PATOTA <cantidad_de_tripulantes> <path>(<pos1> ... <posn>)\n");
		}
	}

	if (comparar_strings(palabras[0],"LISTAR_TRIPULANTES")) {
		if (contador == 1) {
			liberar_puntero_doble(palabras);
			return LISTAR_TRIPULANTES;
		}
		else {
			printf("Error de parametros: LISTAR_TRIPULANTES\n");
		}
	}

	if (comparar_strings(palabras[0],"EXPULSAR_TRIPULANTE")) {
		if (contador == 2) {
			liberar_puntero_doble(palabras);
			return EXPULSAR_TRIPULANTE;
		}
		else {
			printf("Error de parametros: EXPULSAR_TRIPULANTE <codigo_de_tripulante>\n");
		}
	}

	if (comparar_strings(palabras[0],"INICIAR_PLANIFICACION")) {
		if (contador == 1) {
			liberar_puntero_doble(palabras);
			return INICIAR_PLANIFICACION;
		}
		else {
			printf("Error de parametros: INICIAR_PLANIFICACION\n");
		}
	}

	if (comparar_strings(palabras[0],"PAUSAR_PLANIFICACION")) {
		if (contador == 1) {
			liberar_puntero_doble(palabras);
			return PAUSAR_PLANIFICACION;
		}
		else {
			printf("Error de parametros: PAUSAR_PLANIFICACION\n");
		}
	}

	if (comparar_strings(palabras[0],"OBTENER_BITACORA")) {
		if (contador == 2) {
			liberar_puntero_doble(palabras);
			return OBTENER_BITACORA;
		}
		else {
			printf("Error de parametros: OBTENER_BITACORA <codigo_de_tripulante>\n");
		}
	}

	if (comparar_strings(palabras[0],"HELP")) {
		if (contador == 1) {
			liberar_puntero_doble(palabras);
			return HELP;
		}
		else {
			printf("Error de parametros: HELP\n");
		}
	}

	if (comparar_strings(palabras[0],"APAGAR_SISTEMA")) {
		if (contador == 1) {
			liberar_puntero_doble(palabras);
			return APAGAR_SISTEMA;
		}
		else {
			printf("Error de parametros: EXIT\n");
		}
	}

	liberar_puntero_doble(palabras);

	printf("Comando desconocido, escribe HELP para obtener la lista de comandos\n");

	return NO_CONOCIDO;

}

void help_comandos() {

	printf("Lista de comandos:\n");
	printf("- INICIAR_PATOTA <cantidad_de_tripulantes> <path>(<pos1> ... <posn>)\n");
	printf("- INICIAR_PLANIFICACION\n");
	printf("- PAUSAR_PLANIFICACION\n");
	printf("- LISTAR_TRIPULANTES\n");
	printf("- EXPULSAR_TRIPULANTE <codigo_de_tripulante>\n");
	printf("- OBTENER_BITACORA <codigo_de_tripulante>\n");
	printf("- APAGAR_SISTEMA\n");

}

void enviar_archivo_tareas(char* archivo_tareas, int pid, int socket) {

	t_archivo_tareas cont_arc;
	cont_arc.texto = archivo_tareas;
	cont_arc.largo_texto = strlen(archivo_tareas) + 1;
	cont_arc.pid = pid;
	t_buffer* contenido_archivo = serializar_archivo_tareas(cont_arc);
	empaquetar_y_enviar(contenido_archivo, ARCHIVO_TAREAS, socket);

}

void pedir_tarea_a_mi_ram_hq(uint32_t tid, int socket){

	t_buffer* buffer_tripulante = serializar_entero(tid);
	empaquetar_y_enviar(buffer_tripulante, PEDIR_TAREA, socket);

}

void enviar_pid_a_ram(uint32_t pid, int socket){

	t_buffer* pid_buffer = serializar_entero(pid);
	empaquetar_y_enviar(pid_buffer, LISTAR_POR_PID, socket);

}

void enviar_tripulante_a_ram (t_tripulante un_tripulante, int socket){
    t_buffer* buffer_t = serializar_tripulante(un_tripulante);
    empaquetar_y_enviar(buffer_t, RECIBIR_TCB, socket);
}

int esta_tripulante_en_lista(t_list* lista, int elemento){

    bool contains(void* elemento1){
        return (elemento == ((t_tripulante*) elemento1)->TID);
    }
    bool a = list_any_satisfy(lista, contains);
    return a;

}

void* eliminar_tripulante_de_lista(t_list* lista, int elemento){

    bool contains(void* elemento1){
        return (elemento == ((t_tripulante*) elemento1)->TID);
    }

    t_tripulante* aux = list_remove_by_condition(lista, contains);
    return aux;

}

void iniciar_listas() {

	lista_tripulantes_new = list_create();
	lista_tripulantes_exec = list_create();
	lista_pids = list_create();
	lista_patotas = list_create();
	lista_tripulantes = list_create();

}

void iniciar_colas() {

	cola_tripulantes_ready = queue_create();
	cola_tripulantes_block = queue_create();
	cola_tripulantes_block_emergencia = queue_create();
}

void iniciar_semaforos(){

	pthread_mutex_init(&sem_lista_tripulantes, NULL);
	pthread_mutex_init(&sem_lista_exec, NULL);
	pthread_mutex_init(&sem_lista_new, NULL);
	pthread_mutex_init(&sem_cola_ready, NULL);
	pthread_mutex_init(&sem_cola_block, NULL);
	pthread_mutex_init(&sem_cola_block_emergencia, NULL);

}

void liberar_listas() {

	liberar_lista(lista_tripulantes);
	list_destroy(lista_tripulantes_new);
	list_destroy(lista_tripulantes_exec);
	list_destroy(lista_pids);
	liberar_lista(lista_patotas);

}

void liberar_colas() {

	queue_destroy(cola_tripulantes_ready);
	queue_destroy(cola_tripulantes_block);
	queue_destroy(cola_tripulantes_block_emergencia);

}

void liberar_semaforos(){

	pthread_mutex_destroy(&sem_lista_tripulantes);
	pthread_mutex_destroy(&sem_lista_exec);
	pthread_mutex_destroy(&sem_lista_new);
	pthread_mutex_destroy(&sem_cola_ready);
	pthread_mutex_destroy(&sem_cola_block);
	pthread_mutex_destroy(&sem_cola_block_emergencia);

}

void liberar_tripulantes(){

	void terminar(void* un_tripulante){
		((t_tripulante*) un_tripulante)->estado_tripulante = 'F';
	}
	list_iterate(lista_tripulantes, terminar);
}

void liberar_puntero_doble(char** palabras){
	int contador = 0;

	while (palabras[contador] != NULL) {
		contador++;
	}

	for(int i = 0; i<contador; i++){
		free(palabras[i]);
	}
	free(palabras);
}


t_patota* crear_patota(uint32_t un_pid){

    t_patota* patota = malloc(sizeof(t_patota));
    patota ->PID = un_pid;
    return patota;

}

t_tripulante* crear_tripulante(int tid, int x, int y, char estado){

    t_tripulante* tripulante = malloc(sizeof(t_tripulante));
    tripulante->TID = tid;
    tripulante->coord_x = x;
    tripulante->coord_y = y;
    tripulante->estado_tripulante = estado;

    return tripulante;

}

t_tripulante* crear_puntero_tripulante(uint32_t tid, char* posicion){

    t_tripulante* un_tripulante = malloc(sizeof(t_tripulante));
    un_tripulante->TID = tid;
    un_tripulante->estado_tripulante = estado_tripulante[NEW];
    un_tripulante->coord_x = posicion[0] - 48; // equivalencia ascii - numero
    un_tripulante->coord_y = posicion[2] - 48; // equivalencia ascii - numero

    return un_tripulante;
}

int nuevo_pid(){

    int id_patota = 1;
    while(1){
        if(!esta_pid_en_lista(lista_pids, id_patota)){
            list_add(lista_pids, (void*) id_patota);
            return id_patota;
        }
        id_patota++;
    }
}

int esta_pid_en_lista(t_list* lista, int elemento){

    bool contiene(void* elemento1){
        return sonIguales(elemento, (int) elemento1);
    }
    int a = list_any_satisfy(lista, contiene);
    return a;

}
