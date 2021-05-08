/*

 Funcionalidad de paquetes, cortesia de Nico

*/

#include "paquetes.h"

// Serializa un struct tripulante a un buffer
// En esta funcion se pone el comportamiento comun de la serializacion
t_buffer serializar_tripulante(t_tripulante tripulante) {

    *t_buffer buffer = malloc((sizeof(t_buffer))); // Se inicializa buffer
    buffer->tamanio_estructura = 3 * sizeof(uint32_t); // Se le da el tamanio del struct del parametro

    void* estructura = malloc((buffer->tamanio_estructura)); // Se utiliza intermediario
    int desplazamiento = 0; // Desplazamiento para calcular que tanto tengo que correr para que no se sobrepisen cosas del array estructura

    memcpy(estructura + desplazamiento, &tripulante.codigo, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(estructura + desplazamiento, &tripulante.coord_x, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(estructura + desplazamiento, &tripulante.coord_y, sizeof(uint32_t)); // Se copia y pega todo al array estructura ordenado

    buffer->estructura = estructura; // Se iguala el buffer al intermediario

    return buffer;

}

// Serializa un struct tarea a un buffer
t_buffer serializar_tarea(t_tarea tarea) {

    *t_buffer buffer = malloc((sizeof(t_buffer)));
    buffer->tamanio_estructura = 5 * sizeof(uint32_t) + sizeof(tarea.nombre) + 1;

    void* estructura = malloc((buffer->tamanio_estructura));
    int desplazamiento = 0;

    
    memcpy(estructura + desplazamiento, &tarea.nombre_largo, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(estructura + desplazamiento, &tarea.nombre, sizeof(tarea.nombre) + 1);
    desplazamiento += sizeof(tarea.nombre) + 1;
    memcpy(estructura + desplazamiento, &tarea.parametro, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(estructura + desplazamiento, &tarea.coord_x, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(estructura + desplazamiento, &tarea.coord_y, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(estructura + desplazamiento, &tarea.duracion, sizeof(uint32_t));

    buffer->estructura = estructura;

    free(tarea.nombre); // Habria que ver si el nombre de la tarea hace falta en src

    return buffer;

}

// Serializa el sabotaje a un buffer, funcion recontra al pedo pero sirve por polimorfismo para envio/recepcion de mensajes
t_buffer serializar_sabotaje() {

    *t_buffer buffer = malloc((sizeof(t_buffer)));

    buffer->tamanio_estructura = sizeof(char);

    void* estructura = malloc((buffer->tamanio_estructura));
    
    memcpy(estructura, &sabotaje, sizeof(char));

    buffer->estructura = estructura;

    return buffer;

}

// Recibe un buffer, un opcode y un socket a donde se enviara el paquete que se armara en la funcion, y se envia
// Usar con funciones de serializacion de arriba
// Usa funcion de socketes.h
void empaquetar(t_buffer buffer, int codigo_operacion, int socket_receptor) {

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = codigo_operacion;
    paquete->buffer = buffer;
    int tamanio_mensaje = buffer->tamanio_estructura + sizeof(uint8_t) + sizeof(uint32_t);

    void* mensaje = malloc(tamanio_mensaje);
    int desplazamiento = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(uint8_t));
    offset += sizeof(uint8_t);
    memcpy(a_enviar + offset, &(paquete->buffer->tamanio_estructura), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->estructura, paquete->buffer->size);

    enviar_mensaje(socket_receptor, mensaje, tamanio_mensaje);

    free(mensaje);
    free(paquete->buffer->estructura);
    free(paquete->buffer);
    free(paquete);  
}


// Recibe con funcion de socketes.h y a partir del opcode desserializa y devuelve un struct que contiene lo recibido
// Se le pasa el socket donde se estan recibiendo mensajes
// IMPORTANTE, es necesario:
// -Ver las cosas dentro del struct estructura a ver que carajos se mando (puede que sea especifico por el socket y no haga falta)
// -Liberar el struct estructura luego de extraer las cosas
estructura_t* recepcion_y_deserializacion(int socket_receptor) { 

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    estructura_t* intermediario = malloc(sizeof(estructura_t*));

    recibir_mensaje(socket_receptor, &(paquete->codigo_operacion), sizeof(uint8_t));
    recibir_mensaje(socket_receptor, &(paquete->buffer->tamanio_estructura), sizeof(uint32_t));
    paquete->buffer->estructura = malloc(paquete->buffer->size);
    recibir_mensaje(socket_receptor, paquete->buffer->estructura, paquete->buffer->tamanio_estructura);

    switch (paquete->codigo_operacion) {
        case 0:
            intermediario->tripulante = malloc(sizeof(t_tripulante));
            &intermediario->tripulante = desserializar_tripulante(paquete->buffer);
            break;
        case 1:
            intermediario->tarea = malloc(sizeof(t_tarea));
            &intermediario->tarea = desserializar_tarea(paquete->buffer->estructura);
            break;
        case 2:
            &intermediario->es_sabotaje = true;
            break;
    }

    free(paquete->buffer->estructura);
    free(paquete->buffer);
    free(paquete);  

    return intermediario;
}

// Pasa un struct buffer a un tripulante
// Se explica deserializacion en esta funcion
t_tripulante* desserializar_tripulante(t_buffer* buffer) {

    t_tripulante* tripulante = malloc(sizeof(t_tripulante)); // Se toma tamaño de lo que sabemos que viene
    void* estructura = buffer->estructura; // Se inicializa intermediario 

    memcpy(&(tripulante->codigo), estructura, sizeof(uint32_t));
    estructura += sizeof(uint32_t);
    memcpy(&(tripulante->coord_x), estructura, sizeof(uint32_t));
    estructura += sizeof(uint32_t);
    memcpy(&(tripulante->coord_y), estructura, sizeof(uint32_t)); // Se toma cada cosa (por el orden en el que vino) y se la "pega" al struct tripulante
    estructura += sizeof(uint32_t);

    return tripulante;
}

// Pasa un struct buffer a una tarea
t_tarea* desserializar_tarea(t_buffer* buffer) {

    t_tarea* tarea = malloc(sizeof(t_tarea));
    void* estructura = buffer->estructura;

    memcpy(&(tarea->nombre_largo), estructura, sizeof(uint32_t));
    estructura += sizeof(uint32_t);
    tarea->nombre = malloc(tarea->nombre_largo);
    memcpy(tarea->nombre, estructura, tarea->nombre_largo);

    memcpy(&(tarea->parametro), estructura, sizeof(uint32_t));
    estructura += sizeof(uint32_t);
    memcpy(&(tarea->coord_x), estructura, sizeof(uint32_t));
    estructura += sizeof(uint32_t);
    memcpy(&(tarea->coord_y), estructura, sizeof(uint32_t));
    estructura += sizeof(uint32_t);
    memcpy(&(tarea->duracion), estructura, sizeof(uint32_t));
    estructura += sizeof(uint32_t);

    return tarea;
}
