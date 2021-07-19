/*

 Funcionalidad de paquetes, cortesia de Nico

*/

#include "paquetes.h"

// Serializa un struct tcb a un buffer
// En esta funcion se pone el comportamiento comun de la serializacion
t_buffer* serializar_tcb(t_TCB tcb) {

    t_buffer* buffer = malloc(sizeof(uint32_t) + sizeof(uint32_t)*5 + sizeof(char));
    buffer->tamanio_estructura = sizeof(uint32_t)*5 + sizeof(char);  // Se le da el tamanio del struct del parametro
    // estructura NO se debe liberar!!
    void* estructura = malloc(buffer->tamanio_estructura); // Se utiliza intermediario
    int desplazamiento = 0; // Desplazamiento para calcular que tanto tengo que correr para que no se sobrepisen cosas del array estructura

    memcpy(estructura + desplazamiento, &tcb.TID, sizeof(uint32_t)); // Se copia y pega al array estructura ordenado:
    desplazamiento += sizeof(uint32_t);
    memcpy(estructura + desplazamiento, &tcb.estado_tripulante, sizeof(char));
    desplazamiento += sizeof(char);
    memcpy(estructura + desplazamiento, &tcb.coord_x, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(estructura + desplazamiento, &tcb.coord_y, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(estructura + desplazamiento, &tcb.siguiente_instruccion, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(estructura + desplazamiento, &tcb.puntero_a_pcb, sizeof(uint32_t));

    buffer->estructura = estructura; // Se iguala el buffer al intermediario

    return buffer;
}

// Serializa un struct tarea a un buffer
t_buffer* serializar_tarea(t_tarea tarea) {

    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->tamanio_estructura = (5 * sizeof(uint32_t)) + tarea.largo_nombre + 1;

    void* estructura = malloc((buffer->tamanio_estructura));
    int desplazamiento = 0;
    
    memcpy(estructura + desplazamiento, &tarea.largo_nombre, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(estructura + desplazamiento, tarea.nombre, (tarea.largo_nombre + 1));
    desplazamiento += (tarea.largo_nombre + 1);
    memcpy(estructura + desplazamiento, &tarea.parametro, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(estructura + desplazamiento, &tarea.coord_x, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(estructura + desplazamiento, &tarea.coord_y, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(estructura + desplazamiento, &tarea.duracion, sizeof(uint32_t));

    buffer->estructura = estructura;

    return buffer;
}

t_buffer* serializar_posicion(t_posicion posicion) { 
	t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->tamanio_estructura = (2 * sizeof(uint32_t));

    void* estructura = malloc((buffer->tamanio_estructura));
    int desplazamiento = 0;
    
    memcpy(estructura + desplazamiento, &posicion.coord_x, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(estructura + desplazamiento, &posicion.coord_y, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    buffer->estructura = estructura;
	return buffer;
}

// Nombre medio raro, serializa una cantidad de las cosas del filesystem (int, puede ser negativo)
t_buffer* serializar_cantidad(int cantidad) {
    t_buffer* buffer = malloc((sizeof(t_buffer)));
    buffer->tamanio_estructura = sizeof(int);

    void* estructura = malloc((buffer->tamanio_estructura));

    memcpy(estructura, &cantidad, sizeof(int));

    buffer->estructura = estructura;

    return buffer;
}

t_buffer* serializar_entero(uint32_t numero) {
    t_buffer* buffer = malloc((sizeof(t_buffer)));
    buffer->tamanio_estructura = sizeof(int);

    void* estructura = malloc((buffer->tamanio_estructura));

    memcpy(estructura, &numero, sizeof(int));

    buffer->estructura = estructura;

    return buffer;
}

t_buffer* serializar_tripulante(t_tripulante tripulante) {

    t_buffer* buffer = malloc(sizeof(uint32_t) + sizeof(uint32_t)*3 + sizeof(char));
    buffer->tamanio_estructura = sizeof(uint32_t)*3 + sizeof(char);
    void* estructura = malloc(buffer->tamanio_estructura);
    int desplazamiento = 0;

    memcpy(estructura + desplazamiento, &tripulante.TID, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(estructura + desplazamiento, &tripulante.estado_tripulante, sizeof(char));
    desplazamiento += sizeof(char);
    memcpy(estructura + desplazamiento, &tripulante.coord_x, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(estructura + desplazamiento, &tripulante.coord_y, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    buffer->estructura = estructura;

    return buffer;
}

t_buffer* serializar_archivo_tareas(t_archivo_tareas texto_archivo) {

    t_buffer* buffer = malloc(sizeof(uint32_t) + sizeof(uint32_t)*2 + texto_archivo.largo_texto + 1);
    buffer->tamanio_estructura = sizeof(uint32_t)*2 + texto_archivo.largo_texto + 1;

    void* estructura = malloc(buffer->tamanio_estructura);
    int desplazamiento = 0;

    memcpy(estructura + desplazamiento, &texto_archivo.largo_texto, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(estructura + desplazamiento, texto_archivo.texto, (texto_archivo.largo_texto + 1));
    desplazamiento += (texto_archivo.largo_texto + 1);
    memcpy(estructura + desplazamiento, &texto_archivo.pid, sizeof(uint32_t));

    buffer->estructura = estructura;

    return buffer;
}

// Serializa un buffer "vacio" (dejo que lleve un char por las dudas) para envio de codigos nomas (podriamos cambiarlo a semaforos)
t_buffer* serializar_vacio() {

    t_buffer* buffer = malloc((sizeof(t_buffer)));

    buffer->tamanio_estructura = 0;

    buffer->estructura = NULL;

    return buffer;
}

// Recibe un buffer, un opcode y un socket a donde se enviara el paquete que se armara en la funcion, y se envia
// Usar con funciones de serializacion de arriba
// Usa funcion de socketes.h
void empaquetar_y_enviar(t_buffer* buffer, int codigo_operacion, int socket_receptor) {

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = codigo_operacion;
    paquete->buffer = buffer;

    int tamanio_mensaje = ((unsigned int) (buffer->tamanio_estructura)) + sizeof(int) + sizeof(uint32_t);

    void* mensaje = malloc(tamanio_mensaje);
    int desplazamiento = 0;

    memcpy(mensaje + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(mensaje + desplazamiento, &(paquete->buffer->tamanio_estructura), sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(mensaje + desplazamiento, paquete->buffer->estructura, paquete->buffer->tamanio_estructura);

    enviar_mensaje(socket_receptor, mensaje, tamanio_mensaje);

    free(mensaje);
    eliminar_paquete(paquete); 
}

// Recibe un opcode y un socket, y envia paquete con opcode only
// Usa funcion de socketes.h
void enviar_codigo(int codigo_operacion, int socket_receptor) {
    
    t_paquete* paquete = malloc(sizeof(t_paquete));
    t_buffer* buffer = serializar_vacio();
    paquete->codigo_operacion = codigo_operacion;
    paquete->buffer = buffer;

    int tamanio_mensaje = sizeof(int);
    void* mensaje = malloc(tamanio_mensaje);

    memcpy(mensaje, &(paquete->codigo_operacion), sizeof(int));

    enviar_mensaje(socket_receptor, mensaje, tamanio_mensaje);
    free(mensaje);
    eliminar_paquete(paquete);   
}


// Recibe con funcion de socketes.h y a partir del opcode desserializa y devuelve un struct que contiene lo recibido
// Se le pasa el socket donde se estan recibiendo mensajes
// IMPORTANTE, es necesario:
// -Ver las cosas dentro del struct estructura a ver que carajos se mando (puede que sea especifico por el socket y no haga falta)
// -Liberar el struct estructura luego de extraer las cosas
// TODO: Mejorable

t_estructura* recepcion_y_deserializacion(int socket_receptor) { 

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    t_estructura* intermediario = malloc(sizeof(t_estructura));

    int conexion_cerrada = recibir_mensaje(socket_receptor, &(paquete->codigo_operacion), sizeof(int));

    if(conexion_cerrada == 0){
    	intermediario->codigo_operacion = DESCONEXION;
    	free(paquete->buffer);
    	free(paquete);
    	return intermediario;
    }

    // If que maneja llegada de codigos de operacion unicamente (TODO CODIGO UNICO DEBE ESTAR DESPUES DE SABOTAJE)
    if (paquete->codigo_operacion > SABOTAJE && paquete->codigo_operacion >= 0) { // Lo del mayor a cero por si llega trash
        intermediario->codigo_operacion = paquete->codigo_operacion;
    	free(paquete->buffer);
    	free(paquete);
        return intermediario;
    }

    recibir_mensaje(socket_receptor, &(paquete->buffer->tamanio_estructura), sizeof(uint32_t));
    paquete->buffer->estructura = malloc(paquete->buffer->tamanio_estructura);
    recibir_mensaje(socket_receptor, paquete->buffer->estructura, paquete->buffer->tamanio_estructura);

    // Switch estructuras y cosas del fylesystem
    switch (paquete->codigo_operacion) {
    	case FIN_TAREA:
    	case INICIO_TAREA:
    	case CORRE_SABOTAJE:
    	case RESUELVE_SABOTAJE:
    	case MOVIMIENTO:
    	case ACTUALIZAR:
        case RECIBIR_TCB:
        	intermediario->codigo_operacion = paquete->codigo_operacion;
            intermediario->tcb = deserializar_tcb(paquete->buffer);
            break;

        case TAREA:
            intermediario->codigo_operacion = paquete->codigo_operacion;
            intermediario->tarea = deserializar_tarea(paquete->buffer);
            break;
        case BITACORA:
        case ARCHIVO_TAREAS:
        	intermediario->codigo_operacion = paquete->codigo_operacion;
            intermediario->archivo_tareas = deserializar_archivo_tareas(paquete->buffer);
            break;
        case PEDIR_BITACORA:
        case T_SIGKILL:
        case PEDIR_TAREA:
        	intermediario->codigo_operacion = paquete->codigo_operacion;
            memcpy(&(intermediario->tid), paquete->buffer->estructura, sizeof(uint32_t));
            break;

        case LISTAR_POR_PID:
        	intermediario->codigo_operacion = paquete->codigo_operacion;
            memcpy(&(intermediario->pid), paquete->buffer->estructura, sizeof(uint32_t));
            break;

        // Funcionan igual, mismo case en definitiva, queda asi para legibilidad, desserializa in situ porque es ezpz
        case OXIGENO:
        case COMIDA:
        case BASURA:
            intermediario->codigo_operacion = paquete->codigo_operacion;
            memcpy(&(intermediario->cantidad), paquete->buffer->estructura, sizeof(int));
            break;
        case SABOTAJE:
        case POSICION:
            intermediario->codigo_operacion = paquete->codigo_operacion;
            intermediario->posicion = deserializar_posicion(paquete->buffer);
            break;
    }

    eliminar_paquete(paquete);

    return intermediario;
}

// Pasa un struct buffer a un tcb
// Se explica deserializacion en esta funcion
t_TCB* deserializar_tcb(t_buffer* buffer) {

	t_TCB* tcb = malloc(sizeof(uint32_t)*5 + sizeof(char)); // Se toma tamaño de lo que sabemos que viene
    void* estructura = buffer->estructura; // Se inicializa intermediario 

    memcpy(&(tcb->TID), estructura, sizeof(uint32_t));
    estructura += sizeof(uint32_t);
    memcpy(&(tcb->estado_tripulante), estructura, sizeof(char));
    estructura += sizeof(char);
    memcpy(&(tcb->coord_x), estructura, sizeof(uint32_t));
    estructura += sizeof(uint32_t);
    memcpy(&(tcb->coord_y), estructura, sizeof(uint32_t));
    estructura += sizeof(uint32_t);
    memcpy(&(tcb->siguiente_instruccion), estructura, sizeof(uint32_t));
    estructura += sizeof(uint32_t);
    memcpy(&(tcb->puntero_a_pcb), estructura, sizeof(uint32_t));

    return tcb;
}

// Pasa un struct buffer a una tarea
t_tarea* deserializar_tarea(t_buffer* buffer) {

    t_tarea* tarea = malloc(sizeof(t_tarea));
    void* estructura = buffer->estructura;

    memcpy(&(tarea->largo_nombre), estructura, sizeof(uint32_t));
    estructura += sizeof(uint32_t);
    tarea->nombre = malloc(tarea->largo_nombre +1);
    memcpy(tarea->nombre, estructura, (tarea->largo_nombre +1));
    estructura += tarea->largo_nombre + 1;
    memcpy(&(tarea->parametro), estructura, sizeof(uint32_t));
    estructura += sizeof(uint32_t);
    memcpy(&(tarea->coord_x), estructura, sizeof(uint32_t));
    estructura += sizeof(uint32_t);
    memcpy(&(tarea->coord_y), estructura, sizeof(uint32_t));
    estructura += sizeof(uint32_t);
    memcpy(&(tarea->duracion), estructura, sizeof(uint32_t));

    return tarea;
}

t_posicion* deserializar_posicion(t_buffer* buffer) {
    t_posicion* posicion = malloc(sizeof(t_posicion));
    void* estructura = buffer->estructura;

    memcpy(&(posicion->coord_x), estructura, sizeof(uint32_t));
    estructura += sizeof(uint32_t);
    memcpy(&(posicion->coord_y), estructura, sizeof(uint32_t));
    estructura += sizeof(uint32_t);

    return posicion;
}

t_archivo_tareas* deserializar_archivo_tareas(t_buffer* buffer) {

	t_archivo_tareas* texto_archivo = malloc(sizeof(t_archivo_tareas));
    void* estructura = buffer->estructura;

    memcpy(&(texto_archivo->largo_texto), estructura, sizeof(uint32_t));
    estructura += sizeof(uint32_t);
    texto_archivo->texto = malloc(texto_archivo->largo_texto + 1);
    memcpy(texto_archivo->texto, estructura, (texto_archivo->largo_texto + 1));
    estructura += (texto_archivo->largo_texto + 1);
    memcpy(&(texto_archivo->pid), estructura, sizeof(uint32_t));

    return texto_archivo;
}

t_tripulante* deserializar_tripulante(t_buffer* buffer) {

	t_tripulante* un_tripulante = malloc(sizeof(uint32_t)*3 + sizeof(char));
    void* estructura = buffer->estructura;

    memcpy(&(un_tripulante->TID), estructura, sizeof(uint32_t));
    estructura += sizeof(uint32_t);
    memcpy(&(un_tripulante->estado_tripulante), estructura, sizeof(char));
    estructura += sizeof(char);
    memcpy(&(un_tripulante->coord_x), estructura, sizeof(uint32_t));
    estructura += sizeof(uint32_t);
    memcpy(&(un_tripulante->coord_y), estructura, sizeof(uint32_t));
    estructura += sizeof(uint32_t);

    return un_tripulante;
}

void eliminar_paquete(t_paquete* paquete) {
	free(paquete->buffer->estructura);
	free(paquete->buffer);
	free(paquete);
}
