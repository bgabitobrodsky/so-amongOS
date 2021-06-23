#include "envio_y_recepcion.h"

#define IP_MI_RAM_HQ "127.0.0.1"
#define PUERTO_MI_RAM_HQ "25430"
#define IP_I_MONGO_STORE "127.0.0.1"
#define PUERTO_I_MONGO_STORE "5001"

int CUmain_envio_y_recepcion() {
    socket_a_mi_ram_hq = crear_socket_cliente(IP_MI_RAM_HQ, PUERTO_MI_RAM_HQ);
    socket_a_mongo_store = crear_socket_cliente(IP_I_MONGO_STORE, PUERTO_I_MONGO_STORE);
	CU_initialize_registry();

	CU_pSuite empaquetado_y_enviado = CU_add_suite("Suite de empaquetado y enviado", NULL, NULL);
	CU_add_test(empaquetado_y_enviado, "Empaquetar y enviar tcb exitosamente a RAM",  test_empaquetar_y_enviar_tcb_ram);
	/*CU_add_test(empaquetado_y_enviado, "serializar tcb exitosamente",  test_serializar_tcb);
	//CU_add_test(empaquetado_y_enviado, "serializar tarea exitosamente",  test_serializar_tarea);
	CU_add_test(empaquetado_y_enviado, "serializar vacio exitosamente",  test_serializar_vacio);

	CU_pSuite desserializacion = CU_add_suite("Suite de desserializado", NULL, NULL);
	CU_add_test(desserializacion, "desserializar tcb exitosamente",  test_desserializar_tcb);
	CU_add_test(desserializacion, "desserializar tarea exitosamente",  test_desserializar_tarea);*/

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();

	return CU_get_error();
}

void test_empaquetar_y_enviar_tcb_ram() { //TODO habría que implementar funciones especificas para testear en cada módulo. Se puede agregar casos de testeos al enum o hacer un if(testeo){... }else {código}
	t_PCB pcb = pcb_generico();
	t_TCB un_tcb = tcb_generico();
	t_buffer* buffer_tcb = serializar_tcb(un_tcb);
	empaquetar_y_enviar(buffer_tcb , RECIBIR_TCB, socket_a_mi_ram_hq);
	t_estructura* intermediario = recepcion_y_deserializacion(socket_a_mi_ram_hq);

	CU_ASSERT_EQUAL(intermediario->codigo_operacion, RECIBIR_TCB);
	son_tcb_iguales2(intermediario->tcb, un_tcb);
}
/*
void test_empaquetar_y_enviar_tarea() { //TODO
	t_tarea tarea = crear_tarea_();
	t_buffer* buffer_tarea = serializar_tarea(tarea);
	empaquetar_y_enviar(buffer_tarea , TAREA, socket);
	t_estructura* intermediario = recepcion_y_deserializacion(socket);
	CU_ASSERT_EQUAL(intermediario->codigo_operacion, TAREA);
	son_tareas_iguales(intermediario->tarea, tarea);
}

void test_empaquetar_y_enviar_sabotaje() { //TODO
	t_buffer* buffer_;
	empaquetar_y_enviar(buffer_ , SABOTAJE, socket);
	t_estructura* intermediario = recepcion_y_deserializacion(socket);
	CU_ASSERT_EQUAL(intermediario->codigo_operacion, SABOTAJE);
}

void test_empaquetar_y_enviar_mensaje() { //TODO

	t_buffer* buffer_;
	empaquetar_y_enviar(buffer_ , MENSAJE, socket);
	t_estructura* intermediario = recepcion_y_deserializacion(socket);
	CU_ASSERT_EQUAL(intermediario->codigo_operacion, MENSAJE);
}

void test_empaquetar_y_enviar_pedir_tarea() { //TODO
	t_buffer* buffer_;
	empaquetar_y_enviar(buffer_ , PEDIR_TAREA, socket);
	t_estructura* intermediario = recepcion_y_deserializacion(socket);
	CU_ASSERT_EQUAL(intermediario->codigo_operacion, PEDIR_TAREA);
}

void test_empaquetar_y_enviar_codigo_tarea() { //TODO
	t_buffer* buffer_;
	empaquetar_y_enviar(buffer_ , COD_TAREA, socket);
	t_estructura* intermediario = recepcion_y_deserializacion(socket);
	CU_ASSERT_EQUAL(intermediario->codigo_operacion, COD_TAREA);
}

void test_empaquetar_y_enviar_recepcion() { //TODO
	t_buffer* buffer_;
	empaquetar_y_enviar(buffer_ , RECEPCION, socket);
	t_estructura* intermediario = recepcion_y_deserializacion(socket);
	CU_ASSERT_EQUAL(intermediario->codigo_operacion, RECEPCION);
}

void test_empaquetar_y_enviar_desconexion() { //TODO
	t_buffer* buffer_;
	empaquetar_y_enviar(buffer_ , DESCONEXION, socket);
	t_estructura* intermediario = recepcion_y_deserializacion(socket);
	CU_ASSERT_EQUAL(intermediario->codigo_operacion, DESCONEXION);
}*/
