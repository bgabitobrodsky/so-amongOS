/*
 * mi_ram_hq.c
 *
 *  Created on: 9 may. 2021
 *      Author: utnso
 */

#include "mi_ram_hq.h"
#include <comms/generales.h>


#define	IP config_get_string_value(config, "IP")
#define PUERTO config_get_string_value(config, "PUERTO")
#define TAMANIO_MEMORIA config_get_int_value(config, "TAMANIO_MEMORIA")
#define TAMANIO_PAGINA config_get_int_value(config, "TAMANIO_PAGINA")
#define ESQUEMA_MEMORIA config_get_string_value(config, "ESQUEMA_MEMORIA")
#define CRITERIO_SELECCION config_get_string_value(config, "CRITERIO_SELECCION")
#define LIMIT_CONNECTIONS 10

t_log* logger;
t_config* config;

t_list* lista_tcb;
t_list* lista_pcb;	

char estado_tripulante[4] = {'N', 'R', 'E', 'B'};

void gestionar_tareas (t_archivo_tareas* archivo_tareas){
	int pid_patota = archivo_tareas->pid;
	size_t tamanio_tareas = archivo_tareas->largo_texto * sizeof(char);

	if(strcmp(ESQUEMA_MEMORIA, "SEGMENTACION") == 0){
		tabla_segmentos* tabla = (tabla_segmentos*) buscar_tabla(pid_patota);
		if(tabla == NULL){ 
			tabla = crear_tabla_segmentos(pid_patota);
		}
		
		//Creamos segmento para tareas y lo guardamos en la tabla de la patota
		segmento* segmento_tareas = asignar_segmento(tamanio_tareas);
		void* puntero_a_tareas = memcpy(memoria_principal + segmento_tareas->base, archivo_tareas->texto, tamanio_tareas);
		tabla->segmento_tareas = segmento_libre;
		
		//Creamos el PCB
		t_PCB* pcb = malloc(sizeof(t_PCB));
		pcb->pid = pid_patota;
		pcb->direccion_tareas = puntero_a_tareas;

		//Creamos el segmento para el PCB y lo guardamos en la tabla de la patota
		segmento* segmento_pcb = asignar_segmento(sizeof(t_PCB));
		memcpy(memoria_principal + segmento_pcb->base, pcb, sizeof(t_PCB));
		tabla->segmento_pcb = segmento_pcb;

	}else if(strcmp(ESQUEMA_MEMORIA, "SEGMENTACION") == 0){

	}else{
		log_error(logger, "Esquema de memoria desconocido");
		exit(EXIT_FAILURE);
	}
}

void gestionar_pedido_tarea(int tid, int socket){
	// si necesitas el pid, es este
	int pid = tid / 10000;
	// para enviarme una tarea:
	// t_buffer* buffer_tarea = serializar_tarea(t_tarea una_tarea);
	// empaquetar_y_enviar(buffer_tarea, TAREA, socket);
	//
	// si sale mal por alguna razon o no hay tarea;
	// enviar_codigo(FALLO, socket);
}


int main(int argc, char** argv) {
	
	// Reinicio el log
	FILE* f = fopen("mi_ram_hq.log", "w");
    fclose(f);

	// Inicializar
	logger = log_create("mi_ram_hq.log", "MI_RAM_HQ", 1, LOG_LEVEL_DEBUG);
	config = config_create("mi_ram_hq.config");

	lista_tcb = list_create();
	lista_pcb = list_create();

	iniciar_memoria();

	pagina* pag= list_get(paginas,3);
	log_info(logger,"pagina 3, base: %d", pag->base);

	//iniciar_mapa(); TODO dibujar mapa inicial vacio
/*
	int socket_oyente = crear_socket_oyente(IP, PUERTO);
    args_escuchar args_miram;
	args_miram.socket_oyente = socket_oyente;

	pthread_t hilo_escucha;
	pthread_create(&hilo_escucha, NULL, (void*) proceso_handler, (void*) &args_miram);

	//pthread_detach(hilo_escucha);
	pthread_join(hilo_escucha, NULL);
	close(socket_oyente);
*/

	log_destroy(logger);
	config_destroy(config);

	return EXIT_SUCCESS;
}

