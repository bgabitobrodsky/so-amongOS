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
    int o = 1;
    int u = 3;

    list_add(lista_pids, (void*) o);
    list_add(lista_pids, (void*) u);

    while (i<10){
    	log_info(logger, "%d", nuevo_pid());
        i++;
    }

    // Resultado:
    // Los PIDS en la lista son 1 y 3, así que me debe ingresar y printear los primeros 10 pids que no sean esos.
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

