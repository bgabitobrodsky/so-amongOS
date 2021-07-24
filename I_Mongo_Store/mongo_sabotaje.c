#include "mongo_sabotaje.h"

extern char** posiciones_sabotajes;
int pos_actual_sabotaje = 0;

void enviar_posicion_sabotaje(int socket_discordiador) {
	log_warning(logger_mongo, "Enviando posiciones de sabotaje.");

	if (pos_actual_sabotaje != contar_palabras(posiciones_sabotajes)) {
		t_posicion posicion;

		posicion.coord_x = (uint32_t) posiciones_sabotajes[pos_actual_sabotaje][0] - 48; // EQUIVALENCIA ASCII NUMERO
		posicion.coord_y = (uint32_t) posiciones_sabotajes[pos_actual_sabotaje][2] - 48; // EQUIVALENCIA ASCII NUMERO
		empaquetar_y_enviar(serializar_posicion(posicion), SABOTAJE, socket_discordiador);

		pos_actual_sabotaje++;

		if (pos_actual_sabotaje == contar_palabras(posiciones_sabotajes)){
			log_warning(logger_mongo, "No hay posiciones de sabotaje.");
			pos_actual_sabotaje = 0;
			log_warning(logger_mongo, "Reiniciadas posiciones.");
		}
	}
}

void reparar() {

    int reparado = 0;
    
	/*log_warning(logger_mongo, "Verifiquemos la cantidad de bloques");
    reparado = verificar_cant_bloques();

    if (reparado){
    	log_warning(logger_mongo, "Se repara la cantidad de bloques del superbloque");
    }

    log_warning(logger_mongo, "Verifiquemos el bitmap");
    reparado = verificar_bitmap();

    if (reparado){
    	log_warning(logger_mongo, "Se repara el bitmap del superbloque");
    }

    log_warning(logger_mongo, "Verifiquemos el sizes");
    reparado = verificar_sizes();

    if (reparado){
    	log_warning(logger_mongo, "Se repara el tamanios de los archivos");
    }
*/
    log_warning(logger_mongo, "Verifiquemos el block counts");
    reparado = verificar_block_counts();

    if (reparado){
    	log_warning(logger_mongo, "Se repara la cantidad de bloques de los recursos");
    }

/*    log_warning(logger_mongo, "Verifiquemos los blocks");
    reparado = verificar_blocks();

    if (reparado){
    	log_warning(logger_mongo, "Se repara la lista de bloques de los recursos");
    }

    if (!reparado){
    	log_warning(logger_mongo, "Estamos joya");
    }*/
}

int verificar_cant_bloques() {

	log_warning(logger_mongo, "Verificando la cantidad de bloques del superbloque.");
	uint32_t cant_bloques = obtener_cantidad_bloques_superbloque();
	log_trace(logger_mongo, "Cantidad de bloques leida de SuperBloque.ims: %i", cant_bloques);

	fseek(directorio.blocks, 0, SEEK_END);
	int tamanio_en_bytes = ftell(directorio.blocks);
	fseek(directorio.blocks, 0, SEEK_SET);
    int cantidad_real = tamanio_en_bytes / TAMANIO_BLOQUE;

    log_trace(logger_mongo, "Cantidad real: %i", cantidad_real);

    if (cant_bloques != cantidad_real) {
        t_bitarray* bitmap = obtener_bitmap();
        reescribir_superbloque(TAMANIO_BLOQUE, cantidad_real, bitmap);
        free(bitmap->bitarray);
        bitarray_destroy(bitmap);
        return 1;
    }
    else{
    	return 0;
    }
}

int verificar_bitmap() {
	//Creo la lista
	t_list* bloques_ocupados = list_create();
	log_warning(logger_mongo, "0");
    recorrer_recursos(bloques_ocupados);
	log_warning(logger_mongo, "Recorridos recursos");
    recorrer_bitacoras(bloques_ocupados);
	log_warning(logger_mongo, "Recorridas bitacoras");
    sortear(bloques_ocupados);

	log_warning(logger_mongo, "Pre if");
    if (bloques_ocupados_difieren(bloques_ocupados)) {
    	log_warning(logger_mongo, "Entrando");
        actualizar_bitmap(bloques_ocupados);
        return 1;
    }
    else
        return 0;

    list_destroy(bloques_ocupados);
}