void proceso_handler(void* args) {
	log_info(logger,"Se inicia el servidor multi-hilo");
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
		log_error(logger,"Error al configurar recepcion de mensajes");

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

			case ARCHIVO_TAREAS:
				log_info(logger, "Recibido contenido del archivo\n");
				printf("\tpid:%i. \n\tlongitud; %i. \n%s\n", mensaje_recibido->archivo_tareas->pid, mensaje_recibido->archivo_tareas->largo_texto, mensaje_recibido->archivo_tareas->texto);
				gestionar_tareas(mensaje_recibido->archivo_tareas);
				sleep(1);
				break;

			case MENSAJE:
				log_info(logger, "Mensaje recibido\n");
				break;

			case PEDIR_TAREA:
				log_info(logger, "Pedido de tarea recibido\n");
				log_info(logger, "Tripulante: %i\n", mensaje_recibido->tid_condenado->tid);
				// TODO: GABITO Y JULIA
				// gestionar_pedido_tarea(mensaje_recibido->tid_condenado->tid, parametros->socket);
				break;

			case RECIBIR_PCB:
				//log_info(logger, "Recibo una pcb\n");
				//almacenar_pcb(mensaje_recibido); //TODO en un futuro, capaz no podamos recibir el PCB por quedarnos sin memoria
				free(mensaje_recibido->pcb);
				break;

			case RECIBIR_TCB:
				log_info(logger, "Recibo una tcb\n");
				log_info(logger, "Tripulante %i, estado: %c, pos: %i %i, puntero_pcb: %i, sig_ins %i\n", (int) mensaje_recibido->tcb->TID, (char) mensaje_recibido->tcb->estado_tripulante, (int) mensaje_recibido->tcb->coord_x, (int) mensaje_recibido->tcb->coord_y, (int) mensaje_recibido->tcb->puntero_a_pcb, (int) mensaje_recibido->tcb->siguiente_instruccion);
				list_add(lista_tcb, (void*) mensaje_recibido->tcb);
				free(mensaje_recibido->tcb);
				//printf("Tripulante 1 pos: %c %c\n", (int) mensaje_recibido->tcb->coord_x, (int) mensaje_recibido->tcb->coord_y);
				break;

			case T_SIGKILL:
				log_info(logger, "Expulsar Tripulante.\n");
				// TODO: GABITO Y JULIA
				// verifica si existe
				// si existe mandame un enviar_codigo(EXITO, parametros->socket);
				// si no existe, mandame un enviar_codigo(FALLO, parametros->socket);
				log_info(logger, "%i -KILLED", mensaje_recibido->tid_condenado->tid);
				enviar_codigo(EXITO, parametros->socket);
				break;

			case LISTAR_POR_PID:
				log_info(logger, "Recibido pedido de tripulantes.\n");
				// TODO: GABITO Y JULIA
				// consultarme que hacer aca
				enviar_codigo(EXITO, parametros->socket);
				break;

			case DESCONEXION:
				log_info(logger, "Se desconecto un cliente.\n");
				flag = 0;
				// close(parametros->socket);
				break;

			default:
				log_info(logger, "Se recibio un codigo invalido.\n");
				printf("El codigo es %d\n", mensaje_recibido->codigo_operacion);
				break;
		}
		free(mensaje_recibido);
	}

}


t_PCB* crear_pcb(char* path){
	t_PCB* pcb = malloc(sizeof(t_PCB));
	pcb -> PID = 1;
	pcb -> direccion_tareas = (uint32_t) path;
	return pcb;
}

t_TCB crear_tcb(t_PCB* pcb, int tid, char* posicion){
	t_TCB tcb;
	tcb.TID = tid;
	tcb.estado_tripulante = estado_tripulante[NEW];
	tcb.coord_x = posicion[0];
	tcb.coord_y = posicion[2];
	tcb.siguiente_instruccion = 5; //TODO
	tcb.puntero_a_pcb = 7;
	return tcb;
}

