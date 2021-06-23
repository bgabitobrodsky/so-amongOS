#include "serializar_y_envio.h"

int CUmain_serializacion() {
	CU_initialize_registry();

	CU_pSuite serializacion = CU_add_suite("Suite de serializacion", NULL, NULL);
	CU_add_test(serializacion, "serializar vacio exitosamente",  test_serializar_vacio);

	CU_pSuite desserializacion = CU_add_suite("Suite de desserializacion", NULL, NULL);
	CU_add_test(desserializacion, "desserializar tcb exitosamente",  test_desserializar_tcb);
	CU_add_test(desserializacion, "desserializar tarea exitosamente",  test_desserializar_tarea);

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();

	return CU_get_error();
}

void test_serializar_vacio() {
	t_buffer* buffer_vacio = serializar_vacio();
	CU_ASSERT_EQUAL(buffer_vacio->tamanio_estructura, 0);
	CU_ASSERT_EQUAL(buffer_vacio->estructura, NULL);
}

void test_desserializar_tcb() {
	t_PCB pcb = pcb_generico();
	t_TCB un_tcb = tcb_generico();
	t_buffer* buffer_tcb = serializar_tcb(un_tcb);
	t_TCB* un_tcb_desserialziado = deserializar_tcb(buffer_tcb);

	CU_ASSERT_EQUAL(un_tcb_desserialziado->TID, 1);
	CU_ASSERT_EQUAL(un_tcb_desserialziado->estado_tripulante, 'N');
	CU_ASSERT_EQUAL(un_tcb_desserialziado->coord_x, 2);
	CU_ASSERT_EQUAL(un_tcb_desserialziado->coord_y, 3);
	CU_ASSERT_EQUAL(un_tcb_desserialziado->siguiente_instruccion, 0);
	CU_ASSERT_EQUAL(un_tcb_desserialziado->puntero_a_pcb, &pcb); //TODO rompe

	son_tcb_iguales(*un_tcb_desserialziado, un_tcb);
}

void test_desserializar_tarea() {
	t_tarea una_tarea = tarea_generica();
	t_buffer* buffer_tarea_serializada = serializar_tarea(una_tarea);
	t_tarea* una_tarea_desseraializada = deserializar_tarea(buffer_tarea_serializada);

	son_tareas_iguales(una_tarea, *una_tarea_desseraializada);
}



/////////////////////// GENERICOS////////////////////////////////////////////////
t_PCB pcb_generico() {
	return crear_pcb_(1, "No importa");
}

t_TCB tcb_generico() {
	t_PCB pcb = pcb_generico();
	return crear_tcb_(1, 2, 3, 'N', &pcb);
}

t_tarea tarea_generica() {
	return crear_tarea_(2, 3, 4, "Hola, soy una tarea", 5);
}

//////////////////////// UTILES ///////////////777777//////////////////
t_PCB crear_pcb_(int pid, char* direccion_tareas) {
	t_PCB pcb;
	pcb.PID = (uint32_t) pid;
	pcb.direccion_tareas = (uint32_t) direccion_tareas;

	return pcb;
}

t_TCB crear_tcb_(int tid, int coord_x, int coord_y, char estado_tripulante, t_PCB* puntero_a_pcb) {
	t_TCB tcb;
	tcb.TID = (uint32_t) tid;
	tcb.coord_x = (uint32_t) coord_x;
	tcb.coord_y = (uint32_t) coord_y;
	tcb.estado_tripulante = estado_tripulante;
	tcb.puntero_a_pcb = (uint32_t) puntero_a_pcb;
	tcb.siguiente_instruccion = 0;

	return tcb;
}

t_tarea crear_tarea_(int coord_x, int coord_y, int duracion, char* nombre, int parametro) {
	t_tarea tarea;
	tarea.coord_x = (uint32_t) coord_x;
	tarea.coord_y = (uint32_t) coord_y;
	tarea.duracion = (uint32_t) duracion;
	tarea.nombre = nombre;
	tarea.largo_nombre = (uint32_t) strlen(nombre)+1;
	tarea.parametro = (uint32_t) parametro;

	return tarea;
}

void son_pcb_iguales(t_PCB pcb_1, t_PCB pcb_2) {
	CU_ASSERT_EQUAL(pcb_1.PID, pcb_2.PID);
	CU_ASSERT_EQUAL(pcb_1.direccion_tareas, pcb_2.direccion_tareas);
}

void son_tcb_iguales(t_TCB tcb_1, t_TCB tcb_2) {
	CU_ASSERT_EQUAL(tcb_1.TID, tcb_2.TID);
	CU_ASSERT_EQUAL(tcb_1.estado_tripulante, tcb_2.estado_tripulante);
	CU_ASSERT_EQUAL(tcb_1.coord_x, tcb_2.coord_x);
	CU_ASSERT_EQUAL(tcb_1.coord_y, tcb_2.coord_y);
	CU_ASSERT_EQUAL(tcb_1.siguiente_instruccion, tcb_2.siguiente_instruccion);
	CU_ASSERT_EQUAL(tcb_1.puntero_a_pcb, tcb_2.puntero_a_pcb);
}

void son_tcb_iguales2(t_TCB* tcb_1, t_TCB tcb_2) {
	CU_ASSERT_EQUAL(tcb_1->TID, tcb_2.TID);
	CU_ASSERT_EQUAL(tcb_1->estado_tripulante, tcb_2.estado_tripulante);
	CU_ASSERT_EQUAL(tcb_1->coord_x, tcb_2.coord_x);
	CU_ASSERT_EQUAL(tcb_1->coord_y, tcb_2.coord_y);
	CU_ASSERT_EQUAL(tcb_1->siguiente_instruccion, tcb_2.siguiente_instruccion);
	CU_ASSERT_EQUAL(tcb_1->puntero_a_pcb, tcb_2.puntero_a_pcb);
}

void son_tareas_iguales(t_tarea tarea_1, t_tarea tarea_2) {
	CU_ASSERT_EQUAL(tarea_1.coord_x, tarea_2.coord_x);
	CU_ASSERT_EQUAL(tarea_1.coord_y, tarea_2.coord_y);
	CU_ASSERT_EQUAL(tarea_1.duracion, tarea_2.duracion);
	CU_ASSERT_EQUAL(tarea_1.nombre, tarea_2.nombre); //TODO rompe
	CU_ASSERT_EQUAL(tarea_1.largo_nombre, tarea_2.largo_nombre);
	CU_ASSERT_EQUAL(tarea_1.parametro, tarea_2.parametro);
}