int verificar_sizes() {
    // Compara tamanio archivo vs lo que ocupa en sus blocks, uno por uno, si alguna vez rompio, devuelve 3, sino 0

//	log_trace(logger_mongo, "Contando");
	int corrompido = 0;

	if(existe_basura) {
		uint32_t tamanio_real_B = bloques_contar('B');

		if(tamanio_real_B != tamanio_archivo(path_basura)) {
			log_trace(logger_mongo, "Size basura saboteada, tamaño real: %i, tamaño encontrado: %i", tamanio_real_B, tamanio_archivo(path_basura));
			set_tam(path_basura, tamanio_real_B);
			corrompido = 1;
		}
	}

	if(existe_comida) {
		uint32_t tamanio_real_C = bloques_contar('C');

		if(tamanio_real_C != tamanio_archivo(path_comida)) {
			log_trace(logger_mongo, "Size comida saboteada, tamaño real: %i, tamaño encontrado: %i", tamanio_real_C, tamanio_archivo(path_comida));
			set_tam(path_comida, tamanio_real_C);
			corrompido = 1;
		}
	}

	if(existe_oxigeno) {
		uint32_t tamanio_real_O = bloques_contar('O');

		if(tamanio_real_O != tamanio_archivo(path_oxigeno)) {
			log_trace(logger_mongo, "Size oxigeno saboteado, tamaño real: %i, tamaño encontrado: %i", tamanio_real_O, tamanio_archivo(path_oxigeno));
			set_tam(path_oxigeno, tamanio_real_O);
			corrompido = 1;
		}
	}

	return corrompido;
}

int verificar_block_counts(t_TCB* tripulante) { 
    // Compara block count vs el largo de la lista de cada archivo recurso.
	int corrompido = 0;

	if(existe_basura) {
		t_list* lista_bloques_basura = get_lista_bloques(path_basura);
		uint32_t cantidad_real_basura = list_size(lista_bloques_basura);

		if(cantidad_real_basura  != cantidad_bloques_recurso(path_basura)) {
			log_trace(logger_mongo, "Block_count basura saboteado, cantidad real: %i, cantidad encontrada: %i", cantidad_real_basura, cantidad_bloques_recurso(path_basura));
			set_cant_bloques(path_basura, cantidad_real_basura);
			corrompido = 1;
		}

		liberar_lista(lista_bloques_basura);
	}

	if(existe_comida) {
		t_list* lista_bloques_comida = get_lista_bloques(path_comida);
		uint32_t cantidad_real_comida = list_size(lista_bloques_comida);

		if(cantidad_real_comida  != cantidad_bloques_recurso(path_comida)) {
			log_trace(logger_mongo, "Block_count comida saboteado, cantidad real: %i, cantidad encontrada: %i", cantidad_real_comida, cantidad_bloques_recurso(path_comida));
			set_cant_bloques(path_comida, cantidad_real_comida);
			corrompido = 1;
		}

		liberar_lista(lista_bloques_comida);
	}

	if(existe_oxigeno) {
		t_list* lista_bloques_oxigeno = get_lista_bloques(path_oxigeno);
		uint32_t cantidad_real_oxigeno = list_size(lista_bloques_oxigeno);

		if(cantidad_real_oxigeno != cantidad_bloques_recurso(path_oxigeno)) {
			log_trace(logger_mongo, "Block_count oxigeno saboteado, cantidad real: %i, cantidad encontrada: %i", cantidad_real_oxigeno, cantidad_bloques_recurso(path_oxigeno));
			set_cant_bloques(path_oxigeno, cantidad_real_oxigeno);
			corrompido = 1;
		}

		liberar_lista(lista_bloques_oxigeno);
	}

	return corrompido;
}

int verificar_blocks() {

	if (md5_no_concuerda() || bitmap_no_concuerda()) {
		restaurar_blocks();
		return 1;
	}

	return 0;
}