indice_tabla* crear_indice(int pid, void* tabla){
	log_info(logger,"Se crea indice para la tabla de la patota %d",pid);
	indice_tabla* nuevo_indice = malloc(sizeof(indice_tabla));
	nuevo_indice->pid = pid;
	nuevo_indice->tabla = tabla;
	return nuevo_indice;
}

void ordenar_segmentos(){
    bool segmento_anterior(segmento* segmento_antes, segmento* segmento_despues) {
        return segmento_antes->base < segmento_despues->base;
    }
	log_info(logger,"Comienzo a ordenar los segmentos");
    list_sort(segmentos, (void*) segmento_anterior);
	log_info(logger,"Segmentos ordenados");
    return;
}

void liberar_segmento(int base){
    for(int i = 0; i<list_size(segmentos);i++){
        segmento* x = list_get(segmentos, i);
        if(x->base == base) {
            x->libre = true;
            log_info(logger, "Se elimina el segmento con base %d", x->base);
        }
    }
    ordenar_segmentos();
}

// void liberar_pagina(int base){
//     for(int i = 0; i<list_size(paginas);i++){
//         pagina* x = list_get(paginas, i);
//         if(x->base == base) {
//             x->libre = true;
//             log_info(logger, "Se elimina la página con base %d", x->base);
//         }
//     }
//     ordenar_segmentos();
// }


void compactacion(){
    log_debug(logger, "Se comienza la compactacion");
    int size = list_size(segmentos);
    for(int i=0; i<size;i++){
        segmento* segmento_libre = list_get(segmentos, i);
        if(segmento_libre->libre){
            for(int z = i+1; z < size; z++){
                segmento* segmento_ocupado = list_get(segmentos, z);
                if(!segmento_ocupado->libre){

                    // Movemos primero la memoria real
                    /*memcpy(TAMANIO_MEMORIA + segmento_libre->base,
                           segmento_ocupado->mensaje->puntero_a_memoria,
                           segmento_ocupado->mensaje->tam);
                    segmento_ocupado->mensaje->puntero_a_memoria = TAMANIO_MEMORIA + segmento_libre->base;
					*/

                    // Despues acomodamos las estrucuras
                    segmento_ocupado->base = segmento_libre->base;
                    segmento_libre->base += segmento_ocupado->tam;

                    ordenar_segmentos();
                    unificar_segmentos_libres();
                    size = list_size(segmentos);
                    break;
                }
            }
        }
    }
    return;
}

// Recorro la tabla, si encuentro dos segmentos libres consecutivos los uno
void unificar_segmentos_libres(){
    int size = list_size(segmentos);
    for(int i=0; i<size-1; i++){

        segmento* una_segmento = list_get(segmentos, i);
        segmento* siguiente_segmento = list_get(segmentos, i + 1);

        if (una_segmento->libre && siguiente_segmento->libre){
            una_segmento->tam += siguiente_segmento->tam;
            list_remove(segmentos, i+1);
            free(siguiente_segmento);
            size = list_size(segmentos);
            i = 0;
        }
    }
    return;
}

segmento* crear_segmento(int base, int tam, bool libre){
    segmento* nuevo_segmento = malloc(sizeof(segmento));
    nuevo_segmento->base = base;
    nuevo_segmento->tam = tam;
    nuevo_segmento->libre = libre;

    return nuevo_segmento;
}


pagina* crear_pagina(int base, bool libre){
    pagina* nueva_pagina = malloc(sizeof(pagina));
    nueva_pagina->base = base;
    nueva_pagina->libre = libre;

    return nueva_pagina;
}



segmento* buscar_segmento_libre(int tam){
	if (strcmp(CRITERIO_SELECCION, "FF") == 0) {
        log_debug(logger, "Empieza busqueda FirstFit");
        return first_fit(tam);
    } else if (strcmp(CRITERIO_SELECCION, "BF") == 0) {
        log_debug(logger, "Empieza busqueda BestFit");
        return best_fit(tam);
    } else {
        log_error(logger, "Metodo de asignacion desconocido");
        exit(EXIT_FAILURE);
    }
}

