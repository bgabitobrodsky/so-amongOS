/*
 ============================================================================
 Name        : Discordiador.c
 Author      : Rey de fuego
 Version     : 1
 Copyright   : Mala yuyu
 Description : El discordiador
 ============================================================================
 */

// TODO: agregar a los monitorees verificaciones por si alguien intenta quitar cosas
// de una lista vacia.

#define IP_MI_RAM_HQ config_get_string_value(config, "IP_MI_RAM_HQ")
#define PUERTO_MI_RAM_HQ config_get_string_value(config, "PUERTO_MI_RAM_HQ")
#define IP_I_MONGO_STORE config_get_string_value(config, "IP_I_MONGO_STORE")
#define PUERTO_I_MONGO_STORE config_get_string_value(config, "PUERTO_I_MONGO_STORE")
#define ALGORITMO config_get_string_value(config, "ALGORITMO")
#define GRADO_MULTITAREA config_get_string_value(config, "GRADO_MULTITAREA")
#define QUANTUM config_get_int_value(config, "QUANTUM")
#define RETARDO_CICLO_CPU config_get_int_value(config, "RETARDO_CICLO_CPU")
#define DURACION_SABOTAJE config_get_int_value(config, "DURACION_SABOTAJE")
#define LIMIT_CONNECTIONS 10
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
t_list* lista_tripulantes_new;
t_list* lista_tripulantes_exec;

t_queue* cola_tripulantes_ready;

pthread_mutex_t sem_lista_exec;
pthread_mutex_t sem_lista_new;
pthread_mutex_t sem_cola_ready;

// Variables de discordiador
char estado_tripulante[4] = {'N', 'R', 'E', 'B'};
int planificacion_activa = 0;
int sistema_activo = 1;

