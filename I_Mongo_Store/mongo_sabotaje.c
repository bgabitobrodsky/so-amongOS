#include "mongo_sabotaje.h"

void enviar_posiciones_sabotaje(int socket_discordiador) {
    //int* lista_posiciones = config_get_array_value(config_mongo, "POSICIONES_SABOTAJE"); // Ver como carajos los devuelve, puede que haya que acoplar numeros de posicion
	int* lista_posiciones = malloc(sizeof(int));
    int tamanio_lista = (sizeof(lista_posiciones)/sizeof(int)) - 1; // Ultimo es NULL segun commons

    for (int i; i < tamanio_lista; i++) {
        empaquetar_y_enviar(serializar_posicion(1, 2), POSICION, socket_discordiador); //TODO codear serliazar posicion en paquetes.h. Cambiar el (1,2)
    }

    enviar_codigo(FIN_LISTA, socket_discordiador);
}

int reparar() {
    int reparado = 0;
    
    reparado = verificar_cant_bloques();

    if (reparado != 0)
        return reparado; 

    reparado = verificar_bitmap();

    if (reparado != 0)
        return reparado; 

    reparado = verificar_sizes();

    if (reparado != 0)
        return reparado; 

    reparado = verificar_block_counts();

    if (reparado != 0)
        return reparado; 

    reparado = verificar_blocks();

    return reparado;
}

int verificar_cant_bloques() {
    int cant_bloques = obtener_cantidad_bloques();
    int cantidad_real = sizeof(directorio.mapa_blocks)/sizeof(char); // Verificar que asi obtenga tamanio

    if (cant_bloques != cantidad_real) {
        int tamanio = obtener_tamanio_bloque();
        obtener_cantidad_bloques(); // Para desplazamiento only

        t_bitarray* bitmap = obtener_bitmap();
        reescribir_superbloque(tamanio, cantidad_real, bitmap);

        return 1;
    }
    else
        return 0;
}

