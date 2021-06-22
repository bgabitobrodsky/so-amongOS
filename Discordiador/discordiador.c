/*
 ============================================================================
 Name        : Discordiador.c
 Author      : Rey de fuego
 Version     : 1
 Copyright   : Mala yuyu
 Description : El discordiador
 ============================================================================
 */

#define IP_MI_RAM_HQ config_get_string_value(config, "IP_MI_RAM_HQ")
#define PUERTO_MI_RAM_HQ config_get_string_value(config, "PUERTO_MI_RAM_HQ")
#define IP_I_MONGO_STORE config_get_string_value(config, "IP_I_MONGO_STORE")
#define PUERTO_I_MONGO_STORE config_get_string_value(config, "PUERTO_I_MONGO_STORE")
#define ALGORITMO config_get_string_value(config, "ALGORITMO")
#define GRADO_MULTITAREA config_get_string_value(config, "GRADO_MULTITAREA")
#define QUANTUM config_get_int_value(config, "QUANTUM")
#define RETARDO_CICLO_CPU config_get_int_value(config, "RETARDO_CICLO_CPU")
#define DURACION_SABOTAJE config_get_int_value(config, "DURACION_SABOTAJE")
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#include "discordiador.h"

// Variables globales
t_config* config;
t_log* logger;
int socket_a_mi_ram_hq;
int socket_a_mongo_store;

// Listas, colas, semaforos
t_list* lista_pids;
t_list* lista_patotas;
t_list* lista_tripulantes;
t_list* lista_tripulantes_new;
t_list* lista_tripulantes_exec;
t_list* lista_tripulantes_block;

t_queue* cola_tripulantes_ready;

pthread_mutex_t sem_lista_new;
pthread_mutex_t sem_lista_exec;
pthread_mutex_t sem_lista_block;
pthread_mutex_t sem_cola_ready;

// Variables de discordiador
char estado_tripulante[5] = {'N', 'R', 'E', 'B', 'F'};
int planificacion_activa = 0;
int sistema_activo = 1;
int testeo = TEST_SERIALIZACION;

enum {
    GENERAR_OXIGENO, CONSUMIR_OXIGENO, GENERAR_COMIDA, CONSUMIR_COMIDA, GENERAR_BASURA, DESCARTAR_BASURA, OTRA_TAREA
};

void actualizar_tripulante(t_tripulante* un_tripulante, int socket){
    t_buffer* b_tripulante = serializar_tripulante(*un_tripulante);
    empaquetar_y_enviar(b_tripulante, ACTUALIZAR, socket);
}

