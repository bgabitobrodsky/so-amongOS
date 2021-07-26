#include "mongo_sabotaje.h"

extern char** posiciones_sabotajes;
int pos_actual_sabotaje = 0;

void enviar_posicion_sabotaje(int socket_discordiador) {
	log_info(logger_mongo, "Se envia posicion del sabotaje.");

	if (pos_actual_sabotaje != contar_palabras(posiciones_sabotajes)) {
		t_posicion posicion;

		posicion.coord_x = (uint32_t) posiciones_sabotajes[pos_actual_sabotaje][0] - 48; // EQUIVALENCIA ASCII NUMERO
		posicion.coord_y = (uint32_t) posiciones_sabotajes[pos_actual_sabotaje][2] - 48; // EQUIVALENCIA ASCII NUMERO
		empaquetar_y_enviar(serializar_posicion(posicion), SABOTAJE, socket_discordiador);

		pos_actual_sabotaje++;

		if (pos_actual_sabotaje == contar_palabras(posiciones_sabotajes)){
			log_trace(logger_mongo, "No existe posicion de sabotaje.");
			pos_actual_sabotaje = 0;
			log_trace(logger_mongo, "Se reinician las posiciones.");
		}
	}
}

void reparar() {

    int reparado = 0;
    
    log_trace(logger_mongo, "Se verifica la cantidad de bloques.");
    reparado = verificar_cant_bloques();

    if (reparado){
    	log_info(logger_mongo, "Se repara la cantidad de bloques del superbloque.");
    }

    log_trace(logger_mongo, "Se verifica el bitmap.");
    reparado = verificar_bitmap();

    if (reparado){
    	log_info(logger_mongo, "Se repara el bitmap del superbloque.");
    }

    log_trace(logger_mongo, "Se verifica el sizes de los archivos.");
    reparado = verificar_sizes();

    if (reparado){
    	log_info(logger_mongo, "Se repara el tamanio de los archivos.");
    }

    log_trace(logger_mongo, "Se verifica el block counts de los archivos.");
    reparado = verificar_block_counts();

    if (reparado){
    	log_info(logger_mongo, "Se repara la cantidad de bloques de los recursos.");
    }

    log_trace(logger_mongo, "Se verifican los blocks de los archivos.");
    reparado = verificar_blocks();

    if (reparado){
    	log_info(logger_mongo, "Se repara la lista de bloques de los recursos");
    }

    if (!reparado){
    	log_info(logger_mongo, "El FileSystem se encuentra seguro.");
    }
}

int verificar_cant_bloques() {

	uint32_t cant_bloques = obtener_cantidad_bloques_superbloque();
	log_trace(logger_mongo, "Cantidad de bloques leida de SuperBloque.ims: %i", cant_bloques);

	fseek(directorio.blocks, 0, SEEK_END);
	int tamanio_en_bytes = ftell(directorio.blocks);
	fseek(directorio.blocks, 0, SEEK_SET);
    int cantidad_real = tamanio_en_bytes / TAMANIO_BLOQUE;

    log_trace(logger_mongo, "Cantidad real de bloques: %i", cantidad_real);

    if (cant_bloques != cantidad_real) {

        log_trace(logger_mongo, "Cantidades difieren.");
        log_trace(logger_mongo, "Se arregla sabotaje de cantidad de bloques.");

        t_bitarray* bitmap = obtener_bitmap();
        reescribir_superbloque_fd(TAMANIO_BLOQUE, cantidad_real, bitmap);
        free(bitmap->bitarray);
        bitarray_destroy(bitmap);

        return 1;
    }
    else {
    	return 0;
    }
}

int verificar_bitmap() {

	t_list* bloques_ocupados = list_create();

    recorrer_recursos(bloques_ocupados);
	log_trace(logger_mongo, "Recorridos recursos.");
    recorrer_bitacoras(bloques_ocupados);
	log_trace(logger_mongo, "Recorridas bitacoras.");

    sortear(bloques_ocupados);
	log_trace(logger_mongo, "Se ordenan los bloques ocupados.");

    if (bloques_ocupados_difieren(bloques_ocupados)) {

    	log_trace(logger_mongo, "Bloques ocupados difieren.");
    	log_trace(logger_mongo, "Se arregla el bitmap.");

        actualizar_bitmap(bloques_ocupados);
        matar_lista(bloques_ocupados);

        return 1;
    }
    else {
        matar_lista(bloques_ocupados);

        return 0;
    }

}