int main() {
    logger = log_create("discordiador.log", "discordiador", true, LOG_LEVEL_INFO);
    config = config_create("discordiador.config");

    iniciar_listas();
    iniciar_colas();
    iniciar_semaforos();

    socket_a_mi_ram_hq = crear_socket_cliente(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
    socket_a_mongo_store = crear_socket_cliente(IP_I_MONGO_STORE, PUERTO_I_MONGO_STORE);

    if (socket_a_mi_ram_hq != -1 && socket_a_mongo_store != -1) {
    	leer_consola();
        pthread_t hiloConsola;
        pthread_create(&hiloConsola, NULL, (void*)leer_consola, NULL);
        pthread_detach(hiloConsola);

        //pthread_join(hiloConsola, NULL);

        /*
        args_escuchar args_disc;
        args_disc.socket_oyente = socket_a_mi_ram_hq;
        pthread_t hilo_escucha;
        pthread_create(&hilo_escucha, NULL, (void*) proceso_handler, &args_disc);
        pthread_join(hilo_escucha, NULL);
        */
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

void iniciar_patota(char* leido) {
    char** palabras = string_split(leido, " ");
    int cantidadTripulantes = atoi(palabras[1]);
    char* path = palabras[2];

    printf("PATOTA: cantidad de tripulantes %d, url: %s \n", cantidadTripulantes, path);

    int i = 0;

    // pasarle el pcb a ram y matarlo
    t_PCB* pcb = crear_pcb(path);
    char* archivo_tareas = leer_archivo_entero(path);

    if (archivo_tareas != NULL){
    	enviar_archivo_tareas(archivo_tareas, pcb->PID, socket_a_mi_ram_hq);
    }

    t_TCB* aux;

    while (palabras[i+3] != NULL){
        printf("POSICION %d: %s \n", i+1, palabras[i+3]);
        // void* funcion = pedir_funcion()
        // aux = iniciar_tcb(NULL, pcb, ((pcb->PID)*10000) + i+1, palabras[i+3]);
        aux = crear_puntero_tcb(pcb, ((pcb->PID)*10000) + i+1, palabras[i+3]);
        enviar_tcb_a_ram(*aux, socket_a_mi_ram_hq);
        monitor_lista_dos_parametros(sem_lista_new, (void*) list_add, lista_tripulantes_new, aux);
        printf("Tripulante %i, estado: %c, pos: %i %i\n", (int)aux->TID, (char) aux->estado_tripulante, (int) aux->coord_x, (int) aux->coord_y);

        free(aux);
        i++;
    }

    for(int j = i+1; j <= cantidadTripulantes; j++){
        printf("POSICION %d: 0|0\n", j);
        // void* funcion = pedir_funcion()
        // aux = iniciar_tcb(NULL, pcb, ((pcb->PID)*10000) + j, "0|0");
        aux = crear_puntero_tcb(pcb, ((pcb->PID)*10000) + j, "0|0");
        enviar_tcb_a_ram(*aux, socket_a_mi_ram_hq);
        monitor_lista_dos_parametros(sem_lista_new, (void*) list_add, lista_tripulantes_new, aux);
        printf("Tripulante %i, estado: %c, pos: %i %i\n", (int)aux->TID, (char) aux->estado_tripulante, (int) aux->coord_x, (int) aux->coord_y);

        free(aux);
    }

    free(pcb);
    liberar_puntero_doble(palabras);
}


void iniciar_planificacion() {
    printf("Iniciar Planificacion");
    planificacion_activa = 1;

    //codigo para testeos
    t_tarea* aux_tarea = malloc(sizeof(t_tarea));
    aux_tarea->duracion = 5;

    int tiempo_restante = aux_tarea->duracion;

    // TODO NO TESTEADO
    while(planificacion_activa && list_size(lista_tripulantes_exec) < atoi(GRADO_MULTITAREA)){
        t_TCB* aux_tripulante = monitor_cola_pop(sem_cola_ready, cola_tripulantes_ready);
        aux_tripulante->estado_tripulante = estado_tripulante[EXEC];

        monitor_lista_dos_parametros(sem_lista_exec, (void*)list_add, lista_tripulantes_exec, aux_tripulante);

        if(comparar_strings(ALGORITMO, "FIFO")){
            free(aux_tripulante);
        }
        else if(comparar_strings(ALGORITMO, "RR")){

            sleep(MIN(tiempo_restante, QUANTUM)*RETARDO_CICLO_CPU);
            // actualizar la duracion de la tarea
            monitor_lista_dos_parametros(sem_lista_exec, (void*)eliminar_tcb_de_lista, lista_tripulantes_exec, aux_tripulante);
            aux_tripulante->estado_tripulante = estado_tripulante[READY];
            monitor_cola_push(sem_cola_ready, cola_tripulantes_ready, aux_tripulante);
            free(aux_tripulante);
        }
    }
}

void listar_tripulantes() { //TODO falta testear

    printf("listar Tripulantes\n");

    char* fechaHora = fecha_y_hora();

    printf("Estado de la nave: %s\n", fechaHora);

    t_PCB* aux_p;
    t_TCB* aux_t;

    int i,j;

    for(i = 0; i < list_size(lista_patotas); i++){
        aux_p = list_get(lista_patotas, i);

        for(j = 0; j < list_size(lista_tripulantes_patota(aux_p)); j++){
            aux_t = list_get(lista_tripulantes_patota(aux_p), j);
            printf("Tripulante: %d\tPatota: %d\tStatus: %i", aux_t->TID, ((t_PCB*) (aux_t->puntero_a_pcb))->PID, aux_t->estado_tripulante);
            printf("Status: %d\t", ((t_PCB*) (aux_t->puntero_a_pcb))->PID);
        }
    }

    free(aux_p);
    free(aux_t);
}

t_list* lista_tripulantes_patota(t_PCB* pcb){ //TODO codear

    // Necesito listar todos los tripulantes que estan en RAM, y los que estan en NEW
    // Primero; le mando a ram el PID o el PCB (posiblemente este).
    // Segundo, RAM ejecuta su funcion y me devuelve los tripulantes con ese PCB asociado
    // Tercero, los guardo en una lista
    // Cuarto, si lo tenemos diseñado que los tripulantes de NEW no esten en RAM,
    // que es lo teoricamente correcto, entonces verifico lo mismo con la lista o cola de NEW
    // de discordiador y uno las dos listas
    // Quinto: devuelvo la lista

    // t_list* aux_lista_tripulantes = list_create();
    // pedido_ram_tripulantes (t_PCB* pcb);
    // aux_lista_tripulantes = recibir_tripulantes_de_ram();
    // sacar lista_tripulantes_new_de_esa_patota, de Discordiador con las funciones de list.h;
    // juntar(aux_lista_tripulantes, lista_tripulantes_new_de_esa_patota);
    // return aux_lista_tripulantes;
    // list_destroy(aux_lista_tripulantes); debo destrozar esto LUEGO de usarla en la funcion anterior

    return NULL;
}

/*
void lista_tripulantes_patota_version_RAM(t_PCB* pcb){

    // verifico que el pcb/patota esta en la lista de pcbs/patotas
    // en una lista de tripulantes, me quedo con aquellos que cumplan que su puntero_a_pcb es igual al pcb que tenemos.
    // envio esa lista a Discordiador, o bien enviarlos uno por uno que puede ser mas facil

}*/

void pausar_planificacion() {
    printf("Pausar Planificacion\n");
    planificacion_activa = 0;
}

void obtener_bitacora(char* leido) {
    printf("Obtener Bitacora\n");
    enviar_codigo(MENSAJE, socket_a_mi_ram_hq);
}

void expulsar_tripulante(char* leido) {
    printf("Expulsar Tripulante\n");
    char** palabras = string_split(leido, " ");
    int tid_tripulante_a_expulsar = atoi(palabras[1]);

    t_sigkill tripulante_tid;
    tripulante_tid.tid = tid_tripulante_a_expulsar;
    t_buffer* b_tid = serializar_tid(tripulante_tid);
    empaquetar_y_enviar(b_tid, T_SIGKILL, socket_a_mi_ram_hq);

    t_estructura* respuesta = recepcion_y_deserializacion(socket_a_mi_ram_hq);
    if(respuesta->codigo_operacion == EXITO){
    	// TODO: EN el caso de que el tripulante este en discordiador, eliminarlo tambien
    	// monitor_lista_dos_parametros(sem_lista_new, eliminar_tripulante_de_lista, lista_tripulantes_exec, (void*) tid_tripulante_a_expulsar);
    	log_info(logger, "Tripulante expulsado, TID: %d", tid_tripulante_a_expulsar);
    }
    else if (respuesta->codigo_operacion == FALLO){
    	log_info(logger, "No existe el tripulante. TID: %d", tid_tripulante_a_expulsar);
    }
    else{
    	log_info(logger, "Error desconocido.\n");
    }

    liberar_puntero_doble(palabras);
}

char* fecha_y_hora() {
    time_t tiempo = time(NULL);
    struct tm tiempoLocal = *localtime(&tiempo);
    static char fecha_Hora[70];
    char *formato = "%d-%m-%Y %H:%M:%S";
    int bytesEscritos = strftime(fecha_Hora, sizeof fecha_Hora, formato, &tiempoLocal);

    if (bytesEscritos != 0) {
        return fecha_Hora;
    }
    else {
        return "Error formateando fecha";
    }
}

int esta_en_lista(t_list* lista, int elemento){
    bool contiene(void* elemento1){
    return sonIguales(elemento, (int) elemento1);
    }
    int a = list_any_satisfy(lista, contiene);
    return a;
}

int sonIguales(int elemento1, int elemento2){
        return elemento1 == elemento2;
    }

void enlistar_algun_tripulante(){
	if (!list_is_empty(lista_tripulantes_new)){
		t_TCB* tripulante_a_ready = monitor_lista_dos_parametros(sem_lista_new, (void*) list_remove, lista_tripulantes_new, (void*) 0 );
		monitor_cola_push(sem_cola_ready, cola_tripulantes_ready, tripulante_a_ready);
		pedir_tarea_a_mi_ram_hq(tripulante_a_ready->TID, socket_a_mi_ram_hq);

		t_estructura* respuesta = recepcion_y_deserializacion(socket_a_mi_ram_hq);

		if(respuesta->codigo_operacion == TAREA){
			// TODO RELLENAR LUEGO DE MUDARNOS DE T_TCB A TRIPULANTES
			// tripulante_a_ready->tarea = respuesta->tarea;
		}
		else if (respuesta->codigo_operacion == FALLO){
			log_info(logger, "No se recibio ninguna tarea.\n Codigo de error: FALLO\n");
		}
		else{
			log_info(logger, "Error desconocido, no se recibio ninguna tarea.\n");
		}

		// Como son punteros, al cambiar el estado a esa posicion de memoria tambien
		// se lo cambia al elemento de la cola
		tripulante_a_ready->estado_tripulante = estado_tripulante[READY];
	}
	else {
		log_info(logger, "No hay ningun tripulante listo para ser enlistado.\n");
	}
}


void proceso_handler(void* args) {
	args_escuchar* p = malloc(sizeof(args_escuchar));
	p = args;
	int socket_escucha = p->socket_oyente;
	//int socket_escucha = (int) args; //Tema de testeos, no borrar

    int addrlen, socket_especifico;
    struct sockaddr_in address;

    addrlen = sizeof(address);

	// struct sockaddr_storage direccion_a_escuchar;
	// socklen_t tamanio_direccion;

	if (listen(socket_escucha, LIMIT_CONNECTIONS) == -1)
		printf("Error al configurar recepcion de mensajes\n");

	while (1) {
		if ((socket_especifico = accept(socket_escucha, (struct sockaddr*) &address, (socklen_t *) &addrlen)) > 0) {
			// Maté la verificación
			log_info(logger, "Se conecta un nuevo proceso");

			hilo_tripulante* parametros = malloc(sizeof(hilo_tripulante));

			parametros->socket = socket_especifico;
			//parametros->ip_cliente = inet_ntoa(address.sin_addr);
			//parametros->puerto_cliente = ntohs(address.sin_port);

			pthread_t un_hilo_tripulante;

			pthread_create(&un_hilo_tripulante, NULL, (void*) atender_clientes, (void *) parametros);

			pthread_detach(un_hilo_tripulante);
		}
	}
}

void atender_clientes(void* param) {
	hilo_tripulante* parametros = param;

	int flag = 1;
	log_info(logger, "Atendiendo. %i\n", parametros->socket);

	while(flag) {
		t_estructura* mensaje_recibido = recepcion_y_deserializacion(parametros->socket);

		//sleep(1); //para que no se rompa en casos de bug o tiempos de espera

        switch(mensaje_recibido->codigo_operacion) {
            case EXITO:
                log_info(logger, "Mensaje recibido");
                break;

            case FALLO:
                log_info(logger, "Pedido de tarea recibido");
                break;

            case DESCONEXION:
                log_info(logger, "Se desconecto el modulo");
                flag = 0;
                break;

            default:
                log_info(logger, "Se recibio un codigo invalido.");
                break;
        }
		free(mensaje_recibido);
	}

}

void enviar_tcb_a_ram(t_TCB un_tcb, int socket){
    t_buffer* buffer_tcb = serializar_tcb(un_tcb);
    empaquetar_y_enviar(buffer_tcb , RECIBIR_TCB, socket);
    // NOTA: si se libera el buffer, tira error de doble free.
}

int esta_tcb_en_lista(t_list* lista, t_TCB* elemento){
    bool contains(void* elemento1){
        return (elemento->TID == ((t_TCB*) elemento1)->TID);
        }
    bool a = list_any_satisfy(lista, contains);
    return a;
}

void* eliminar_tcb_de_lista(t_list* lista, t_TCB* elemento){
    bool contains(void* elemento1){
        return (elemento->TID == ((t_TCB*) elemento1)->TID);
        }
    t_TCB* aux = list_remove_by_condition(lista, contains);
    return aux;
}

/*
t_patota* crear_patota(t_PCB* un_pcb){
    t_patota* patota = malloc(sizeof(t_patota));
    patota -> pcb = un_pcb;
    //patota -> archivo_de_tareas
    return patota;
}*/

t_PCB* crear_pcb(char* path){
    t_PCB* pcb = malloc(sizeof(t_PCB));
    pcb -> PID = (uint32_t) nuevo_pid();
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
}

void iniciar_hilo_tripulante(void* funcion){
    pthread_t hilo1;
    pthread_create(&hilo1, NULL, funcion, NULL);
}


t_TCB* crear_puntero_tcb(t_PCB* pcb, int tid, char* posicion){
	// No asigna siguiente instruccion
    t_TCB* tcb = malloc(sizeof(t_TCB));
    tcb -> TID = tid;
    tcb -> estado_tripulante = estado_tripulante[NEW];
    tcb -> coord_x = posicion[0] - 48; // equivalencia ascii - numero
    tcb -> coord_y = posicion[2] - 48; // equivalencia ascii - numero
    tcb -> siguiente_instruccion = 0;
    tcb -> puntero_a_pcb = (uint32_t) pcb;

    return tcb;
}

t_TCB crear_tcb(t_PCB* pcb, int tid, char* posicion){
	// No asigna siguiente instruccion
    t_TCB tcb;
    tcb.TID = tid;
    tcb.estado_tripulante = estado_tripulante[NEW];
    tcb.coord_x = posicion[0] - 48; // equivalencia ascii - numero;
    tcb.coord_y = posicion[2] - 48; // equivalencia ascii - numero;
    tcb.siguiente_instruccion = 0;
    tcb.puntero_a_pcb = (uint32_t) pcb;

    return tcb;
}

t_TCB* iniciar_tcb(void* funcion, t_PCB* pcb, int tid, char* posicion){
    t_TCB* un_tcb = crear_puntero_tcb(pcb, tid, posicion);
    //t_tripulante* nuestro_tripulante = crear_tripulante(un_tcb);
    iniciar_hilo_tripulante(funcion);
    return un_tcb;
}
