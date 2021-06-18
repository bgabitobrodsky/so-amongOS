/*

 Funcionalidad de paquetes, cortesia de Nico
 TODO de SEBA, falta testear serializacion, envio y recepcion de tareas.

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

    free(tarea.nombre); // TODO: Habria que ver si el nombre de la tarea hace falta en src

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

    // TODO seba: por que esto no te tiro error antes? solo tiene un argumento paquete = realloc(sizeof(int) + sizeof(uint32_t) + sizeof(buffer->tamanio_estructura));

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
    	eliminar_paquete(paquete);
		return intermediario;
    }

    // If que maneja llegada de codigos de operacion unicamente (TODO CODIGO UNICO DEBE ESTAR DESPUES DE SABOTAJE)
    if (paquete->codigo_operacion >= SABOTAJE && paquete->codigo_operacion >= 0) { // Lo del mayor a cero por si llega trash
        intermediario->codigo_operacion = paquete->codigo_operacion;
    	eliminar_paquete(paquete);
        return intermediario;
    }

    recibir_mensaje(socket_receptor, &(paquete->buffer->tamanio_estructura), sizeof(uint32_t));
    paquete->buffer->estructura = malloc(paquete->buffer->tamanio_estructura);
    recibir_mensaje(socket_receptor, paquete->buffer->estructura, paquete->buffer->tamanio_estructura);

    // Switch estructuras y cosas del fylesystem
    switch (paquete->codigo_operacion) { 
    	case RECIBIR_PCB:
    		intermediario->codigo_operacion = RECIBIR_PCB;
    		break;

        case RECIBIR_TCB:
        	intermediario->codigo_operacion = RECIBIR_TCB;
        	intermediario->tcb = malloc(sizeof(uint32_t)*5 + sizeof(char));
            intermediario->tcb = deserializar_tcb(paquete->buffer);
            break;

        case TAREA:
            intermediario->codigo_operacion = TAREA;

            // asignar un malloc? tienes idea de lo loco que se oye eso?
            intermediario->tarea = deserializar_tarea(paquete->buffer);
            break;

        case ARCHIVO_TAREAS:
        	intermediario->codigo_operacion = ARCHIVO_TAREAS;
        	intermediario->archivo_tareas = malloc(paquete->buffer->tamanio_estructura);
            intermediario->archivo_tareas = deserializar_archivo_tareas(paquete->buffer);
            break;

        case T_SIGKILL:
        	intermediario->codigo_operacion = T_SIGKILL;
        	intermediario->tid_condenado = malloc(sizeof(uint32_t));
            intermediario->tid_condenado = deserializar_tid(paquete->buffer);
            break;

        case PEDIR_TAREA:
        	intermediario->codigo_operacion = PEDIR_TAREA;
        	intermediario->tid_condenado = malloc(sizeof(uint32_t));
            intermediario->tid_condenado = deserializar_tid(paquete->buffer);
            break;

        // Funcionan igual, mismo case en definitiva, queda asi para legibilidad, desserializa in situ porque es ezpz
        case OXIGENO:
        case COMIDA:
        case BASURA:
            intermediario->codigo_operacion = paquete->codigo_operacion;
            memcpy(&(intermediario->cantidad), paquete->buffer->estructura, sizeof(int));
            break;
    }

    eliminar_paquete(paquete);

    return intermediario;
}

// Pasa un struct buffer a un tcb
// Se explica deserializacion en esta funcion
t_TCB* deserializar_tcb(t_buffer* buffer) { // TODO: En implementaciones se esta pasando paquete->buffer->estructura, ver si es error

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

void eliminar_paquete(t_paquete* paquete) {
	//free(paquete->buffer->estructura); // TODO: Ver si se puede descomentar
	free(paquete->buffer);
	free(paquete);
}

t_buffer* serializar_archivo_tareas(t_archivo_tareas texto_archivo) {

    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->tamanio_estructura = sizeof(uint32_t)*2 + texto_archivo.largo_texto + 1;

    void* estructura = malloc(buffer->tamanio_estructura);
    int desplazamiento = 0;

    memcpy(estructura + desplazamiento, &texto_archivo.largo_texto, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(estructura + desplazamiento, texto_archivo.texto, (texto_archivo.largo_texto + 1));
    desplazamiento += (texto_archivo.largo_texto + 1);
    memcpy(estructura + desplazamiento, &texto_archivo.pid, sizeof(uint32_t));

    buffer->estructura = estructura;

    free(texto_archivo.texto);

    return buffer;
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

t_buffer* serializar_tid(t_sigkill t_kill) {

    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->tamanio_estructura = sizeof(uint32_t);

    void* estructura = malloc(buffer->tamanio_estructura);
    memcpy(estructura, &t_kill.tid, sizeof(uint32_t));

    buffer->estructura = estructura;

    return buffer;
}

t_sigkill* deserializar_tid(t_buffer* buffer) {

	t_sigkill* trip_kill = malloc(sizeof(t_sigkill));
    void* estructura = buffer->estructura;

    memcpy(&(trip_kill->tid), estructura, sizeof(uint32_t));

    return trip_kill;
}