int verificar_sizes() {
    // Compara tamanio archivo vs lo que ocupa en sus blocks, uno por uno, si alguna vez rompio, devuelve 3, sino 0

	int corrompido = 0;

	if(existe_basura) {
		if(verificar_size_recurso(path_basura)) {
			log_trace(logger_mongo, "Tamanio de basura incorrecto.");
			corrompido = 1;
		}
	}

	if(existe_comida) {
		if(verificar_size_recurso(path_comida)) {
			log_trace(logger_mongo, "Tamanio de comida incorrecto.");
			corrompido = 1;
		}
	}

	if(existe_oxigeno) {
		if(verificar_size_recurso(path_oxigeno)){
			log_trace(logger_mongo, "Tamanio de oxigeno incorrecto.");
			corrompido = 1;
		}
	}

	return corrompido;
}

int verificar_size_recurso(char* path) {

	//int corrompido = 0;
	char caracter = caracter_llenado_archivo(path);
	uint32_t tamanio_real_recurso = bloques_contar(caracter);

	if (tamanio_real_recurso != tamanio_archivo(path)) {
		log_trace(logger_mongo, "Tamanio real: %i.", tamanio_real_recurso);
		log_trace(logger_mongo, "Tamanio encontrando: %i.", tamanio_archivo(path));
		log_trace(logger_mongo, "Tamanios difieren, se arregla el tamanio del archivo de path %s.", path);

		set_tam(path, tamanio_real_recurso);

		//corrompido = 1;
		return 1;
	}

	// return corrompido;
	return 0;
}

int verificar_block_counts(t_TCB* tripulante) { 
    // Compara block count vs el largo de la lista de cada archivo recurso.
	int corrompido = 0;

	if(existe_basura) {
		if(verificar_block_counts_recurso(path_basura)){
			log_trace(logger_mongo, "Cantidad de bloques de basura incorrecto.");
			corrompido = 1;
		}
	}

	if(existe_comida) {
		if(verificar_block_counts_recurso(path_comida)){
			log_trace(logger_mongo, "Cantidad de bloques de comida incorrecto.");
			corrompido = 1;
		}
	}

	if(existe_oxigeno) {
		if(verificar_block_counts_recurso(path_oxigeno)){
			log_trace(logger_mongo, "Cantidad de bloques de oxigeno incorrecto.");
			corrompido = 1;
		}
	}

	return corrompido;
}

int verificar_block_counts_recurso(char* path) {

	int corrompido = 0;

	t_list* lista_bloques = get_lista_bloques(path);
	uint32_t cantidad_real = list_size(lista_bloques);

	if(cantidad_real  != cantidad_bloques_recurso(path)) {
		log_trace(logger_mongo, "Cantidad real: %i.", cantidad_real);
		log_trace(logger_mongo, "Cantidad encontrando: %i.", cantidad_bloques_recurso(path));
		log_trace(logger_mongo, "Cantidades difieren, se arregla la cantidad de bloques del path %s.", path);

		set_cant_bloques(path, cantidad_real);
		corrompido = 1;
	}

	matar_lista(lista_bloques);
	return corrompido;
}

int verificar_blocks() {

	int codigo = md5_no_concuerda();

	if(codigo != 0){
		restaurar_blocks(codigo);

		return 1;
	}

	return 0;
}

int md5_no_concuerda() {

	if(existe_basura) {
		if(md5_no_concuerda_recurso(path_basura)) {
			log_trace(logger_mongo, "No concuerda MD5 de Basura.");
			return BASURA;
		}
	}

	if(existe_comida) {
		if(md5_no_concuerda_recurso(path_comida)) {
			log_trace(logger_mongo, "No concuerda MD5 de Comida.");
			return COMIDA;
		}
	}

	if(existe_oxigeno) {
		if(md5_no_concuerda_recurso(path_oxigeno)) {
			log_trace(logger_mongo, "No concuerda MD5 de Oxigeno.");
			return OXIGENO;
		}
	}

	log_trace(logger_mongo, "Los MD5 de los archivos concuerdan.");

	return 0;
}