int md5_no_concuerda() {

	t_config* config_basura = config_create(path_basura);
	t_config* config_oxigeno = config_create(path_oxigeno);
	t_config* config_comida = config_create(path_comida);

	char* bloques_basura = config_get_string_value(config_basura, "BLOCKS");
	char* bloques_oxigeno = config_get_string_value(config_oxigeno, "BLOCKS");
	char* bloques_comida = config_get_string_value(config_comida, "BLOCKS");

	char* basura_aux = concatenar_numeros(bloques_basura);
	char* oxigeno_aux = concatenar_numeros(bloques_oxigeno);
	char* comida_aux = concatenar_numeros(bloques_comida);

	// log_warning(logger_mongo, "cadena bloque basura: %s", bloques_basura);
	// log_warning(logger_mongo, "cadena bloque oxigeno: %s", bloques_oxigeno);
	// log_warning(logger_mongo, "cadena bloque comida: %s", bloques_comida);

	char* nuevo_md5_basura = md5_archivo(basura_aux);
	char* nuevo_md5_oxigeno = md5_archivo(oxigeno_aux);
	char* nuevo_md5_comida = md5_archivo(comida_aux);

	log_warning(logger_mongo, "nuevo_md5_basura : %s", nuevo_md5_basura);
	log_warning(logger_mongo, "nuevo_md5_oxigeno: %s", nuevo_md5_oxigeno);
	log_warning(logger_mongo, "nuevo_md5_comida: %s", nuevo_md5_comida);

	char* md5_basura = config_get_string_value(config_basura, "MD5_ARCHIVO");
	char* md5_oxigeno = config_get_string_value(config_oxigeno, "MD5_ARCHIVO");
	char* md5_comida = config_get_string_value(config_comida, "MD5_ARCHIVO");

	log_warning(logger_mongo, "viejo md5_basura: %s", md5_basura);
	log_warning(logger_mongo, "viejo md5_oxigeno: %s", md5_oxigeno);
	log_warning(logger_mongo, "viejo md5_comida: %s", md5_comida);

	// Revisar frees
	free(bloques_basura);
	free(bloques_oxigeno);
	free(bloques_comida);
	free(basura_aux);
	free(oxigeno_aux);
	free(comida_aux);

	if (strcmp(nuevo_md5_basura, md5_basura) || strcmp(nuevo_md5_oxigeno, md5_oxigeno) || strcmp(nuevo_md5_comida, md5_comida)) {
		log_warning(logger_mongo, "md5 no concuerda");

		free(nuevo_md5_basura);
		free(nuevo_md5_oxigeno);
		free(nuevo_md5_comida);
		free(md5_basura);
		free(md5_oxigeno);
		free(md5_comida);

		return 1;
	}

	free(nuevo_md5_basura);
	free(nuevo_md5_oxigeno);
	free(nuevo_md5_comida);
	free(md5_basura);
	free(md5_oxigeno);
	free(md5_comida);
	config_destroy(config_basura);
	config_destroy(config_oxigeno);
	config_destroy(config_comida);

	log_warning(logger_mongo, "md5 concuerda");

	return 0;
}

int tamanio_correcto() {
	uint32_t cant_bloques = obtener_cantidad_bloques_superbloque();
	uint32_t tamanio_bloque = obtener_tamanio_bloque_superbloque();

	return (cant_bloques * tamanio_bloque) == strlen(directorio.mapa_blocks); // Revisar logica
}

int bloques_sin_sentido() {
	log_warning(logger_mongo, "Bloques_sin_sentido");
	t_list* bloques_basura = get_lista_bloques(path_basura);
	t_list* bloques_oxigeno = get_lista_bloques(path_oxigeno);
	t_list* bloques_comida = get_lista_bloques(path_comida);
	log_warning(logger_mongo, "Conseguidas listas");
	int* nro_bloque = malloc(sizeof(int));

	for(int i = 0; i < list_size(bloques_basura); i++) {
		nro_bloque = list_get(bloques_basura, i);
		if (*nro_bloque > obtener_cantidad_bloques_superbloque() || *nro_bloque < 0){
			free(nro_bloque);
			//liberar listas
			return 1;
		}
	}

	for(int i = 0; i < list_size(bloques_oxigeno) ; i++) {
		nro_bloque = list_get(bloques_oxigeno, i);
		if (*nro_bloque > obtener_cantidad_bloques_superbloque() || *nro_bloque < 0){
			free(nro_bloque);
			//liberar listas
			return 1;
		}
	}

	for(int i = 0; i < list_size(bloques_comida) ; i++) {
		nro_bloque = list_get(bloques_comida, i);
		if (*nro_bloque > obtener_cantidad_bloques_superbloque() || *nro_bloque < 0){
			//liberar listas
			free(nro_bloque);
			return 1;
		}
	}
	//liberar listas

	free(nro_bloque);
	return 0;
}

int bitmap_no_concuerda() {
	t_bitarray* bitmap = obtener_bitmap();
	t_list* bloques_basura = get_lista_bloques(path_basura);
	t_list* bloques_oxigeno = get_lista_bloques(path_oxigeno);
	t_list* bloques_comida = get_lista_bloques(path_comida);

	int* nro_bloque = malloc(sizeof(int));

	list_add_all(bloques_basura, bloques_oxigeno);
	list_add_all(bloques_basura, bloques_comida);

	for(int i = 0; i < list_size(bloques_basura) ; i++){
		nro_bloque = list_get(bloques_basura, i);
		if (!bitarray_test_bit(bitmap, *nro_bloque)){
			free(nro_bloque);
			//liberar listas
			return 1;
		}

	}

	free(nro_bloque);
	// liberar listas
	return 0;
}

