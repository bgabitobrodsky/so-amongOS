/*
 ============================================================================
 Name        : Discordiador.c
 Author      : Rey de fuego
 Version     : 1
 Copyright   : Mala yuyu
 Description : El discordiador
 ============================================================================
 */

// TODO: Destruir listas
// TODO: Revisar como actualiza rr en ram

#define IP_MI_RAM_HQ config_get_string_value(config, "IP_MI_RAM_HQ")
#define PUERTO_MI_RAM_HQ config_get_string_value(config, "PUERTO_MI_RAM_HQ")
#define IP_I_MONGO_STORE config_get_string_value(config, "IP_I_MONGO_STORE")
#define PUERTO_I_MONGO_STORE config_get_string_value(config, "PUERTO_I_MONGO_STORE")
#define ALGORITMO config_get_string_value(config, "ALGORITMO")
#define GRADO_MULTITAREA config_get_int_value(config, "GRADO_MULTITAREA")
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

t_queue* cola_tripulantes_ready;
t_queue* cola_tripulantes_block;
t_queue* cola_tripulantes_block_emergencia;

pthread_mutex_t sem_lista_tripulantes;
pthread_mutex_t sem_lista_new;
pthread_mutex_t sem_cola_ready;
pthread_mutex_t sem_lista_exec;
pthread_mutex_t sem_cola_block;
pthread_mutex_t sem_cola_block_emergencia;

// Variables de discordiador
char estado_tripulante[6] = {'N', 'R', 'E', 'B', 'F', 'V'};
int estamos_en_peligro = 0; // variable de sabotaje
int planificacion_activa = 0;
int sistema_activo = 1;
int testeo = DISCORDIADOR;

void notificar_fin_de_tarea(t_tripulante* un_tripulante, int socket_mongo){
	t_buffer* trip_buffer = serializar_tripulante(*un_tripulante);
	empaquetar_y_enviar(trip_buffer, FIN_TAREA, socket_mongo);
	log_debug(logger, "Notifico a Mongo que termino una tarea");
}

void notificar_inicio_de_tarea(t_tripulante* un_tripulante, int socket_mongo){
	t_buffer* trip_buffer = serializar_tripulante(*un_tripulante);
	empaquetar_y_enviar(trip_buffer, INICIO_TAREA, socket_mongo);
	log_debug(logger, "Notifico a Mongo que inicio una tarea");
}

void notificar_movimiento(t_tripulante* un_tripulante, int socket_mongo){
	t_buffer* trip_buffer = serializar_tripulante(*un_tripulante);
	empaquetar_y_enviar(trip_buffer, MOVIMIENTO, socket_mongo);
	log_debug(logger, "Notifico a Mongo que camino");
}

void notificar_inicio_sabotaje(t_tripulante* un_tripulante, int socket_mongo){
	t_buffer* trip_buffer = serializar_tripulante(*un_tripulante);
	empaquetar_y_enviar(trip_buffer, CORRE_SABOTAJE, socket_mongo);
	log_debug(logger, "Notifico a mongo que corro por mi vida");
}