int main() {
	if(testeo != DISCORDIADOR)
		correr_tests(testeo);
	else {

    FILE* reiniciar_logger = fopen("discordiador.log", "w");
    fclose(reiniciar_logger);

    logger = log_create("discordiador.log", "discordiador", true, LOG_LEVEL_TRACE);
    config = config_create("discordiador.config");

    iniciar_listas();
    iniciar_colas();
    iniciar_semaforos();

    socket_a_mi_ram_hq = crear_socket_cliente(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
    socket_a_mongo_store = crear_socket_cliente(IP_I_MONGO_STORE, PUERTO_I_MONGO_STORE);

//    test_iniciar_patota();

    if (socket_a_mi_ram_hq != -1 && socket_a_mongo_store != -1) {

        leer_consola();
        pthread_t hiloConsola;
        pthread_create(&hiloConsola, NULL, (void*)leer_consola, NULL);
        pthread_detach(hiloConsola);

    }

    while(sistema_activo){
        usleep(1);
    }

    close(socket_a_mi_ram_hq);
    close(socket_a_mongo_store);

    config_destroy(config);
    log_destroy(logger);

    return EXIT_SUCCESS;

}
}

int correr_tests(int enumerado) {
	switch(enumerado) {
	case TEST_SERIALIZACION:
		return CUmain_serializacion();
		break;

	case TEST_ENVIO_Y_RECEPCION:
		return CUmain_envio_y_recepcion();
			break;

	case TEST_DISCORDIADOR:
		break;
	}
	return 1;
}

void iniciar_patota(char* leido) {

    char** palabras = string_split(leido, " ");
    int cantidadTripulantes = atoi(palabras[1]);
    char* path = palabras[2];

    printf("PATOTA: cantidad de tripulantes %d, url: %s \n", cantidadTripulantes, path);

    int i = 0;

    t_patota* patota = malloc(sizeof(t_patota));
    patota->PID = nuevo_pid();
    list_add(lista_patotas, patota);

    char* archivo_tareas = leer_archivo_entero(path);

    if (archivo_tareas != NULL){
        enviar_archivo_tareas(archivo_tareas, patota->PID, socket_a_mi_ram_hq);
    }

    t_tripulante* t_aux;

    while (palabras[i+3] != NULL){
        printf("POSICION %d: %s \n", i+1, palabras[i+3]);
        t_aux = crear_puntero_tripulante(((patota->PID)*10000) + i+1, palabras[i+3]);
        enviar_tripulante_a_ram(*t_aux, socket_a_mi_ram_hq);

        list_add(lista_tripulantes, t_aux);
        monitor_lista_dos_parametros(sem_lista_new, (void*) list_add, lista_tripulantes_new, t_aux);

        log_trace(logger, "Tripulante creado:\n tid: %i, estado: %c, pos: %i %i\n", (int)t_aux->TID, (char) t_aux->estado_tripulante, (int) t_aux->coord_x, (int) t_aux->coord_y);
        crear_hilo_tripulante(t_aux);
        // free(aux); // no liberar
        i++;
    }

    for(int j = i+1; j <= cantidadTripulantes; j++){
        printf("POSICION %d: 0|0\n", j);
        t_aux = crear_puntero_tripulante(((patota->PID)*10000) + j, "0|0");
        enviar_tripulante_a_ram(*t_aux, socket_a_mi_ram_hq);

        list_add(lista_tripulantes, t_aux);
        monitor_lista_dos_parametros(sem_lista_new, (void*) list_add, lista_tripulantes_new, t_aux);

        log_trace(logger, "Tripulante creado:\n tid: %i, estado: %c, pos: %i %i\n", (int)t_aux->TID, (char) t_aux->estado_tripulante, (int) t_aux->coord_x, (int) t_aux->coord_y);
        crear_hilo_tripulante(t_aux);
        // free(aux); // no liberar
    }

    // free(patota); // no liberar
    liberar_puntero_doble(palabras);

}


void iniciar_planificacion() {

    planificacion_activa = 1;

    // TODO hacer un hilo?
    // while(planificacion_activa){
        while(list_size(lista_tripulantes_exec) < atoi(GRADO_MULTITAREA) && !queue_is_empty(cola_tripulantes_ready)){
            log_debug(logger, "\nPlanificando\n");
            // t_tripulante* aux_tripulante = monitor_cola_pop(sem_cola_ready, cola_tripulantes_ready);
            // aux_tripulante->estado_tripulante = estado_tripulante[EXEC];
            // monitor_lista_dos_parametros(sem_lista_exec, (void*)list_add, lista_tripulantes_exec, aux_tripulante);

            if(comparar_strings(ALGORITMO, "FIFO")){
                t_tripulante* aux_tripulante = monitor_cola_pop(sem_cola_ready, cola_tripulantes_ready);
                aux_tripulante->estado_tripulante = estado_tripulante[EXEC];

                monitor_lista_dos_parametros(sem_lista_exec, (void*)list_add, lista_tripulantes_exec, aux_tripulante);
                // mandar_a_trabajar(aux_tripulante);
                // free(aux_tripulante); // cuando termina de trabajar lo libero che?
            }/*
            else if(comparar_strings(ALGORITMO, "RR")){
                sleep(MIN(tiempo_restante, QUANTUM)*RETARDO_CICLO_CPU);
                // actualizar la duracion de la tarea
                monitor_lista_dos_parametros(sem_lista_exec, (void*)eliminar_tripulante_de_lista, lista_tripulantes_exec, aux_tripulante);
                aux_tripulante->estado_tripulante = estado_tripulante[READY];
                monitor_cola_push(sem_cola_ready, cola_tripulantes_ready, aux_tripulante);
                free(aux_tripulante);
            }*/
        }
    // }
}

void listar_tripulantes() {

    char* fechaHora = fecha_y_hora();

    printf(">>> Estado de la nave: %s\n\n", fechaHora);

    t_patota* aux_p;
    t_tripulante* aux_t;
    t_list* lista_tripulantes_de_una_patota;

    int i,j;

    for(i = 0; i < list_size(lista_patotas); i++){
        aux_p = list_get(lista_patotas, i);

        lista_tripulantes_de_una_patota = lista_tripulantes_patota(aux_p->PID);

        for(j = 0; j < list_size(lista_tripulantes_de_una_patota); j++){
            aux_t = list_get(lista_tripulantes_de_una_patota, j);
            printf("    Tripulante: %d \t   Patota: %d \t Status: %c\n", aux_t->TID, aux_t->TID/10000, aux_t->estado_tripulante);
        }
    }
}

t_list* lista_tripulantes_patota(uint32_t pid){

    t_list* lista_tripulantes_patota = list_create();

    enviar_pid_a_ram(pid, socket_a_mi_ram_hq);

    t_estructura* respuesta = recepcion_y_deserializacion(socket_a_mi_ram_hq);

    while(respuesta->codigo_operacion != EXITO){
        list_add(lista_tripulantes_patota, respuesta->tcb);
        //free(respuesta->tcb);
        //free(respuesta);
        respuesta = recepcion_y_deserializacion(socket_a_mi_ram_hq);
    }
    if(respuesta->codigo_operacion == FALLO){
        log_info(logger, "Error al pedir los tripulantes para listar.\n");
        log_info(logger, "Codigo de error: FALLO\n");
    }

    bool ordenar_por_tid(void* un_elemento, void* otro_elemento){
         return ((((t_tripulante*) un_elemento)->TID) < (((t_tripulante*) otro_elemento)->TID));
    }

    list_sort(lista_tripulantes_patota, ordenar_por_tid);

    return lista_tripulantes_patota;

}

// TODO: mejorable?
void crear_hilo_tripulante(t_tripulante* un_tripulante){

    t_tripulante* tripulante_real = un_tripulante;
    pthread_t hilo_tripulante;
    pthread_create(&hilo_tripulante, NULL, (void*)tripulante, (void*)tripulante_real);
    pthread_detach(hilo_tripulante);

}

void tripulante(t_tripulante* un_tripulante){

    //int st_ram = crear_socket_cliente(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
    //int st_mongo = crear_socket_cliente(IP_I_MONGO_STORE, PUERTO_I_MONGO_STORE);

    usleep(500);
    log_trace(logger, "Iniciando tripulante: %i\n", un_tripulante->TID);
    iniciar_tripulante(un_tripulante);

    while(un_tripulante->estado_tripulante != estado_tripulante[EXIT]){
        while(un_tripulante->estado_tripulante == estado_tripulante[EXEC]){
            // TODO: actualizar socket
            actualizar_tripulante(un_tripulante, socket_a_mi_ram_hq);
            sleep(1);
            realizar_tarea(un_tripulante);
            pedir_tarea_a_mi_ram_hq(un_tripulante->TID, socket_a_mi_ram_hq);
            t_estructura* tarea = recepcion_y_deserializacion(socket_a_mi_ram_hq);
            if(tarea->codigo_operacion == TAREA){
                un_tripulante->tarea = *tarea->tarea;
            }
            else if(tarea->codigo_operacion == FALLO){
                un_tripulante->estado_tripulante = estado_tripulante[EXIT];
                actualizar_tripulante(un_tripulante, socket_a_mi_ram_hq);
            }
            // pedir_otra_tarea();
        }
    };

}

void iniciar_tripulante(t_tripulante* un_tripulante){
    // Se verifica que se creo bien y se enlista

    if (un_tripulante->estado_tripulante == estado_tripulante[NEW]){
        enlistarse(un_tripulante);
    }
    else {
        log_error(logger, "\nPor un motivo desconocido, el tripulante se ha creado en un estado distinto a NEW. \n");
    }
}

void enlistarse(t_tripulante* un_tripulante){
    // Se le asigna la tarea y se lo pasa a READY
    pedir_tarea_a_mi_ram_hq(un_tripulante->TID, socket_a_mi_ram_hq);

    t_estructura* respuesta = recepcion_y_deserializacion(socket_a_mi_ram_hq);

    if(respuesta->codigo_operacion == TAREA){
        un_tripulante->tarea = *(respuesta->tarea);
        // log_trace(logger, "\nYa tengo trabajo. TID: %i", un_tripulante->TID);
        // log_trace(logger, "\nTarea: %s", un_tripulante->tarea.nombre);
        // log_trace(logger, "\nDuracion: %i", un_tripulante->tarea.duracion);
        // log_trace(logger, "\nCoord x: %i", un_tripulante->tarea.coord_x);
        // log_trace(logger, "\nCoord y: %i", un_tripulante->tarea.coord_x);

    }
    else if (respuesta->codigo_operacion == FALLO){
        log_error(logger, "\nNo se recibio ninguna tarea.\n Codigo de error: FALLO\n");
    }
    else{
        log_error(logger, "\nNo se recibio ninguna tarea.\n Error desconocido\n");
    }

    un_tripulante->estado_tripulante = estado_tripulante[READY];
    // TODO: socket
 actualizar_tripulante(un_tripulante, socket_a_mi_ram_hq);
    log_debug(logger, "\nEstado cambiado a READY\n");

    monitor_lista_dos_parametros(sem_lista_new, (void*)eliminar_tripulante_de_lista, lista_tripulantes_new, (void*) un_tripulante->TID);
    monitor_cola_push(sem_cola_ready, cola_tripulantes_ready, un_tripulante);

}

void realizar_tarea(t_tripulante* un_tripulante){

    // TODO TODO SE DEBE PODER INTERRUMPIR EN CASO DE QUERER DETENER LAS COSAS
    int codigo_tarea = identificar_tarea(un_tripulante->tarea.nombre);
    log_debug(logger, "codigo de tarea: %i", codigo_tarea);

    switch(codigo_tarea){

        case GENERAR_OXIGENO:
            llegar_a_destino(un_tripulante);
            break;

        case CONSUMIR_OXIGENO:
            llegar_a_destino(un_tripulante);
            break;

        case GENERAR_COMIDA:
            llegar_a_destino(un_tripulante);
            break;

        case CONSUMIR_COMIDA:
            llegar_a_destino(un_tripulante);
            break;

        case GENERAR_BASURA:
            llegar_a_destino(un_tripulante);
            break;

        case DESCARTAR_BASURA:
            llegar_a_destino(un_tripulante);
            break;

        default:
            llegar_a_destino(un_tripulante);
            no_me_despierten_estoy_trabajando(un_tripulante);
            log_debug(logger, "Tarea realizada! %s\n", un_tripulante->tarea.nombre);
            break;
    }
}


int identificar_tarea(char* nombre_recibido){

    if(comparar_strings(nombre_recibido, "GENERAR_OXIGENO")){
        return GENERAR_OXIGENO;
    }
    else if(comparar_strings(nombre_recibido, "CONSUMIR_OXIGENO")){
        return CONSUMIR_OXIGENO;
    }
    else if(comparar_strings(nombre_recibido, "GENERAR_COMIDA")){
        return GENERAR_COMIDA;
    }
    else if(comparar_strings(nombre_recibido, "CONSUMIR_COMIDA")){
        return CONSUMIR_COMIDA;
    }
    else if(comparar_strings(nombre_recibido, "GENERAR_BASURA")){
        return GENERAR_BASURA;
    }
    else if(comparar_strings(nombre_recibido, "DESCARTAR_BASURA")){
        return DESCARTAR_BASURA;
    }

    return OTRA_TAREA;

}

void llegar_a_destino(t_tripulante* un_tripulante){

    uint32_t origen_x = un_tripulante->coord_x;
    uint32_t origen_y = un_tripulante->coord_y;
    uint32_t destino_x = un_tripulante->tarea.coord_x;
    uint32_t destino_y = un_tripulante->tarea.coord_y;

    uint32_t distancia_x = abs(destino_x - origen_x);
    uint32_t distancia_y = abs(destino_y - origen_y);

    // TODO: esto es, si no esta detenido todo
    // while(){
        log_debug(logger, "Empezando a caminar");
        log_debug(logger, "Distancia x: %i", distancia_x);
        log_debug(logger, "Distancia y: %i", distancia_y);
        // Se mueve primero en la x y luego en la y
        while(distancia_x != 0){

            if(destino_x > origen_x){
                sleep(RETARDO_CICLO_CPU);
                un_tripulante->coord_x++; // el eje positivo es hacia la derecha?
                // TODO: socket
                actualizar_tripulante(un_tripulante, socket_a_mi_ram_hq);
                log_trace(logger, "Me muevo hacia la derecha.\n");
                distancia_x--;
            }
            else if (destino_x < origen_x){
                sleep(RETARDO_CICLO_CPU);
                un_tripulante->coord_x--;
                // TODO: socket
                actualizar_tripulante(un_tripulante, socket_a_mi_ram_hq);
                log_trace(logger, "Me muevo hacia la izquierda.\n");
                distancia_x--;
            }
        }

        while(distancia_y != 0){
            if(destino_y > origen_y){
                sleep(RETARDO_CICLO_CPU);
                un_tripulante->coord_y++; // el eje positivo es hacia arriba?
                // TODO: socket
                actualizar_tripulante(un_tripulante, socket_a_mi_ram_hq);
                log_trace(logger, "Me muevo hacia arriba.\n");
                distancia_y--;
            }
            else if (destino_x < origen_x){
                sleep(RETARDO_CICLO_CPU);
                un_tripulante->coord_y--;
                // TODO: socket
                actualizar_tripulante(un_tripulante, socket_a_mi_ram_hq);
                log_trace(logger, "Me muevo hacia abajo.\n");
                distancia_y--;
            }
        }
    // }

}

void no_me_despierten_estoy_trabajando(t_tripulante* un_tripulante){
    // El tripulante realiza sus tareas.
    // while(un_tripulante->tarea->duracion > 0 && NO ESTA TODO PARADO){
    while(un_tripulante->tarea.duracion > 0 ){
        sleep(RETARDO_CICLO_CPU);
        un_tripulante->tarea.duracion--;
    }
}

void leer_consola() {

    char* leido;
    int comando;

    do {

        leido = readline(">>> ");

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

                case APAGAR_SISTEMA:
                    sistema_activo = 0;
                    exit(1);
                    break;

                case NO_CONOCIDO:
                    break;
            }
        }

        free(leido);

    } while (comando != EXIT);

    sistema_activo = 0;

}