void restaurar_blocks() {
	log_warning(logger_mongo, "inicio restaurar blocks");
	uint32_t tamanio_archivo_basura = tamanio_archivo(path_basura);
	uint32_t tamanio_archivo_oxigeno = tamanio_archivo(path_oxigeno);
	uint32_t tamanio_archivo_comida = tamanio_archivo(path_comida);

	liberar_bloques(path_basura);
	liberar_bloques(path_oxigeno);
	liberar_bloques(path_comida);

	limpiar_metadata(path_basura);
	limpiar_metadata(path_oxigeno);
	limpiar_metadata(path_comida);

	log_warning(logger_mongo, "Tamanio NUEVO BASURA debe ser 0 : %i", tamanio_archivo(path_basura));

	agregar(BASURA, (int) tamanio_archivo_basura);
	agregar(OXIGENO, (int) tamanio_archivo_oxigeno);
	agregar(COMIDA, (int) tamanio_archivo_comida);
	log_warning(logger_mongo, "Agregadas cosas locas");
}

void recorrer_recursos(t_list* bloques_ocupados) {
    // Recorre las listas de las metadatas de los recursos y va anotando en la lista que bloques estan ocupados

	int* aux;

	//BASURA
	if(existe_basura){
		t_list* lista_bloques_basura = get_lista_bloques(path_basura);

		for(int i = 0; i < list_size(lista_bloques_basura); i++) {
			aux = list_get(lista_bloques_basura, i);
			list_add(bloques_ocupados, aux);
		}
		list_destroy(lista_bloques_basura);
	}

	//COMIDA
	if(existe_comida){
		t_list* lista_bloques_comida = get_lista_bloques(path_comida);

		for(int i = 0; i < list_size(lista_bloques_comida); i++) {
			aux = list_get(lista_bloques_comida, i);
			list_add(bloques_ocupados, aux);
		}
		list_destroy(lista_bloques_comida);
	}


	//OXIGENO
	if(existe_oxigeno){
		t_list* lista_bloques_oxigeno = get_lista_bloques(path_oxigeno);

		for(int i = 0; i < list_size(lista_bloques_oxigeno); i++) {
			aux = list_get(lista_bloques_oxigeno, i);
			list_add(bloques_ocupados, aux);
		}
		list_destroy(lista_bloques_oxigeno);
	}

}

void recorrer_bitacoras(t_list* bloques_ocupados) {
	// Recorre las listas de las metadatas de las bitacoras y va anotando en la lista que bloques estan ocupados

	t_bitacora* bitacora_aux;
	int* aux;

	//Itero por todas las bitacoras
	for(int i = 0; i < list_size(bitacoras); i++) {
		bitacora_aux = list_get(bitacoras, i);
		for(int j = 0; j < list_size(bitacora_aux->bloques); j++){
			aux = list_get(bitacora_aux->bloques, j);
			list_add(bloques_ocupados, aux);
		}
	}
}

void sortear(t_list* bloques_ocupados) {
	// Ordeno de menor a mayor
	bool es_menor(void* elemento, void* otro_elemento){
		if(*((int*) elemento) > *((int*) otro_elemento)){
			return 0;
		} else{
			return 1;
		}
	}
	list_sort(bloques_ocupados, es_menor);
}

int bloques_ocupados_difieren(t_list* bloques_ocupados) {
    // Compara lista contra el bitmap, apenas difieren devuelve 1 (como true), sino 0
	int no_difieren = 1;
	log_warning(logger_mongo, "bloques_ocupados_difieren");
	t_bitarray* bitmap = obtener_bitmap();

	for(int i = 0; i < CANTIDAD_BLOQUES; i++) {

		//Si el bit es 1, la lista debe contener el bloque n° i
		if(bitarray_test_bit(bitmap, i)){
			no_difieren = esta_en_lista(bloques_ocupados, i);
		}
		//Si el bit es 0, la lista no debe contener el bloque n° i
		else{
			no_difieren = !esta_en_lista(bloques_ocupados, i);
		}

		//Si el flag es 0, los bloques difieren
		if(!no_difieren){
			log_warning(logger_mongo, "difieren");
			free(bitmap->bitarray);
			bitarray_destroy(bitmap);
			return 1;
		}

	}
	log_warning(logger_mongo, "no difieren");
	free(bitmap->bitarray);
	bitarray_destroy(bitmap);
	return 0;
}
