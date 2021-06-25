#include "mongo_sabotaje.h"

void enviar_posiciones_sabotaje(int socket_discordiador) {
    int* lista_posiciones = config_get_array_value(config_mongo, "POSICIONES_SABOTAJE"); // Ver como carajos los devuelve, puede que haya que acoplar numeros de posicion
    int tamanio_lista = (sizeof(lista_posiciones)/sizeof(int)) - 1; // Ultimo es NULL segun commons

    for (int i; i < tamanio_lista; i++) {
        empaquetar_y_enviar(serializar_posicion(lista_posiciones[i]), POSICION, socket_discordiador); // TODO: Agregar a enum
    }

    enviar_codigo(FIN_LISTA, socket_discordiador);
}

int reparar(t_TCB* tripulante) { // Ver si llega tripulante o tcb, aunque no tiene comportamiento por ahora
    int reparado = 0;
    
    reparado = verificar_cant_bloques(tripulante);

    if (reparado != 0)
        return reparado; 

    reparado = verificar_bitmap(tripulante);

    if (reparado != 0)
        return reparado; 

    reparado = verificar_sizes(tripulante);

    if (reparado != 0)
        return reparado; 

    reparado = verificar_block_counts(tripulante);

    if (reparado != 0)
        return reparado; 

    reparado = verificar_blocks(tripulante);

    return reparado;
}

int verificar_cant_bloques(t_TCB* tripulante) {
    int cant_bloques = obtener_cantidad_bloques();
    int cantidad_real = sizeof(directorio.mapa_blocks)/sizeof(char); // Verificar que asi obtenga tamanio

    if (cant_bloques != cantidad_real) {
        int tamanio = obtener_tamanio_bloque();
        obtener_cantidad_bloques(); // Para desplazamiento only

        t_bitarray* bitmap = malloc(sizeof(t_bitarray));
	    bitmap = bitarray_create_with_mode(bitmap->bitarray, cant_bloques, LSB_FIRST); //Puede romper
	    fread(bitmap->bitarray, 1/CHAR_BIT, cant_bloques, directorio.superbloque); //CHAR_BIT: represents the number of bits in a char
        reescribir_superbloque(tamanio, cantidad_real, bitmap);

        return 1;
    }
    else
        return 0;
}

int verificar_bitmap(t_TCB* tripulante) {
    int* lista_bloques_ocupados = malloc(sizeof(int) * obtener_cantidad_bloques());
    recorrer_recursos(lista_bloques_ocupados);
    recorrer_bitacoras(lista_bloques_ocupados);

    sortear(lista_bloques_ocupados);

    if (bloques_ocupados_difieren(lista_bloques_ocupados)) {
        t_bitarray* bitmap = actualizar_bitmap();
        reescribir_superbloque(obtener_tamanio_bloque(), obtener_cantidad_bloques(), bitmap);

        return 2;
    }
    else
        return 0;

}

int verificar_sizes(t_TCB* tripulante) {
    return reasignar_tamanios_archivo();
}

int verificar_block_counts(t_TCB* tripulante) {
    int revision = revisar_block_count_recursos();

    if (revision > 0) { // No hago frees, revisar
        switch (revision) {
            case 1:
                uint32_t* lista_oxigenos = lista_bloques_recurso(recurso.oxigeno);
                escribir_archivo_recurso(recurso.oxigeno, tamanio_archivo(recurso.oxigeno), sizeof(lista_oxigenos)/sizeof(uint32_t), lista_oxigenos);
                break;
            case 2:
                uint32_t* lista_comida = lista_bloques_recurso(recurso.comida);
                escribir_archivo_recurso(recurso.comida, tamanio_archivo(recurso.comida), sizeof(lista_comida)/sizeof(uint32_t), lista_comida);
                break;
            case 3:
                uint32_t* lista_basura = lista_bloques_recurso(recurso.basura);
                escribir_archivo_recurso(recurso.basura, tamanio_archivo(recurso.basura), sizeof(lista_basura)/sizeof(uint32_t), lista_basura);
                break;
            default:
                // Ver como hacer si rompen mas de uno, funcion parecida pero auxiliar
                break;
        }

        return 4;
    }
    else
        return 0;
}

int verificar_blocks(t_TCB* tripulante) {
    // TODO: No se entiende enunciado
}

void recorrer_recursos(int* lista_bloques_ocupados) {
    // Recorre las listas de las metadatas de los recursos y va anotando en la lista que bloques estan ocupados
}

void recorrer_bitacoras(int* lista_bloques_ocupados) {
    // Recorre las listas de las metadatas de las bitacoras y va anotando en la lista que bloques estan ocupados
}

void sortear(int* lista_bloques_ocupados) {
    // Ordena la lista para mas facil comparacion con bitmap
}

int bloques_ocupados_difieren(int* lista_bloques_ocupados) {
    // Compara lista contra el bitmap, apenas difieren devuelve 1 (como true), sino 0
}

int reasignar_tamanios_archivo() {
    // Compara tamanio archivo vs lo que ocupa en sus blocks, uno por uno, si alguna vez rompio alguno devuelve 3, sino 0
}

int revisar_block_count_recursos() {
    // Compara block count vs el largo de la lista de cada archivo recurso. Devuelve el bloque roto (1 oxigeno, 2 comida, 3 basura). Si mas de uno roto ver que hacer.
}

char* rompio(int codigo) {
    switch (codigo) {
        case 0:
            return "nada";
        case 1:
            return "la cantidad de bloques del superbloque";
        case 2:
            return "el bitmap del superbloque";
        case 3:
            return "los tamanios de los archivos";
        case 4:
            return "la cantidad de bloques de los recursos";
        case 5:
            return "el blocks";
    }
}