segmento* first_fit(int tam){
    int size = list_size(segmentos);
    for(int i=0; i<size; i++){
        segmento* x = list_get(segmentos, i);
        if(x->libre == true && tam <= x->tam){
            log_info(logger, "Segmento libre encontrado (base: %d)", x->base);
            return x;
        }
    }
    log_warning(logger, "No se encontró segmento");
    return NULL;
}

segmento* best_fit(int tam){
	//TODO: Se puede mejorar la algoritmia, ahora recorre todo 2 veces, podria ir seleccionando al mejor candidato mientras recorre la primera vez
    int size = list_size(segmentos);
    t_list* candidatos = list_create();
    for(int i=0; i<size; i++){
        segmento* x = list_get(segmentos, i);
        if(x->libre == true && tam <= x->tam){
            log_info(logger, "Segmento libre con suficiente espacio encontrado (base: %d)", x->base);
            if(tam == x->tam){
				log_info(logger, "Mejor segmento encontrado (base:%d)", x->base);
				return x;
			}
            list_add(candidatos, x);
        }
    }
    log_debug(logger, "Buscando el mejor segmento");
    int candidatos_size = list_size(candidatos);
    if(candidatos_size != 0){
        segmento* best_fit;
        int best_fit_diff = 999999;
        for(int i=0; i<candidatos_size; i++){
            segmento* y = list_get(candidatos, i);
            int diff = y->tam - tam;
            if(best_fit_diff > diff){
                best_fit_diff = diff;
                best_fit = y;
            }
        }
        log_info(logger, "Mejor segmento encontrado (base:%d)", best_fit->base);
        return best_fit;
    }else{
        log_warning(logger, "No hay segmentos disponibles");
        return NULL;
    }
}

segmento* asignar_segmento(int tam){
	segmento* segmento_libre = buscar_segmento_libre(tam);
	if(segmento_libre != NULL){
		//Si el segmento es del tamaño justo, no tengo que reordenar
		if(segmento_libre->tam == tam){
			segmento_libre->libre = false;
			log_info(logger,"Segmento asignado (base:%d)", segmento_libre->base);
			return segmento_libre;
		}
		//Si no tengo que dividir el segmento
		else{
			segmento* nuevo_segmento = crear_segmento(segmento_libre->base,tam,false);
			list_add(segmentos,nuevo_segmento);
			segmento_libre->base += tam;
			segmento_libre->tam -= tam;
			log_info(logger,"Segmento asignado (base:%d)", nuevo_segmento->base);
			//Ordeno los segmentos por base ascendente
			ordenar_segmentos();

			return nuevo_segmento;
		}
	}else{
		//TODO
	}
}

tabla_segmentos* crear_tabla_segmentos(uint32_t pid){
	tabla_segmentos* nueva_tabla = malloc(sizeof(tabla_segmentos));
	nueva_tabla->segmentos_tcb = list_create();
	list_add(indices,crear_indice(pid, (void*) nueva_tabla));
	return nueva_tabla;
}

tabla_paginas* crear_tabla_paginas(uint32_t pid){
	tabla_paginas* nueva_tabla = malloc(sizeof(tabla_paginas));
	nueva_tabla->paginas = list_create();
	list_add(indices,crear_indice(pid, (void*) nueva_tabla));
	return nueva_tabla;
}

void iniciar_memoria(){
	memoria_principal = malloc(TAMANIO_MEMORIA);
	indices = list_create();
	if(strcmp(ESQUEMA_MEMORIA,"SEGMENTACION")==0){
		log_info(logger,"Se inicia memoria con esquema se SEGMENTACION");
		segmentos = list_create();
		segmento* segmento_principal = crear_segmento(0,TAMANIO_MEMORIA,true);
		list_add(segmentos,segmento_principal);
	}else if(strcmp(ESQUEMA_MEMORIA,"PAGINACION")==0){
		log_info(logger,"Se inicia memoria con esquema de PAGINACION");
		paginas = list_create();

		int cantidad_paginas = TAMANIO_MEMORIA/TAMANIO_PAGINA;
		
		for(int i=0; i < cantidad_paginas ; i++) {
			pagina* pagina = crear_pagina(TAMANIO_PAGINA * i, true);

			list_add(paginas,pagina);
		}

	}else{
		log_error(logger,"Esquema de memoria desconocido");
		exit(EXIT_FAILURE);
	}
}