int verificar_bitmap() {
	//Creo la lista
	int* lista_bloques_ocupados = malloc(sizeof(int) * obtener_cantidad_bloques());

	//Agrego los bloques usados en la lista, con en el indice = n° bloque
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

int verificar_sizes() {
    return reasignar_tamanios_archivo();
}

int verificar_block_counts(t_TCB* tripulante) { // Ya que cambio enunciado, se podria hacer la func directamente aca adentro
    int revision = revisar_block_count_recursos();

    if (revision > 0)
        return 4;
    else
        return 0;
}

int verificar_blocks() {
    // TODO: No se entiende enunciado
	return 0;
}

void recorrer_recursos(int* lista_bloques_ocupados) {
    // Recorre las listas de las metadatas de los recursos y va anotando en la lista que bloques estan ocupados
	int i = 0;

	//BASURA
	uint32_t cantidad_bloques_basura = cantidad_bloques_recurso(recurso.basura);
	uint32_t* lista_bloques_basura = lista_bloques_recurso(recurso.basura);

	for(uint32_t j = 0; j < cantidad_bloques_basura; j++) {
		lista_bloques_ocupados[i] = (int) lista_bloques_basura[j];
		i++;
	}

	//COMIDA
	uint32_t cantidad_bloques_comida = cantidad_bloques_recurso(recurso.comida);
	uint32_t* lista_bloques_comida = lista_bloques_recurso(recurso.comida);

	for(uint32_t j = 0; j < cantidad_bloques_comida; j++) {
		lista_bloques_ocupados[i] = (int) lista_bloques_comida[j];
		i++;
	}

	//OXIGENO
	uint32_t cantidad_bloques_oxigeno = cantidad_bloques_recurso(recurso.oxigeno);
	uint32_t* lista_bloques_oxigeno = lista_bloques_recurso(recurso.oxigeno);

	for(uint32_t j = 0; j < cantidad_bloques_oxigeno; j++) {
		lista_bloques_ocupados[i] =(int) lista_bloques_oxigeno[j];
		i++;
	}

	free(lista_bloques_basura);
	free(lista_bloques_comida);
	free(lista_bloques_oxigeno);
}

void recorrer_bitacoras(int* lista_bloques_ocupados) {
	// Recorre las listas de las metadatas de las bitacoras y va anotando en la lista que bloques estan ocupados

	//Obtengo la cantidad de bloques de lista_bloques_ocupados y la cantidad de bitacoras
	int i = sizeof(lista_bloques_ocupados) / sizeof(int);
	int cant_bitacoras = list_size(bitacoras);

	//Itero por todas las bitacoras
	for(int j = 0; j < cant_bitacoras; j++) {
		t_bitacora* bitacora = list_get(bitacoras, i);
		int cant_bloques_bitacora = (int) sizeof(bitacora->bloques) / sizeof(int);
		//Le asigno a lista_bloques_ocupados el bloque n°k de la bitacora n°j
		for(int k = 0; k < cant_bloques_bitacora; k++)
			lista_bloques_ocupados[i] = bitacora->bloques[k];
		i++;
	}
}

void sortear(int* lista_bloques_ocupados) {
	//Obtengo la cantidad de bloques de lista_bloques_ocupados
	int n = sizeof(lista_bloques_ocupados) / sizeof(int);

    int i, j;
    for (i = 0; i < n-1; i++) {
        for (j = 0; j < n-i-1; j++) {
            if (lista_bloques_ocupados[j] > lista_bloques_ocupados[j+1]) {
                int temp = lista_bloques_ocupados[j];
                lista_bloques_ocupados[j] = lista_bloques_ocupados[j+1];
                lista_bloques_ocupados[j+1] = temp;
            }
        }
    }
}

int bloques_ocupados_difieren(int* lista_bloques_ocupados) { //TODO Nico, si queres cambiale el nombre al flag.
    // Compara lista contra el bitmap, apenas difieren devuelve 1 (como true), sino 0
	int no_difieren;

	for(int i = 0; i < CANTIDAD_BLOQUES; i++) {
		t_bitarray* bitmap = obtener_bitmap();

		//Si el bit es 1, la lista debe contener el bloque n° i
		if(bitarray_test_bit(bitmap, i))
			no_difieren = contiene(lista_bloques_ocupados, i);

		//Si el bit es 0, la lista no debe contener el bloque n° i
		else
			no_difieren = ! contiene(lista_bloques_ocupados, i);

		//Si el flag es 0, los bloques difieren
		if(! no_difieren)
			return 1;
	}
	return 0;
}

int contiene(int* lista, int valor) {
	int tamanio_lista = (int) sizeof(lista) / sizeof(int);

	for(int i = 0; i < tamanio_lista; i++) {
		if(lista[i] == valor)
			return 1;
	}
	return 0;
}

int reasignar_tamanios_archivo() {
    // Compara tamanio archivo vs lo que ocupa en sus blocks, uno por uno, si alguna vez rompio alguno devuelve 3, sino 0
	uint32_t * bloques_basura  = lista_bloques_recurso(recurso.basura);
	uint32_t * bloques_comida  = lista_bloques_recurso(recurso.comida);
	uint32_t * bloques_oxigeno = lista_bloques_recurso(recurso.oxigeno);

	int tamanio_real_B = bloques_contar(bloques_basura, 'B');
	int tamanio_real_C = bloques_contar(bloques_basura, 'C');
	int tamanio_real_O = bloques_contar(bloques_basura, 'O');

	if(tamanio_real_B != (int) tamanio_archivo(recurso.basura)) {
		escribir_tamanio(recurso.basura, tamanio_real_B);
		return 3;
	}
	if(tamanio_real_C != (int) tamanio_archivo(recurso.comida)) {
		escribir_tamanio(recurso.comida, tamanio_real_C);
		return 3;
	}
	if(tamanio_real_O != (int) tamanio_archivo(recurso.oxigeno)) {
		escribir_tamanio(recurso.oxigeno, tamanio_real_O);
		return 3;
	}

	return 0;
}

int revisar_block_count_recursos() {
    // Compara block count vs el largo de la lista de cada archivo recurso. Devuelve el bloque roto (1 oxigeno, 2 comida, 3 basura). Si mas de uno roto ver que hacer.
	uint32_t cantidad_real_basura  = sizeof(lista_bloques_recurso(recurso.basura)) / sizeof(uint32_t);
	uint32_t cantidad_real_comida  = sizeof(lista_bloques_recurso(recurso.comida)) / sizeof(uint32_t);
	uint32_t cantidad_real_oxigeno = sizeof(lista_bloques_recurso(recurso.oxigeno)) / sizeof(uint32_t);

	if(cantidad_real_oxigeno != cantidad_bloques_recurso(recurso.oxigeno)) {
		escribir_tamanio(recurso.oxigeno, cantidad_real_oxigeno);
		return 1;
	}
	if(cantidad_real_comida  != cantidad_bloques_recurso(recurso.comida)) {
		escribir_tamanio(recurso.comida, cantidad_real_comida);
		return 2;
	}
	if(cantidad_real_basura  != cantidad_bloques_recurso(recurso.basura)) {
		escribir_tamanio(recurso.basura, cantidad_real_basura);
		return 3;
	}
	return 0;
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
	return 0;
}
