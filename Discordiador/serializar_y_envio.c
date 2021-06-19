#include "serializar_y_envio.h"

int CUmain() {
	CU_initialize_registry();

	CU_pSuite serializacion = CU_add_suite("Suite de serializado", NULL, NULL);
	CU_add_test(serializacion, "crear tcb exitosamente",  test_crear_tcb);
	CU_add_test(serializacion, "serializar tcb exitosamente",  test_serializar_tcb);
	//CU_add_test(serializacion, "serializar tarea exitosamente",  test_serializar_tarea);
	CU_add_test(serializacion, "serializar vacio exitosamente",  test_serializar_vacio);

	CU_pSuite desserializacion = CU_add_suite("Suite de desserializado", NULL, NULL);
	CU_add_test(desserializacion, "desserializar tcb exitosamente",  test_desserializar_tcb);
	CU_add_test(desserializacion, "desserializar tarea exitosamente",  test_desserializar_tarea);

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();

	return CU_get_error();
}

void test_crear_tcb() {
	t_PCB* pcb = malloc(sizeof(t_PCB));
	pcb = crear_pcb("No importa");
	t_TCB un_tcb = crear_tcb(pcb, 0, "0|0");
	CU_ASSERT_EQUAL(un_tcb.TID, 0);
	CU_ASSERT_EQUAL(un_tcb.coord_x, 0);
	CU_ASSERT_EQUAL(un_tcb.coord_y, 0);
	CU_ASSERT_EQUAL(un_tcb.estado_tripulante, 'N');
	//CU_ASSERT_EQUAL(un_tcb.puntero_a_pcb, &pcb);
	//TODO CU_ASSERT_EQUAL(un_tcb->siguiente_instruccion, );
	free(pcb);
}

void test_serializar_tcb() {
	t_PCB* pcb = crear_pcb("No importa");
	t_TCB un_tcb = crear_tcb(pcb, 0, "1|2");
	t_buffer* buffer_tcb = serializar_tcb(un_tcb);

	void* estructura = malloc(sizeof(un_tcb));
	int desplazamiento = 0;

	int* tid;
	char* estado_tripulante;
	int* coord_x;
	int* coord_y;
	uint32_t* siguiente_instruccion;
	uint32_t* puntero_a_pcb;
	memcpy(&tid, buffer_tcb->estructura, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	memcpy(&estado_tripulante, buffer_tcb->estructura + desplazamiento, sizeof(char));
	desplazamiento += sizeof(char);
	memcpy(&coord_x, buffer_tcb->estructura + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(&coord_y, buffer_tcb->estructura + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    //TODO siguiente_instruccion memcpy(siguiente_instruccion, buffer_tcb->estructura + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(&siguiente_instruccion, buffer_tcb->estructura + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(&puntero_a_pcb, buffer_tcb->estructura + desplazamiento, sizeof(uint32_t));


	CU_ASSERT_EQUAL(buffer_tcb->tamanio_estructura, sizeof(t_TCB));
	//CU_ASSERT_EQUAL(buffer_tcb->estructura, un_tcb); //TODO
	CU_ASSERT_EQUAL(*tid, 0);
	CU_ASSERT_EQUAL(*estado_tripulante, 'N');
	CU_ASSERT_EQUAL(*coord_x, 1);
	CU_ASSERT_EQUAL(*coord_y, 2);
	//TODO CU_ASSERT_EQUAL(siguiente_instruccion, ???);
	CU_ASSERT_EQUAL(*puntero_a_pcb, &pcb);
	free(estructura);
}

void test_serializar_tarea() {
	t_tarea tarea = crear_tarea_();
	t_buffer* buffer_tarea_serializada = serializar_tarea(tarea);

	int desplazamiento = 0;

	int* coord_x;
	int* coord_y;
	int* duracion;
	char* nombre;
	int* nombre_largo;
	uint32_t* parametro;

    memcpy(&coord_x, buffer_tarea_serializada->estructura + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(&coord_y, buffer_tarea_serializada->estructura + desplazamiento, sizeof(char));
    desplazamiento += sizeof(char);
    memcpy(&duracion, buffer_tarea_serializada->estructura + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(&nombre, buffer_tarea_serializada->estructura + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(&nombre_largo, buffer_tarea_serializada->estructura + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(&parametro, buffer_tarea_serializada->estructura + desplazamiento, sizeof(uint32_t));

    CU_ASSERT_EQUAL(*coord_x, 2);
	CU_ASSERT_EQUAL(*coord_y, 3);
	CU_ASSERT_EQUAL(*duracion, 4);
	CU_ASSERT_EQUAL(*nombre, "Hola, soy una tarea");
	CU_ASSERT_EQUAL(*nombre_largo, 20);
	CU_ASSERT_EQUAL(*parametro, NULL);
}

void test_serializar_vacio() {
	t_buffer* buffer_vacio = serializar_vacio();
	CU_ASSERT_EQUAL(buffer_vacio->tamanio_estructura, 0);
	CU_ASSERT_EQUAL(buffer_vacio->estructura, NULL);
}

void test_desserializar_tcb() {
	t_PCB* pcb = crear_pcb("No importa");
	t_TCB un_tcb = crear_tcb(pcb, 0, "1|2");
	t_buffer* buffer_tcb = serializar_tcb(un_tcb);
	t_TCB* un_tcb_desserialziado = deserializar_tcb(buffer_tcb);

	CU_ASSERT_EQUAL(un_tcb_desserialziado->TID, 0);
	CU_ASSERT_EQUAL(un_tcb_desserialziado->estado_tripulante, 'N');
	CU_ASSERT_EQUAL(un_tcb_desserialziado->coord_x, 1);
	CU_ASSERT_EQUAL(un_tcb_desserialziado->coord_y, 2);
	//CU_ASSERT_EQUAL(un_tcb_desserialziado->siguiente_instruccion, ???);
	CU_ASSERT_EQUAL(un_tcb_desserialziado->puntero_a_pcb, &pcb);

	son_tcb_iguales(*un_tcb_desserialziado, un_tcb);
}

void test_desserializar_tarea() {
	t_tarea una_tarea = crear_tarea_();
	t_buffer* buffer_tarea_serializada = serializar_tarea(una_tarea);
	t_tarea* una_tarea_desseraializada = deserializar_tarea(buffer_tarea_serializada);

	son_tareas_iguales(una_tarea, *una_tarea_desseraializada);
}