void* buscar_tabla(int pid){
	bool criterio(void* un_indice){
		indice_tabla* indice = (indice_tabla*) un_indice;
		return indice->pid == pid;
	}
	
	log_debug(logger,"Se comienza la busqueda de tabla de pid: %d",pid);

	indice_tabla* indice = (indice_tabla*) list_find(indices, criterio);
	if(indice != NULL){
		log_debug(logger,"Tabla encontrada, pid: %d",indice->pid);
		return indice->tabla;
	}
	log_debug(logger,"Tabla no encontrada");
	return NULL;
}

void test_segmentos(){
	segmento* seg = asignar_segmento(sizeof(char[2]));
	segmento* seg2 = asignar_segmento(sizeof(char));
	segmento* seg3 = asignar_segmento(sizeof(char));
	liberar_segmento(0);
	liberar_segmento(3);
	compactacion();
	segmento* seg4 = asignar_segmento(sizeof(char));
	print_segmentos_info();
}

void test_tabla_segmentos(){
	log_info(logger,"Se crea la tabla de segmentos pid: 1");
	tabla_segmentos* tabla1 = crear_tabla_segmentos(1);
	log_info(logger,"Se crea segmento para pcb 1");
	tabla1->segmento_pcb = asignar_segmento(8);
	log_info(logger,"Segmento para pcb creado 1, base: %d", tabla1->segmento_pcb->base);
	log_info(logger,"Se crea segmento para tareas 1");
	tabla1->segmento_tareas = asignar_segmento(45);
	log_info(logger,"Segmento para tareas creado 1, base: %d", tabla1->segmento_tareas->base);

	list_add(indices,crear_indice(1, (void*) tabla1));

	log_info(logger,"Se crea la tabla de segmentos pid: 2");
	tabla_segmentos* tabla2 = crear_tabla_segmentos(2);
	log_info(logger,"Se crea segmento para pcb 2");
	tabla2->segmento_pcb = asignar_segmento(8);
	log_info(logger,"Segmento para pcb creado 2, base: %d", tabla2->segmento_pcb->base);
	log_info(logger,"Se crea segmento para tareas 2");
	tabla2->segmento_tareas = asignar_segmento(21);
	log_info(logger,"Segmento para tareas creado 2, base: %d", tabla2->segmento_tareas->base);

	list_add(indices,crear_indice(2, (void*) tabla2));
	
	tabla_segmentos* t_seg = buscar_tabla(2);
	log_info(logger,"Base del segmento de tareas de la tabla 2: %d",t_seg->segmento_tareas->base);

	print_segmentos_info();
	print_tablas_segmentos_info();
}

void print_segmentos_info() {
    int size = list_size(segmentos);
    printf("\n<------ SEGMENTOS -----------\n");
    for(int i=0; i<size; i++) {
        segmento *s = list_get(segmentos, i);
        printf("base: %d, tam: %d, libre: %s\n", s->base, s->tam, s->libre ? "true" : "false");
    }
    printf("------------------->\n");
}

void print_tablas_segmentos_info(){
	int size = list_size(indices);
	printf("\n<----- TABLAS DE SEGMENTOS ---------------------\n");
	for(int i = 0; i < size; i++) {
        indice_tabla* index = list_get(indices, i);
		tabla_segmentos* tabla = (tabla_segmentos*) index->tabla;
		printf("Tabla pid: %d\n", index->pid);
		printf("\t Segmento PCB:\n");
		printf("\t\t base: %d\n",tabla->segmento_pcb->base);
		printf("\t\t tam: %d\n",tabla->segmento_pcb->tam);
		printf("\t Segmento Tarea:\n");
		printf("\t\t base: %d\n",tabla->segmento_tareas->base);
		printf("\t\t tam: %d\n",tabla->segmento_tareas->tam);
    }
	printf("------------------>\n");
}