int md5_no_concuerda_recurso(char* path_recurso) {

	t_config* config = config_create(path_recurso);
	char* bloques = config_get_string_value(config, "BLOCKS");
	int cant_bloques = config_get_int_value(config, "BLOCK_COUNT");
	log_trace(logger_mongo, "Cadena de bloques del path %s: %s", path_recurso, bloques);
	char* bloques_aux = concatenar_numeros(bloques, cant_bloques);

	char* nuevo_md5 = md5_archivo(bloques_aux);
	log_trace(logger_mongo, "MD5 real: %s", nuevo_md5);
	char* md5 = config_get_string_value(config, "MD5_ARCHIVO");
	log_trace(logger_mongo, "MD5 de archivo: %s", md5);

	if (strcmp(nuevo_md5, md5)) {
		free(nuevo_md5);
		free(bloques_aux);
		config_destroy(config);
		return 1;
	}

	free(nuevo_md5);
	free(bloques_aux);
	config_destroy(config);
	return 0;
}

int bitmap_no_concuerda() {

	t_bitarray* bitmap = obtener_bitmap();
	int* nro_bloque = malloc(sizeof(int));
	t_list* bloques = list_create();

	if(existe_basura) {
		t_list* bloques_basura = get_lista_bloques(path_basura);
		list_add_all(bloques, bloques_basura);
	}

	if(existe_comida) {
		t_list* bloques_comida = get_lista_bloques(path_comida);
		list_add_all(bloques, bloques_comida);
	}

	if(existe_oxigeno) {
		t_list* bloques_oxigeno = get_lista_bloques(path_oxigeno);
		list_add_all(bloques, bloques_oxigeno);
	}

	for(int i = 0; i < list_size(bloques) ; i++){
		nro_bloque = list_get(bloques, i);

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

void restaurar_blocks(int codigo) {

	uint32_t tamanio;

	switch(codigo){
		case BASURA:
			tamanio = tamanio_archivo(path_basura);
			liberar_bloques(path_basura);
			limpiar_metadata(path_basura);
			log_trace(logger_mongo, "Tamanio de Basura debe ser 0 : %i", tamanio_archivo(path_basura));
			agregar(BASURA, (int) tamanio);
			break;

		case COMIDA:
			tamanio = tamanio_archivo(path_comida);
			liberar_bloques(path_comida);
			limpiar_metadata(path_comida);
			log_trace(logger_mongo, "Tamanio de Comida debe ser 0 : %i", tamanio_archivo(path_comida));
			agregar(COMIDA, (int) tamanio);
			break;

		case OXIGENO:
			tamanio = tamanio_archivo(path_oxigeno);
			liberar_bloques(path_oxigeno);
			limpiar_metadata(path_oxigeno);
			log_trace(logger_mongo, "Tamanio de Oxigeno debe ser 0 : %i", tamanio_archivo(path_oxigeno));
			agregar(OXIGENO, (int) tamanio);
			break;
	}
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
	bool es_menor(void* elemento, void* otro_elemento) {
		if (*((int*) elemento) > *((int*) otro_elemento)) {
			return 0;
		}
		else {
			return 1;
		}
	}

	list_sort(bloques_ocupados, es_menor);
}

int bloques_ocupados_difieren(t_list* bloques_ocupados) {
    // Compara lista contra el bitmap, apenas difieren devuelve 1 (como true), sino 0
	int no_difieren = 1;
	t_bitarray* bitmap = obtener_bitmap();

	for(int i = 0; i < CANTIDAD_BLOQUES; i++) {

		//Si el bit es 1, la lista debe contener el bloque n° i
		if (bitarray_test_bit(bitmap, i)) {
			no_difieren = esta_en_lista(bloques_ocupados, i);
		}
		//Si el bit es 0, la lista no debe contener el bloque n° i
		else {
			no_difieren = !esta_en_lista(bloques_ocupados, i);
		}

		//Si el flag es 0, los bloques difieren
		if (!no_difieren) {
			log_trace(logger_mongo, "Los bloques ocupados difieren.");
			free(bitmap->bitarray);
			bitarray_destroy(bitmap);

			return 1;
		}
	}

	log_trace(logger_mongo, "Los bloques ocupados no difieren.");
	free(bitmap->bitarray);
	bitarray_destroy(bitmap);

	return 0;
}