void notificar_fin_sabotaje(t_tripulante* un_tripulante, int socket_mongo){
	t_buffer* trip_buffer = serializar_tripulante(*un_tripulante);
	empaquetar_y_enviar(trip_buffer, RESUELVE_SABOTAJE, socket_mongo);
	log_debug(logger, "Notifico a Mongo que soy un heroe");
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

    iniciar_patota("INICIAR_PATOTA 2 Random.ims 9|9");

    // iniciar_patota("INICIAR_PATOTA 1 Prueba.ims 1|1");
    // iniciar_patota("INICIAR_PATOTA 2 Oxigeno.ims 1|1");
    iniciar_planificacion();

    // sleep(1);
    // peligro("9|9", socket_a_mi_ram_hq);

    if (socket_a_mi_ram_hq != -1 && socket_a_mongo_store != -1) {

        pthread_t hiloConsola;
        pthread_create(&hiloConsola, NULL, (void*)leer_consola, NULL);
        pthread_detach(hiloConsola);

    }

    while(sistema_activo){
    	sleep(1);
    	// guardian_sabotaje(socket_a_mi_ram_hq, socket_a_mongo_store);
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

int iniciar_patota(char* leido) {

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
    t_estructura* respuesta = recepcion_y_deserializacion(socket_a_mi_ram_hq);

    if(respuesta->codigo_operacion == EXITO){
        log_info(logger, "Cargado el archivo en memmoria ");
        //free(respuesta);
    } else{
        log_warning(logger, "No hay memoria para el archivo");
        //free(respuesta);
        return 0;
    }

    t_tripulante* t_aux;

    while (palabras[i+3] != NULL){
        printf("POSICION %d: %s \n", i+1, palabras[i+3]);
        t_aux = crear_puntero_tripulante(((patota->PID)*10000) + i+1, palabras[i+3]);
        enviar_tripulante_a_ram(*t_aux, socket_a_mi_ram_hq);

        if(!verificacion_tcb(socket_a_mi_ram_hq)){
        	return 0;
        }

        log_debug(logger, "enviado tripulante %i a ram", t_aux->TID);
        list_add(lista_tripulantes, t_aux);
        monitor_lista(sem_lista_new, (void*) list_add, lista_tripulantes_new, t_aux);

        log_trace(logger, "Tripulante creado:\n tid: %i, estado: %c, pos: %i %i\n", (int)t_aux->TID, (char) t_aux->estado_tripulante, (int) t_aux->coord_x, (int) t_aux->coord_y);
        crear_hilo_tripulante(t_aux);
        // free(aux); // no liberar
        i++;
    }

    for(int j = i+1; j <= cantidadTripulantes; j++){
        printf("POSICION %d: 0|0\n", j);
        t_aux = crear_puntero_tripulante(((patota->PID)*10000) + j, "0|0");
        enviar_tripulante_a_ram(*t_aux, socket_a_mi_ram_hq);

        if(!verificacion_tcb(socket_a_mi_ram_hq)){
        	return 0;
        }

        log_debug(logger, "enviado tripulante %i a ram", t_aux->TID);

        list_add(lista_tripulantes, t_aux);
        monitor_lista(sem_lista_new, (void*) list_add, lista_tripulantes_new, t_aux);

        log_trace(logger, "Tripulante creado:\n tid: %i, estado: %c, pos: %i %i\n", (int)t_aux->TID, (char) t_aux->estado_tripulante, (int) t_aux->coord_x, (int) t_aux->coord_y);
        crear_hilo_tripulante(t_aux);
    }

    log_debug(logger, "enviados tripulantes a ram");

    liberar_puntero_doble(palabras);

    return 1;
}


void iniciar_planificacion() {

    planificacion_activa = 1;
    log_debug(logger, "\nTripulantes en NEW: %i\n", list_size(lista_tripulantes_new));
    log_debug(logger, "\nTripulantes en READY: %i\n", queue_size(cola_tripulantes_ready));
    log_debug(logger, "\nTripulantes en EXEC: %i\n", list_size(lista_tripulantes_exec));
    log_debug(logger, "\nTripulantes en BLOQ I/O: %i\n", queue_size(cola_tripulantes_block));
    log_debug(logger, "\nTripulantes en BLOQ EMERGENCIA: %i\n", queue_size(cola_tripulantes_block));
    log_debug(logger, "\nTripulantes VIVOS: %i\n", list_size(lista_tripulantes));

    pthread_t t_planificador;
    pthread_create(&t_planificador, NULL, (void*) planificador, NULL);
    pthread_detach(t_planificador);

}

void planificador(){
    log_info(logger, "Planificando");
    while(planificacion_activa){
    	sleep(1);
        while(list_size(lista_tripulantes_exec) < GRADO_MULTITAREA && !queue_is_empty(cola_tripulantes_ready)){

            if(comparar_strings(ALGORITMO, "FIFO")){
                t_tripulante* aux_tripulante = monitor_cola_pop(sem_cola_ready, cola_tripulantes_ready);
                cambiar_estado(aux_tripulante, estado_tripulante[EXEC], socket_a_mi_ram_hq);
                monitor_lista(sem_lista_exec, (void*)list_add, lista_tripulantes_exec, aux_tripulante);
                // log_trace(logger, "Muevo %i a EXEC", aux_tripulante->TID);
            }

            else if(comparar_strings(ALGORITMO, "RR")){
                t_tripulante* aux_tripulante = monitor_cola_pop(sem_cola_ready, cola_tripulantes_ready);
                cambiar_estado(aux_tripulante, estado_tripulante[EXEC], socket_a_mi_ram_hq);
                aux_tripulante->quantum_restante = QUANTUM;
                monitor_lista(sem_lista_exec, (void*)list_add, lista_tripulantes_exec, aux_tripulante);
                // log_trace(logger, "Muevo %i a EXEC", aux_tripulante->TID);
            }
        }
    }
}

void tripulante(t_tripulante* un_tripulante){

    int st_ram = crear_socket_cliente(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
    int st_mongo = crear_socket_cliente(IP_I_MONGO_STORE, PUERTO_I_MONGO_STORE);

    usleep(500);
    log_trace(logger, "Iniciando tripulante: %i", un_tripulante->TID);
    char estado_guardado = un_tripulante->estado_tripulante; // NEW
    iniciar_tripulante(un_tripulante, st_ram);
    estado_guardado = un_tripulante->estado_tripulante; // READY

    if(comparar_strings(ALGORITMO, "FIFO")){
        log_info(logger, "ALGORITMO FIFO\n");
        ciclo_de_vida_fifo(un_tripulante, st_ram, st_ram, &estado_guardado);

    } else if (comparar_strings(ALGORITMO, "RR")){
        log_info(logger, "ALGORITMO RR\n");
        ciclo_de_vida_rr(un_tripulante, st_ram, st_ram, &estado_guardado);
    }

    close(st_ram);
    // close(st_mongo);
    morir(un_tripulante);

}

int conseguir_siguiente_tarea(t_tripulante* un_tripulante, int socket_ram, int socket_mongo){
    pedir_tarea_a_mi_ram_hq(un_tripulante->TID, socket_ram);
    t_estructura* tarea = recepcion_y_deserializacion(socket_ram);
    if(tarea->codigo_operacion == TAREA){
        un_tripulante->tarea = *tarea->tarea;
        if(llegue(un_tripulante)){
        	notificar_inicio_de_tarea(un_tripulante, socket_mongo);
        }
        return 1;
    }
    else if(tarea->codigo_operacion == FALLO){
        un_tripulante->estado_tripulante = estado_tripulante[EXIT];
    	cambiar_estado(un_tripulante, estado_tripulante[EXIT], socket_ram);
    }
    return 0;
}

void morir(t_tripulante* un_tripulante){
    // YA ESTA ACTUALIZADO EN RAM, ASI QUE NO HACE FALTA ACTUALIZAR AHI

    quitar_tripulante_de_listas(un_tripulante);

    if(esta_tripulante_en_lista(lista_tripulantes, un_tripulante->TID)){
        monitor_lista(sem_lista_tripulantes, (void*) eliminar_tripulante_de_lista, lista_tripulantes, (void*) un_tripulante->TID);
    }

    if(soy_el_ultimo_de_mi_especie(un_tripulante->TID)){
        eliminar_patota_de_lista(lista_patotas, un_tripulante->TID/10000);
        log_trace(logger, "Muere el ultimo de la patota %i", un_tripulante->TID/10000);
    }


    free(un_tripulante); // error de doble free en algun lado
    pthread_exit(NULL);
}

void iniciar_tripulante(t_tripulante* un_tripulante, int socket){
    // Se verifica que se creo bien y se enlista

    if (un_tripulante->estado_tripulante == estado_tripulante[NEW]){
        while(planificacion_activa == 0){
        	sleep(1);
            // por si lo matan antes de iniciar la planificacion
            if(un_tripulante->estado_tripulante == estado_tripulante[EXIT]){
                morir(un_tripulante);
            }
        }
        enlistarse(un_tripulante, socket);
    }
    else {
        log_error(logger, "\nPor un motivo desconocido, el tripulante se ha creado en un estado distinto a NEW. \n");
    }
}

void enlistarse(t_tripulante* un_tripulante, int socket){
    // Se le asigna la tarea y se lo pasa a READY
    pedir_tarea_a_mi_ram_hq(un_tripulante->TID, socket);

    t_estructura* respuesta = recepcion_y_deserializacion(socket);

    if(respuesta->codigo_operacion == TAREA){
        un_tripulante->tarea = *(respuesta->tarea);
    }
    else if (respuesta->codigo_operacion == FALLO){
        log_error(logger, "\nNo se recibio ninguna tarea.\n Codigo de error: FALLO\n");
    }
    else{
        log_error(logger, "\nNo se recibio ninguna tarea.\n Error desconocido\n");
    }
	cambiar_estado(un_tripulante, estado_tripulante[READY], socket);
    monitor_cola_push(sem_cola_ready, cola_tripulantes_ready, un_tripulante);
    log_debug(logger, "Estado cambiado a READY");

}

void realizar_tarea(t_tripulante* un_tripulante, int socket_ram, int socket_mongo){

    int codigo_tarea = identificar_tarea(un_tripulante->tarea.nombre);
    log_trace(logger, "%i mi tarea: %s", un_tripulante->TID, un_tripulante->tarea.nombre);

    // si el estado es distinto a bloqueado por ES
    switch(codigo_tarea){

        case GENERAR_OXIGENO:
            if(!llegue(un_tripulante)){
                atomic_llegar_a_destino(un_tripulante, socket_ram);
                notificar_movimiento(un_tripulante, socket_mongo);
            }else{
            	notificar_inicio_de_tarea(un_tripulante, socket_mongo);
            	sleep(RETARDO_CICLO_CPU);
            	t_buffer* b_oxigeno = serializar_cantidad(un_tripulante->tarea.parametro);
            	empaquetar_y_enviar(b_oxigeno, OXIGENO, socket_mongo);
            	cambiar_estado(un_tripulante, estado_tripulante[BLOCK], socket_ram);
            	monitor_cola_push(sem_cola_block, cola_tripulantes_block, un_tripulante);
            }
            break;

        case CONSUMIR_OXIGENO:
        	if(!llegue(un_tripulante)){
                atomic_llegar_a_destino(un_tripulante, socket_ram);
                notificar_movimiento(un_tripulante, socket_mongo);
			}else{
            	notificar_inicio_de_tarea(un_tripulante, socket_mongo);
				sleep(RETARDO_CICLO_CPU);
				t_buffer* b_oxigeno = serializar_cantidad(-un_tripulante->tarea.parametro);
				empaquetar_y_enviar(b_oxigeno, OXIGENO, socket_mongo);
            	cambiar_estado(un_tripulante, estado_tripulante[BLOCK], socket_ram);
            	monitor_cola_push(sem_cola_block, cola_tripulantes_block, un_tripulante);
            }
            break;

        case GENERAR_COMIDA:
            if(!llegue(un_tripulante)){
                atomic_llegar_a_destino(un_tripulante, socket_ram);
                notificar_movimiento(un_tripulante, socket_mongo);
            }else{
            	notificar_inicio_de_tarea(un_tripulante, socket_mongo);
            	sleep(RETARDO_CICLO_CPU);
            	t_buffer* b_oxigeno = serializar_cantidad(un_tripulante->tarea.parametro);
            	empaquetar_y_enviar(b_oxigeno, COMIDA ,socket_mongo);
            	cambiar_estado(un_tripulante, estado_tripulante[BLOCK], socket_ram);
            	monitor_cola_push(sem_cola_block, cola_tripulantes_block, un_tripulante);
            }
            break;

        case CONSUMIR_COMIDA:
            if(!llegue(un_tripulante)){
                atomic_llegar_a_destino(un_tripulante, socket_ram);
                notificar_movimiento(un_tripulante, socket_mongo);
            }else{
            	notificar_inicio_de_tarea(un_tripulante, socket_mongo);
            	sleep(RETARDO_CICLO_CPU);
            	t_buffer* b_oxigeno = serializar_cantidad(-un_tripulante->tarea.parametro);
            	empaquetar_y_enviar(b_oxigeno, COMIDA ,socket_mongo);
            	cambiar_estado(un_tripulante, estado_tripulante[BLOCK], socket_ram);
            	monitor_cola_push(sem_cola_block, cola_tripulantes_block, un_tripulante);
            }
            break;

        case GENERAR_BASURA:
            if(!llegue(un_tripulante)){
                atomic_llegar_a_destino(un_tripulante, socket_ram);
                notificar_movimiento(un_tripulante, socket_mongo);
            }else{
            	notificar_inicio_de_tarea(un_tripulante, socket_mongo);
            	sleep(RETARDO_CICLO_CPU);
            	t_buffer* b_oxigeno = serializar_cantidad(un_tripulante->tarea.parametro);
            	empaquetar_y_enviar(b_oxigeno, BASURA ,socket_mongo);
            	cambiar_estado(un_tripulante, estado_tripulante[BLOCK], socket_ram);
            	monitor_cola_push(sem_cola_block, cola_tripulantes_block, un_tripulante);
            }
            break;

        case DESCARTAR_BASURA:
            if(!llegue(un_tripulante)){
                atomic_llegar_a_destino(un_tripulante, socket_ram);
                notificar_movimiento(un_tripulante, socket_mongo);
            }else{
            	notificar_inicio_de_tarea(un_tripulante, socket_mongo);
            	sleep(RETARDO_CICLO_CPU);
            	t_buffer* b_oxigeno = serializar_cantidad(un_tripulante->tarea.parametro);
            	empaquetar_y_enviar(b_oxigeno, BASURA ,socket_mongo);
            	cambiar_estado(un_tripulante, estado_tripulante[BLOCK], socket_ram);
            	monitor_cola_push(sem_cola_block, cola_tripulantes_block, un_tripulante);
            }
            break;

        default:
            if(!llegue(un_tripulante)){
                atomic_llegar_a_destino(un_tripulante, socket_ram);
                notificar_movimiento(un_tripulante, socket_mongo);
                if(llegue(un_tripulante)){
                	notificar_inicio_de_tarea(un_tripulante, socket_mongo);
                }
            }else{
                atomic_no_me_despierten_estoy_trabajando(un_tripulante, socket_ram, socket_mongo);
            }
            break;
    }
}

int llegue(t_tripulante* un_tripulante){
    return un_tripulante->coord_x == un_tripulante->tarea.coord_x && un_tripulante->coord_y == un_tripulante->tarea.coord_y;
}

void atomic_llegar_a_destino(t_tripulante* un_tripulante, int socket){

    log_trace(logger, "%i Me encuentro en %i|%i.", un_tripulante->TID, un_tripulante->coord_x, un_tripulante->coord_y);

    uint32_t origen_x = un_tripulante->coord_x;
    uint32_t origen_y = un_tripulante->coord_y;
    uint32_t destino_x = un_tripulante->tarea.coord_x;
    uint32_t destino_y = un_tripulante->tarea.coord_y;

    uint32_t distancia_x = abs(destino_x - origen_x);
    uint32_t distancia_y = abs(destino_y - origen_y);

	sleep(RETARDO_CICLO_CPU);

	if(distancia_x != 0){

		destino_x > origen_x ? un_tripulante->coord_x++ : un_tripulante->coord_x--;
		distancia_x--;

	} else if (distancia_y != 0){

		destino_y > origen_y ? un_tripulante->coord_y++ : un_tripulante->coord_y--;
		distancia_y--;

	}

	actualizar_tripulante(un_tripulante, socket);
	log_trace(logger, "%i Distancia restante: %i!%i", un_tripulante->TID, distancia_x, distancia_y);

}


void atomic_no_me_despierten_estoy_trabajando(t_tripulante* un_tripulante, int socket_ram, int socket_mongo){

    if (un_tripulante->tarea.duracion > 0){
        sleep(RETARDO_CICLO_CPU);
        un_tripulante->tarea.duracion--;

        if (un_tripulante->tarea.duracion == 0){
            log_info(logger, "Tarea finalizada: %s\n", un_tripulante->tarea.nombre);
            notificar_fin_de_tarea(un_tripulante, socket_mongo);

    		cambiar_estado(un_tripulante, estado_tripulante[READY], socket_ram);
    		monitor_cola_push(sem_cola_ready, cola_tripulantes_ready, un_tripulante);

            if(conseguir_siguiente_tarea(un_tripulante, socket_ram, socket_mongo)){
            	log_trace(logger, "%i nueva tarea!: %s", un_tripulante->TID, un_tripulante->tarea.nombre);
            }
            else{
            	log_trace(logger, "%i Termine mis tareas!", un_tripulante->TID);
            }


        }else{
            log_debug(logger, "%i trabajando! tarea restante %i", un_tripulante->TID, un_tripulante->tarea.duracion);
        }
    }

    else {
        log_error(logger, "ERROR, duracion de tarea negativa.%i", un_tripulante->TID);
        log_error(logger, "o tripulante no pidio tarea.");
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

        //free(leido);

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

void expulsar_tripulante(char* leido) {

    printf("Expulsar Tripulante\n");
    char** palabras = string_split(leido, " ");
    int tid_tripulante_a_expulsar = atoi(palabras[1]);

    t_buffer* b_tid = serializar_entero(tid_tripulante_a_expulsar);
    empaquetar_y_enviar(b_tid, T_SIGKILL, socket_a_mi_ram_hq);

    t_estructura* respuesta = recepcion_y_deserializacion(socket_a_mi_ram_hq);

    if(respuesta->codigo_operacion == EXITO){
        if(esta_tripulante_en_lista(lista_tripulantes, tid_tripulante_a_expulsar)){

            bool obtener_tripulante(void* elemento){
                return (((t_tripulante*) elemento)->TID == tid_tripulante_a_expulsar);
            }

            t_tripulante* t_aux = list_find(lista_tripulantes, obtener_tripulante);

            log_info(logger, "Tripulante expulsado, TID: %d\n", tid_tripulante_a_expulsar);
            log_info(logger, "Lugar del deceso: %i|%i\n", t_aux->coord_x, t_aux->coord_y);
        	cambiar_estado(t_aux, estado_tripulante[EXIT], socket_a_mi_ram_hq);
            // t_aux->estado_tripulante = estado_tripulante[EXIT];
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

void crear_hilo_tripulante(t_tripulante* un_tripulante){

    pthread_t hilo_tripulante;
    pthread_create(&hilo_tripulante, NULL, (void*)tripulante, (void*)un_tripulante);
    pthread_detach(hilo_tripulante);
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

void listar_tripulantes() {

    log_debug(logger, "\nTripulantes en NEW: %i\n", list_size(lista_tripulantes_new));
    log_debug(logger, "\nTripulantes en READY: %i\n", queue_size(cola_tripulantes_ready));
    log_debug(logger, "\nTripulantes en EXEC: %i\n", list_size(lista_tripulantes_exec));
    log_debug(logger, "\nTripulantes en BLOQ I/O: %i\n", queue_size(cola_tripulantes_block));
    log_debug(logger, "\nTripulantes en BLOQ EMERGENCIA: %i\n", queue_size(cola_tripulantes_block));
    log_debug(logger, "\nTripulantes VIVOS: %i\n", list_size(lista_tripulantes));

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
            // TODO: dejarlo como logger
            // log_info(logger, "    Tripulante: %d \t   Patota: %d \t Status: %c\n", aux_t->TID, aux_t->TID/10000, aux_t->estado_tripulante);
        }
    }
    // TODO: revisar esto de abajo, tira segmentation fault en ciertos casos:
    // iniciar_patota("INICIAR_PATOTA 5 Random.ims 1|1 3|4");
    // iniciar_planificacion();
    // esṕerar a que termine
    // listar tripulantes
    // segmentation fault
    /*
    void liberar(void* elemento){
    	free(elemento);
    }

    list_destroy_and_destroy_elements(lista_tripulantes_de_una_patota, liberar);*/
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

void test_config_discordiador(){

    log_warning(logger, "%s", IP_MI_RAM_HQ);
    log_warning(logger, "%s", PUERTO_MI_RAM_HQ);
    log_warning(logger, "%s", IP_I_MONGO_STORE);
    log_warning(logger, "%s", PUERTO_I_MONGO_STORE);
    log_warning(logger, "%s", ALGORITMO);
    log_warning(logger, "%i", GRADO_MULTITAREA);
    log_warning(logger, "%i", QUANTUM);
    log_warning(logger, "%i", RETARDO_CICLO_CPU);
    log_warning(logger, "%i", DURACION_SABOTAJE);

}

int soy_el_ultimo_de_mi_especie(int tid){
    bool t_misma_patota(void* elemento1){
        return tid/10000 == (((t_tripulante*) elemento1)->TID)/10000;
    }

    int respuesta = list_count_satisfying(lista_tripulantes, t_misma_patota);
    // 0 porque elimino el tripulante de la lista antes de verificar esto
    // si lo eliminara despues de verificar, seria 1
    return respuesta == 0;
}


void* eliminar_patota_de_lista(t_list* lista, int elemento){

    bool contains(void* elemento1){
        return (elemento == ((t_patota*) elemento1)->PID);
    }

    t_patota* aux = list_remove_by_condition(lista, contains);
    return aux;
}

void test_soy_el_ultimo_de_mi_especie(){

    t_tripulante* trip_1 = malloc(sizeof(t_tripulante));
    trip_1->TID = 10001;
    t_tripulante* trip_2 = malloc(sizeof(t_tripulante));
    trip_2->TID = 10002;
    t_tripulante* trip_3 = malloc(sizeof(t_tripulante));
    trip_3->TID = 20001;
    t_tripulante* trip_4 = malloc(sizeof(t_tripulante));
    trip_4->TID = 20002;
    t_tripulante* trip_5 = malloc(sizeof(t_tripulante));
    trip_5->TID = 30001;

    list_add(lista_tripulantes, trip_1);
    list_add(lista_tripulantes, trip_2);
    list_add(lista_tripulantes, trip_3);
    list_add(lista_tripulantes, trip_4);
    list_add(lista_tripulantes, trip_5);

    printf("trip 1: %i", soy_el_ultimo_de_mi_especie(10001));
    printf("trip 2: %i", soy_el_ultimo_de_mi_especie(10002));
    printf("trip 3: %i", soy_el_ultimo_de_mi_especie(20001));
    printf("trip 4: %i", soy_el_ultimo_de_mi_especie(20002));
    printf("trip 5: %i", soy_el_ultimo_de_mi_especie(30001));
    eliminar_tripulante_de_lista(lista_tripulantes, 20002);
    printf("trip 3: %i", soy_el_ultimo_de_mi_especie(20001));

}

void test_eliminar_patota_de_lista(){

    t_patota* patota = malloc(sizeof(t_patota));
    patota->PID = 1;
    list_add(lista_patotas, patota);

    printf("una patota en la lista %i", list_size(lista_patotas));
    printf("la elimino");
    t_patota* otra_patota = eliminar_patota_de_lista(lista_patotas, 1);
    printf("cero patotas en la lista %i", list_size(lista_patotas));
    printf("pid de la eliminada: %i", otra_patota->PID);

}

void quitar_tripulante_de_listas(t_tripulante* un_tripulante){

    if(esta_tripulante_en_lista(lista_tripulantes_new, un_tripulante->TID)){
        monitor_lista(sem_lista_new, (void*) eliminar_tripulante_de_lista, lista_tripulantes_new, (void*) un_tripulante->TID);
    }

    if(esta_tripulante_en_lista(cola_tripulantes_ready->elements, un_tripulante->TID)){
        monitor_lista(sem_cola_ready, (void*) eliminar_tripulante_de_lista, cola_tripulantes_ready->elements, (void*) un_tripulante->TID);
    }

    if(esta_tripulante_en_lista(lista_tripulantes_exec, un_tripulante->TID)){
        monitor_lista(sem_lista_exec, (void*) eliminar_tripulante_de_lista, lista_tripulantes_exec, (void*) un_tripulante->TID);
    }

    if(esta_tripulante_en_lista(cola_tripulantes_block->elements, un_tripulante->TID)){
        monitor_lista(sem_cola_block, (void*) eliminar_tripulante_de_lista, cola_tripulantes_block->elements, (void*) un_tripulante->TID);
    }

    if(esta_tripulante_en_lista(cola_tripulantes_block_emergencia->elements, un_tripulante->TID)){
        monitor_lista(sem_cola_block_emergencia, (void*) eliminar_tripulante_de_lista, cola_tripulantes_block_emergencia->elements, (void*) un_tripulante->TID);
    }

}

void verificar_cambio_estado(char* estado_guardado, t_tripulante* un_tripulante, int socket){
    if(*estado_guardado != un_tripulante->estado_tripulante){
        log_warning(logger, "%i estoy en %c y cambio a %c", un_tripulante->TID, *estado_guardado, un_tripulante->estado_tripulante);
        actualizar_tripulante(un_tripulante, socket);
        *estado_guardado = un_tripulante->estado_tripulante;
    }
}

void actualizar_tripulante(t_tripulante* un_tripulante, int socket){
    if(un_tripulante->estado_tripulante != estado_tripulante[EXIT]){
        t_buffer* b_tripulante = serializar_tripulante(*un_tripulante);
        empaquetar_y_enviar(b_tripulante, ACTUALIZAR, socket);
    }
}

int verificacion_tcb(int socket){

    t_estructura* respuesta = recepcion_y_deserializacion(socket);

    if(respuesta->codigo_operacion == EXITO){
        log_info(logger, "Cargado el TCB en memoria ");
        free(respuesta);
        return 1;
    } else{
        log_warning(logger, "No hay memoria para el TCB");
        free(respuesta);
        return 0;
    }
}

void cambiar_estado(t_tripulante* un_tripulante, char estado, int socket){
	quitar_tripulante_de_listas(un_tripulante);
	un_tripulante->estado_tripulante = estado;
	actualizar_tripulante(un_tripulante, socket);
}


void peligro(char* posicion_sabotaje, int socket_ram){
	// PRIMERO los de EXEC, los de mayor TID primero
	// despues lo de READY, los de mayor TID primero

	pausar_planificacion();

	int i, j;
	t_tripulante* t_aux;
	t_list* lista_auxiliar = list_create();

    bool ordenar_por_tid(void* un_elemento, void* otro_elemento){
         return ((((t_tripulante*) un_elemento)->TID) > (((t_tripulante*) otro_elemento)->TID));
    }

    j = list_size(lista_tripulantes_exec);
	for(i = 0; i < j ; i++){
		t_aux = monitor_lista(sem_lista_exec, (void*) list_remove, lista_tripulantes_exec, 0);
        log_trace(logger, "Tripulante removido:\n tid: %i, estado: %c, pos: %i %i\n", (int)t_aux->TID, (char) t_aux->estado_tripulante, (int) t_aux->coord_x, (int) t_aux->coord_y);
		list_add(lista_auxiliar, t_aux);
	}
	list_sort(lista_auxiliar, ordenar_por_tid);

	j = list_size(lista_auxiliar);
	for(i = 0; i < j; i++){
		t_aux = list_remove(lista_auxiliar, 0);
		cambiar_estado(t_aux, estado_tripulante[PANIK], socket_ram);
		monitor_cola_push(sem_cola_block_emergencia, cola_tripulantes_block_emergencia, t_aux);
	}

    j = queue_size(cola_tripulantes_ready);
	for(i = 0; i < j; i++){
		t_aux = monitor_cola_pop(sem_lista_exec, cola_tripulantes_ready);
		log_trace(logger, "Tripulante removido:\n tid: %i, estado: %c, pos: %i %i\n", (int)t_aux->TID, (char) t_aux->estado_tripulante, (int) t_aux->coord_x, (int) t_aux->coord_y);
		list_add(lista_auxiliar, t_aux);
	}
	list_sort(lista_auxiliar, ordenar_por_tid);

	j = list_size(lista_auxiliar);

	for(i = 0; i < j; i++){
		t_aux = list_remove(lista_auxiliar, 0);
		cambiar_estado(t_aux, estado_tripulante[PANIK], socket_ram);
		monitor_cola_push(sem_cola_block_emergencia, cola_tripulantes_block_emergencia, t_aux);
	}

	// si necesitamos printear
	// j = queue_size(cola_tripulantes_block_emergencia);

	// for(i = 0; i < j; i++){
		// t_aux = queue_pop(cola_tripulantes_block_emergencia);
		// log_trace(logger, "Tripulante removido:\n tid: %i, estado: %c, pos: %i %i\n", (int)t_aux->TID, (char) t_aux->estado_tripulante, (int) t_aux->coord_x, (int) t_aux->coord_y);
	// }

	list_destroy(lista_auxiliar);

	estamos_en_peligro = 1;

	t_aux = tripulante_mas_cercano_a(posicion_sabotaje);
	log_warning(logger, "%i ES EL TRIPULANTE MAS CERCANO", t_aux->TID);

	t_tarea contexto = t_aux->tarea;
	t_tarea resolver_sabotaje;
	resolver_sabotaje.coord_x = posicion_sabotaje[0] - 48;
	resolver_sabotaje.coord_y = posicion_sabotaje[2] - 48;
	resolver_sabotaje.duracion = DURACION_SABOTAJE;
	t_aux->tarea = resolver_sabotaje;

    log_error(logger, "Cordenada en X: %i\n", resolver_sabotaje.coord_x);
    log_error(logger, "Cordenada en Y: %i\n", resolver_sabotaje.coord_y);
    log_error(logger, "Duracion: %i\n", resolver_sabotaje.duracion);

	cambiar_estado(t_aux, estado_tripulante[EXEC], socket_ram);

	while(estamos_en_peligro){
		sleep(1); // todos se esperan a que termine el sabotaje
	}

	t_aux->tarea = contexto;

	// al terminar el sabotaje paso todos a ready (issue #2163)
	j = queue_size(cola_tripulantes_block_emergencia);

	for(i = 0; i < j; i++){
		t_aux = monitor_cola_pop(sem_cola_block_emergencia, cola_tripulantes_block_emergencia);
		cambiar_estado(t_aux, estado_tripulante[READY], socket_ram);
		monitor_cola_push(sem_cola_ready, cola_tripulantes_ready, t_aux);
	}

	iniciar_planificacion();

}

t_tripulante* tripulante_mas_cercano_a(char* posicion){
	int x_sabotaje = posicion[0];
	int y_sabotaje = posicion[2];
	t_tripulante* un_tripulante;

	void* mas_cercano (void* un_trip, void* otro_trip){
		int distancia_x_t1 = abs(x_sabotaje - ((t_tripulante*) un_trip)->coord_x);
		int distancia_y_t1 = abs(y_sabotaje - ((t_tripulante*) un_trip)->coord_y);
		int distancia_x_t2 = abs(x_sabotaje - ((t_tripulante*) otro_trip)->coord_x);
		int distancia_y_t2 = abs(y_sabotaje - ((t_tripulante*) otro_trip)->coord_y);

		int distancia_t1 = distancia_x_t1 + distancia_y_t1;
		int distancia_t2 = distancia_x_t2 + distancia_y_t2;

		if(distancia_t1 < distancia_t2 ){
			return un_trip;
		}
		else{
			return otro_trip;
		}

	}

	un_tripulante = list_get_minimum(cola_tripulantes_block_emergencia->elements, mas_cercano);

	return un_tripulante;
}

void resolver_sabotaje(t_tripulante* un_tripulante, int socket_ram, int socket_mongo){
	notificar_inicio_sabotaje(un_tripulante, socket_mongo);

	while(!llegue(un_tripulante)){
		atomic_llegar_a_destino(un_tripulante, socket_ram);
		notificar_movimiento(un_tripulante, socket_mongo);
		log_warning(logger, "AAAAAAAAAAAAA");
	}

	while(un_tripulante->tarea.duracion > 0){
		sleep(RETARDO_CICLO_CPU);
		un_tripulante->tarea.duracion--;
		log_warning(logger, "DESABOTEATEEEEE");
	}

	estamos_en_peligro = 0;
	log_warning(logger, "Sabotaje resuelto, viviremos un dia mas.");
	notificar_fin_sabotaje(un_tripulante, socket_mongo);
}

void esperar_entrada_salida(t_tripulante* un_tripulante, int st_ram, int st_mongo){
	atomic_no_me_despierten_estoy_trabajando(un_tripulante, st_ram, st_mongo);
}

void guardian_sabotaje(int st_ram, int st_mongo){ // TODO: Sinergia con el mongo

	t_estructura* mensaje_peligro = recepcion_y_deserializacion(st_mongo);
	if (mensaje_peligro->codigo_operacion == SABOTAJE){
		// peligro(POS, st_ram);
	}
	else{
		log_info(logger, "No hay sabotajes a la vista");
	}

}

int es_mi_turno(t_tripulante* un_tripulante){
	t_tripulante* titular = monitor_cola_pop_or_peek(sem_cola_block, (void*) queue_peek, cola_tripulantes_block);
	return (titular == un_tripulante);
}



void ciclo_de_vida_fifo(t_tripulante* un_tripulante, int st_ram, int st_mongo, char* estado_guardado){
    while(un_tripulante->estado_tripulante != estado_tripulante[EXIT]){
        verificar_cambio_estado(estado_guardado, un_tripulante, st_ram);
        if(un_tripulante->estado_tripulante == estado_tripulante[EXEC]){
        	if(estamos_en_peligro){
				resolver_sabotaje(un_tripulante, st_ram, st_mongo);
			}
            if(planificacion_activa == 0){
            	sleep(1); // este espacio vacio es EXACTAMENTE lo que tiene que estar
            }
            else{
                realizar_tarea(un_tripulante, st_ram, st_mongo);
            }
        } else if (un_tripulante->estado_tripulante == estado_tripulante[BLOCK]){
            if(planificacion_activa == 0){
            	sleep(1); // este espacio vacio es EXACTAMENTE lo que tiene que estar
            } else{
            	if(es_mi_turno(un_tripulante)){
					esperar_entrada_salida(un_tripulante, st_ram, st_mongo);
				} else {
					log_trace(logger, "%i espero mi turno!", un_tripulante->TID);
					sleep(1); // espero hasta que la entrada deje de estar ocupada
				}
            }
        }
        verificar_cambio_estado(estado_guardado, un_tripulante, st_ram);
    }
}

void ciclo_de_vida_rr(t_tripulante* un_tripulante, int st_ram, int st_mongo, char* estado_guardado){
    while(un_tripulante->estado_tripulante != estado_tripulante[EXIT]){
        verificar_cambio_estado(estado_guardado, un_tripulante, st_ram);
        if(un_tripulante->estado_tripulante == estado_tripulante[EXEC]){
        	if(estamos_en_peligro){
				resolver_sabotaje(un_tripulante, st_ram, st_mongo);
        	}

            if(un_tripulante->quantum_restante > 0){
                if(planificacion_activa == 0){
                	sleep(1);
                    // este espacio vacio es EXACTAMENTE lo que tiene que estar
                }
                else{
                    realizar_tarea(un_tripulante, st_ram, st_mongo);
                    un_tripulante->quantum_restante--;
                    log_debug(logger, "%i Trabaaajo muy duuro, como un esclaaaaavo: quantum restante %i", un_tripulante->TID, un_tripulante->quantum_restante);
                }

            } else {
            	cambiar_estado(un_tripulante, estado_tripulante[READY], st_ram);
                monitor_cola_push(sem_cola_ready, cola_tripulantes_ready, un_tripulante);
                log_warning(logger, "%i Ayudaaa me desalojan", un_tripulante->TID);
                actualizar_tripulante(un_tripulante, st_ram);
            }
        } else if (un_tripulante->estado_tripulante == estado_tripulante[BLOCK]){
            if(planificacion_activa == 0){
            	sleep(1); // este espacio vacio es EXACTAMENTE lo que tiene que estar
            } else{
            	if(es_mi_turno(un_tripulante)){
					esperar_entrada_salida(un_tripulante, st_ram, st_mongo);
				} else {
					log_trace(logger, "%i espero mi turno!", un_tripulante->TID);
					sleep(1); // espero hasta que la entrada deje de estar ocupada
				}
            }
        }

    verificar_cambio_estado(estado_guardado, un_tripulante, st_ram);
    }
}