void pausar_planificacion() {

    printf("Pausar Planificacion\n");
    planificacion_activa = 0;

}

void obtener_bitacora(char* leido) {

    printf("Obtener Bitacora\n");

}

// TODO: como finalizo el hilo?
void expulsar_tripulante(char* leido) {

    printf("Expulsar Tripulante\n");
    char** palabras = string_split(leido, " ");
    int tid_tripulante_a_expulsar = atoi(palabras[1]);

    t_buffer* b_tid = serializar_entero(tid_tripulante_a_expulsar);
    empaquetar_y_enviar(b_tid, T_SIGKILL, socket_a_mi_ram_hq);

    t_estructura* respuesta = recepcion_y_deserializacion(socket_a_mi_ram_hq);

    if(respuesta->codigo_operacion == EXITO){
        if(esta_tripulante_en_lista(lista_tripulantes, tid_tripulante_a_expulsar)){
            t_tripulante* t_aux = eliminar_tripulante_de_lista(lista_tripulantes, tid_tripulante_a_expulsar);
            log_info(logger, "Tripulante expulsado, TID: %d\n", tid_tripulante_a_expulsar);
            log_info(logger, "Lugar del deceso: %i|%i\n", t_aux->coord_x, t_aux->coord_y);
            free(t_aux);
        }
        else{
            log_info(logger, "Dicho tripulante no existe en Discordiador.\n");
        }
    }
    else if (respuesta->codigo_operacion == FALLO){
        log_info(logger, "No existe el tripulante. TID: %d\n", tid_tripulante_a_expulsar);
    }
    else{
        log_info(logger, "Error desconocido.\n");
    }

    liberar_puntero_doble(palabras);

}
