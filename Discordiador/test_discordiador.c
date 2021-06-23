/*
 * test_discordiador.c
 *
 *  Created on: 22 jun. 2021
 *      Author: utnso
 */

#include "test_discordiador.h"


void test_iniciar_planificacion(){
    test_iniciar_patota();
    sleep(1); // tiempo para que inicie el hilo.
    iniciar_planificacion();
}


// borrar?
void enlistar_algun_tripulante(){

    if (!list_is_empty(lista_tripulantes_new)){
        t_tripulante* tripulante_a_ready = monitor_lista_dos_parametros(sem_lista_new, (void*) list_remove, lista_tripulantes_new, (void*) 0);
        monitor_cola_push(sem_cola_ready, cola_tripulantes_ready, tripulante_a_ready);
        pedir_tarea_a_mi_ram_hq(tripulante_a_ready->TID, socket_a_mi_ram_hq);

        t_estructura* respuesta = recepcion_y_deserializacion(socket_a_mi_ram_hq);

        if(respuesta->codigo_operacion == TAREA){
            tripulante_a_ready->tarea = *respuesta->tarea;
        }
        else if (respuesta->codigo_operacion == FALLO){
            log_info(logger, "No se recibio ninguna tarea.\n Codigo de error: FALLO\n");
        }
        else{
            log_info(logger, "Error desconocido, no se recibio ninguna tarea.\n");
        }

        tripulante_a_ready->estado_tripulante = estado_tripulante[READY];
    }
    else {
        log_info(logger, "No hay ningun tripulante listo para ser enlistado.\n");
    }
}


void test_serializar_tcb(){

    t_TCB tcb;

    tcb.TID = 10001;
    tcb.estado_tripulante = estado_tripulante[NEW];
    tcb.coord_x = 1;
    tcb.coord_y = 1;
    tcb.siguiente_instruccion = 0;
    tcb.puntero_a_pcb = 0;

    log_info(logger, "TCB antes de serializar:\n");
    log_info(logger, "Tripulante %i, estado: %c pos: %i %i\n", (int)tcb.TID, (char) tcb.estado_tripulante,(int) tcb.coord_x, (int) tcb.coord_y);

    t_buffer* b = serializar_tcb(tcb);
    t_TCB* tcb_deserializado = deserializar_tcb(b);

    log_info(logger, "TCB despues de serializar:\n");
    log_info(logger, "Tripulante %i, estado: %c pos: %i %i\n", (int)tcb_deserializado->TID, (char) tcb_deserializado->estado_tripulante,(int) tcb_deserializado->coord_x, (int) tcb_deserializado->coord_y);

}

void test_iniciar_patota(){

    iniciar_patota("INICIAR_PATOTA 1 Random.ims 1|1");
    // iniciar_patota("INICIAR_PATOTA 2 Oxigeno.ims 1|1");
    // iniciar_patota("INICIAR_PATOTA 5 Random.ims 1|1 2|2 3|3");

}

void test_listar_tripulantes(){

    test_iniciar_patota();
    sleep(1);
    listar_tripulantes();

}

void test_nuevo_pid(){
    int i = 0;
    list_add(lista_pids, (void*) 1);
    list_add(lista_pids, (void*) 3);

    while (i<10){
        printf("%d", nuevo_pid());
        i++;
    }

    // Resultado:
    // Los PIDS en la lista son 1 y 3, así que me debe ingresar y printear los primeros 10 pids que no sean esos.
}

void test_enlistar_algun_tripulante(){
    t_TCB* tcb = crear_puntero_tcb(0, 5, "8a9");
    printf("Tripulante. pos: %i %i, tid: %i estado %c \n", (int) tcb->coord_x, (int) tcb->coord_y, (int) tcb->TID, tcb->estado_tripulante);

    monitor_lista_dos_parametros(sem_lista_new, (void*) list_add, lista_tripulantes_new, tcb);
    enlistar_algun_tripulante();

    t_TCB* prueba = monitor_cola_pop(sem_cola_ready, cola_tripulantes_ready);
    printf("Tripulante. pos: %i %i, tid: %i estado %c \n", (int) prueba->coord_x, (int) prueba->coord_y, (int) prueba->TID, prueba->estado_tripulante);

}

void test_serializar_tarea(){

    t_tarea* t = crear_tarea("GENERAR_OXIGENO 12;2;3;5");

    printf("Largo nombre: %i\n", t->largo_nombre);
    printf("Nombre: %s\n", t->nombre);
    printf("Parametros: %i\n", t->parametro);
    printf("Cordenada en X: %i\n", t->coord_x);
    printf("Cordenada en Y: %i\n", t->coord_y);
    printf("Duracion: %i\n", t->duracion);

    t_buffer* b = serializar_tarea(*t);
    t_tarea* t2 = deserializar_tarea(b);

    printf("Largo nombre: %i\n", t2->largo_nombre);
    printf("Nombre: %s\n", t2->nombre);
    printf("Parametros: %i\n", t2->parametro);
    printf("Cordenada en X: %i\n", t2->coord_x);
    printf("Cordenada en Y: %i\n", t2->coord_y);
    printf("Duracion: %i\n", t2->duracion);

}

void test_serializar_tripulante(){

    t_tripulante un_tripulante;

    un_tripulante.TID = 10001;
    un_tripulante.estado_tripulante = estado_tripulante[NEW];
    un_tripulante.coord_x = 1;
    un_tripulante.coord_y = 1;

    log_info(logger, "Tripulante antes de serializar:\n");
    log_info(logger, "Tripulante %i, estado: %c pos: %i %i\n", (int)un_tripulante.TID, (char) un_tripulante.estado_tripulante,(int) un_tripulante.coord_x, (int) un_tripulante.coord_y);

    t_buffer* b = serializar_tripulante(un_tripulante);
    t_tripulante* un_tripulante_deserializado = deserializar_tripulante(b);

    log_info(logger, "Tripulante despues de serializar:\n");
    log_info(logger, "Tripulante %i, estado: %c pos: %i %i\n", (int)un_tripulante_deserializado->TID, (char) un_tripulante_deserializado->estado_tripulante,(int) un_tripulante_deserializado->coord_x, (int) un_tripulante_deserializado->coord_y);

}

