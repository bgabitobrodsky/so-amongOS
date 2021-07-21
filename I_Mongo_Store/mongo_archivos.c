#include "mongo_archivos.h"


// Vars globales
t_log* logger_mongo;
t_config* config_mongo;
t_list* bitacoras;

// TODO ver tema MD5:
//		-https://www.it-swarm-es.com/es/c/como-crear-un-hash-md5-de-una-cadena-en-c/939706723/
//		-https://stackoverflow.com/questions/58065208/calculate-md5-in-c-display-output-as-string

void inicializar_archivos() {
	// Se obtiene el path al archivo oxigeno dentro de la carpeta files
	path_oxigeno = malloc((strlen(path_files)+1) + strlen("/Oxigeno.ims"));
	sprintf(path_oxigeno, "%s/Oxigeno.ims", path_files);

	// Se obtiene el path al archivo comida dentro de la carpeta files
	path_comida = malloc((strlen(path_files)+1) + strlen("/Comida.ims"));
	sprintf(path_comida, "%s/Comida.ims", path_files);

	// Se obtiene el path al archivo basura dentro de la carpeta files
	path_basura = malloc((strlen(path_files)+1) + strlen("/Basura.ims"));
	sprintf(path_basura, "%s/Basura.ims", path_files);

	// Se obtiene el path al archivo superbloque dentro de la carpeta files (deberia ser dentro del punto de montaje nomas)
	path_superbloque = malloc((strlen(path_directorio)+1) + strlen("/SuperBloque.ims"));
	sprintf(path_superbloque, "%s/SuperBloque.ims", path_directorio);

	// Se obtiene el path al archivo blocks dentro de la carpeta files (deberia ser dentro del punto de montaje nomas)
	path_blocks = malloc((strlen(path_directorio)+1) + strlen("/Blocks.ims"));
	sprintf(path_blocks, "%s/Blocks.ims", path_directorio);

	log_trace(logger_mongo, "post printf");

	int filedescriptor_blocks = open(path_blocks, O_RDWR | O_APPEND | O_CREAT, (mode_t) 0777);

	// Trunco los archivos o los creo en modo escritura y lectura
	// Se guarda to.do en un struct para uso en distintas funciones
    recurso.oxigeno        = fopen(path_oxigeno, "w+b");
	recurso.comida         = fopen(path_comida, "w+b");
	recurso.basura         = fopen(path_basura, "w+b");
	directorio.superbloque = fopen(path_superbloque, "w+b");
	directorio.blocks      = fdopen(filedescriptor_blocks, "w+b");

	iniciar_archivo_recurso(path_oxigeno, 0, 0, NULL);
	iniciar_archivo_recurso(path_comida, 0, 0, NULL);
	iniciar_archivo_recurso(path_basura, 0, 0, NULL);

	log_trace(logger_mongo, "pre superbloque");
	iniciar_superbloque(directorio.superbloque);
	log_trace(logger_mongo, "post_superbloque");
	iniciar_blocks(filedescriptor_blocks); // Actualizar struct
	inicializar_mapa();
}

void inicializar_archivos_preexistentes() {
	// Se obtiene el path al archivo oxigeno dentro de la carpeta files
	path_oxigeno = malloc((strlen(path_files)+1) + strlen("/Oxigeno.ims"));
	sprintf(path_oxigeno, "%s/Oxigeno.ims", path_files);

	// Se obtiene el path al archivo comida dentro de la carpeta files
	path_comida = malloc((strlen(path_files)+1) + strlen("/Comida.ims"));
	sprintf(path_comida, "%s/Comida.ims", path_files);

	// Se obtiene el path al archivo basura dentro de la carpeta files
	path_basura = malloc((strlen(path_files)+1) + strlen("/Basura.ims"));
	sprintf(path_basura, "%s/Basura.ims", path_files);

	// Se obtiene el path al archivo superbloque dentro de la carpeta files (deberia ser dentro del punto de montaje nomas)
	path_superbloque = malloc((strlen(path_directorio)+1) + strlen("/SuperBloque.ims"));
	sprintf(path_superbloque, "%s/SuperBloque.ims", path_directorio);

	// Se obtiene el path al archivo blocks dentro de la carpeta files (deberia ser dentro del punto de montaje nomas)
	path_blocks = malloc((strlen(path_directorio)+1) + strlen("/Blocks.ims"));
	sprintf(path_blocks, "%s/Blocks.ims", path_directorio);

	int filedescriptor_blocks = open(path_blocks, O_RDWR | O_APPEND | O_CREAT, (mode_t) 0777);

	// Abro los archivos en modo escritura y lectura (deben existir archivos)
	// Se guarda to.do en un struct para uso en distintas funciones

	recurso.oxigeno        = fopen(path_oxigeno, "r+b");
	recurso.comida         = fopen(path_comida, "r+b");
	recurso.basura         = fopen(path_basura, "r+b");
	directorio.superbloque = fopen(path_superbloque, "r+b");
	directorio.blocks      = fdopen(filedescriptor_blocks, "r+b");

	iniciar_blocks(filedescriptor_blocks); // Actualizar struct TODO: que?
	// mapea y sincroniza
	memcpy(directorio.mapa_blocks, mapa, CANTIDAD_BLOQUES * TAMANIO_BLOQUE);
    msync(mapa, CANTIDAD_BLOQUES * TAMANIO_BLOQUE, MS_ASYNC);

    cargar_bitmap(); // cargando la lista de bloqueados
    limpiar_cuerpos(); // TODO codear
    // rename("/home/utnso/polus/Files/Bitacoras/Tripulante10001.ims", "/home/utnso/polus/Files/Bitacoras/OldTripulante10001.ims");

	log_error(logger_mongo, "3 inicializar_archivos_preexistentes");
}

void limpiar_cuerpos() {
	/*
	path_bitacoras = malloc((strlen(path_files)+1) + strlen("/Bitacoras"));
	sprintf(path_bitacoras, "%s/Bitacoras", path_files);
	char * const * lista_path = {path_bitacoras};

	FTS *ftsp;
	FTSENT *p, *chp;
	int fts_options = FTS_COMFOLLOW | FTS_LOGICAL | FTS_NOCHDIR;
	int rval = 0;

	fts_open(lista_path, fts_options, NULL);
	chp = fts_children(ftsp, 0);

	if (chp == NULL) {
		return;
	}

	while ((p = fts_read(ftsp)) != NULL) {
		switch (p->fts_info) {
	    	case FTS_F:
	    		*(p->fts_name) = strcat(*(p->fts_name), "RIP");
	    		break;
	    	default:
	    		break;
	    	}
		}
	fts_close(ftsp);
	return; */
}

void asignar_nuevo_bloque(char* path, int size_agregado) { //TODO sincronizar
	log_warning(logger_mongo, "sincronizar inicio");
	lockearEscritura(path_superbloque); //Esta feo pero sino me rompía
	lockearEscritura(path_blocks);
	log_trace(logger_mongo, "0 asignar_nuevo_bloque");

	t_bitarray* bitmap = obtener_bitmap();

	int bit_libre = 0;
	int* pos_libre = malloc(sizeof(int));

	//Recorro todas las posiciones del bitarray
	for (uint32_t i = 0; i < CANTIDAD_BLOQUES; i++){
		//Entra si el bit del bitmap está en 0
		if(!bitarray_test_bit(bitmap, i)){
			bit_libre = 1;
			*pos_libre = i;
			break;
		}
	}

	//Si había un bloque libre
	if (bit_libre == 1) {
		log_trace(logger_mongo, "Habemus bloque libre, el bit libre es = %i", *pos_libre);
		//Marco el bit como ocupado
		bitarray_set_bit(bitmap, *pos_libre);

		if (es_recurso(path)){
			log_trace(logger_mongo, "Asignemos un bloque a un recurso");
			asignar_bloque_recurso(path, pos_libre);
		}
		else {
			log_trace(logger_mongo, "Asignemos un bloque a un tripulante");
			asignar_bloque_tripulante(path, pos_libre, size_agregado);
		}


		list_add(lista_bloques_ocupados, pos_libre);
		actualizar_bitmap(lista_bloques_ocupados);

	}
	else{
		log_info(logger_mongo, "No hay bloques disponibles en este momento");
	}
	unlockear(path_superbloque);
	unlockear(path_blocks);
	log_warning(logger_mongo, "sincronizar fin");

	/*
	log_trace(logger_mongo, "fin asignar_nuevo_bloque");
	imprimir_bitmap();
	log_trace(logger_mongo, "pre bitmap destroy");
    */

	bitarray_destroy(bitmap);
	log_trace(logger_mongo, "post");

}

int llenar_bloque_recurso(t_list* lista_bloques, int cantidad_deseada, char tipo, char* path) {

	log_trace(logger_mongo, "0 asignar_primer_bloque, cant bloques %i", list_size(lista_bloques));
	int cantidad_alcanzada = 0;

	if(list_is_empty(lista_bloques) && cantidad_deseada != 0){
		log_trace(logger_mongo, "la lista ta vacia");
		asignar_nuevo_bloque(path, 0);
		lista_bloques = obtener_lista_bloques(path);
	}

	int* aux = malloc(sizeof(int));

	log_trace(logger_mongo, "Entrando al for de agregar");
	for(int i = 0; i < list_size(lista_bloques); i++){
		aux = list_get(lista_bloques, i);

		for(int j = 0; j < TAMANIO_BLOQUE; j++){
			// log_trace(logger_mongo, "pos %i lista; %i: ", i, *aux);

			if (*(directorio.mapa_blocks + *aux * TAMANIO_BLOQUE + j) == ',') {
				*(directorio.mapa_blocks + *aux * TAMANIO_BLOQUE + j) = tipo;

				cantidad_alcanzada++;
				// log_trace(logger_mongo, "cantidad mas mas");
			}

			if (cantidad_alcanzada == cantidad_deseada) {
				// log_trace(logger_mongo, "retorno cero");
				return 0;
			}
		}
	}

	return cantidad_alcanzada - cantidad_deseada;
}

int quitar_ultimo_bloque_libre(t_list* lista_bloques, int cantidad_deseada, char tipo) {
	// TODO LIBERAR BLOQUES QUE SE TERMINAN
	log_trace(logger_mongo, "INICIO quitar_ultimo_bloque, cant bloques %i", list_size(lista_bloques));

	int cantidad_alcanzada = 0;
	int* aux = malloc(sizeof(int));

	log_trace(logger_mongo, "Entrando al for de quitar");
	for(int i = (list_size(lista_bloques) - 1); i >= 0 ; i--){

		aux = list_get(lista_bloques, i);

		for(int j = 0; j < TAMANIO_BLOQUE; j++){
			if (*(directorio.mapa_blocks + (*aux + 1) * TAMANIO_BLOQUE - j) == tipo) {
				*(directorio.mapa_blocks + (*aux + 1) * TAMANIO_BLOQUE - j) = ',';
				cantidad_alcanzada++;
			}

			if (cantidad_alcanzada == cantidad_deseada) {
				return 0;
			}
		}
	}
	
	return cantidad_alcanzada - cantidad_deseada;
}

void alterar(int codigo_archivo, int cantidad) {  
	log_trace(logger_mongo, "INICIO alterar, codigo = %i cantidad = %i", codigo_archivo, cantidad);

	if (cantidad >= 0){
		agregar(codigo_archivo, cantidad);
		log_info(logger_mongo, "Se agregaron %i unidades a %s.", cantidad, conseguir_tipo(conseguir_char(codigo_archivo)));
	}
	else{
		quitar(codigo_archivo, cantidad * (-1));
		log_info(logger_mongo, "Se quitaron %i unidades a %s.", cantidad, conseguir_tipo(conseguir_char(codigo_archivo)));
	}
}

void agregar(int codigo_archivo, int cantidad) { // Puede que haya que hacer mallocs previos
	log_trace(logger_mongo, "INICIO agregar, codigo: %i, cantidad %i", codigo_archivo, cantidad);

	char* path = conseguir_path_recurso_codigo(codigo_archivo);

	log_error(logger_mongo, "nuestro path es %s", path);

	t_list* lista_bloques = obtener_lista_bloques(path);
	char tipo = caracter_llenado_archivo(path);
	log_trace(logger_mongo, "1 agregar");

	int offset = llenar_bloque_recurso(lista_bloques, cantidad, tipo, path);

	log_trace(logger_mongo, "list tamanio %i", list_size(lista_bloques));
	log_trace(logger_mongo, "OFFSET: %i", offset);

	if (offset < 0) { // Falto agregar cantidad, dada por offset
		log_error(logger_mongo, "entro a luchar por el offset. Cantidad faltante: %i", offset);
		asignar_nuevo_bloque(path, 0);
		agregar(codigo_archivo, offset * (-1)); // Recursividad con la cantidad que falto
	}

	log_trace(logger_mongo, "2 agregar");

	uint32_t tam_archivo = tamanio_archivo(path);
	uint32_t cant_bloques = cantidad_bloques_recurso(path);
	lista_bloques = obtener_lista_bloques(path);

	iniciar_archivo_recurso(path, tam_archivo + cantidad + offset, cant_bloques, lista_bloques);
	log_error(logger_mongo, "Cantidad agregada: %i", cantidad);

	log_trace(logger_mongo, "FIN agregar");

    // pthread_mutex_unlock(&mutex_blocks);
}

void quitar(int codigo_archivo, int cantidad) {
	log_trace(logger_mongo, "INICIO quitar");
	char* path = conseguir_path_recurso_codigo(codigo_archivo);

	uint32_t tam_archivo = tamanio_archivo(path);
	uint32_t cant_bloques = cantidad_bloques_recurso(path);
	t_list* lista_bloques = obtener_lista_bloques(path);
	char tipo = caracter_llenado_archivo(path);

	quitar_ultimo_bloque_libre(lista_bloques, cantidad, tipo);

	// TODO: ver como quitar los bloques que se eliminan al quitar
	iniciar_archivo_recurso(path, tam_archivo - cantidad, cant_bloques, lista_bloques);


	log_trace(logger_mongo, "FIN quitar");
    // pthread_mutex_unlock(&mutex_blocks);
}

char conseguir_char(int codigo_operacion) {

	switch(codigo_operacion) {
	case OXIGENO:
		return 'O';
		break;
	case COMIDA:
		return 'C';
		break;
	case BASURA:
		return 'B';
		break;
	}
	return '\0';
}

char* conseguir_tipo(char tipo) {
	if (tipo == 'O')
        return "Oxigeno";
    if (tipo == 'C')
        return "Comida";
    if (tipo == 'B')
        return "Basura";
    return NULL;
}

FILE* conseguir_archivo_recurso(int codigo) {
	switch(codigo) {
	case BASURA:
		return recurso.basura;
		break;
	case COMIDA:
		return recurso.comida;
		break;
	case OXIGENO:
		return recurso.oxigeno;
		break;
	}
	log_error(logger_mongo, "Archivo de recurso no encontrado");
	return NULL;
}

FILE* conseguir_archivo_char(char tipo) {
	if (tipo == 'O')
        return recurso.oxigeno;
    if (tipo == 'C')
        return recurso.comida;
    if (tipo == 'B')
        return recurso.basura;
    return NULL;
}

char* conseguir_path_recurso_codigo(int codigo) {
	switch(codigo) {
		case OXIGENO:
			return path_oxigeno;
			break;
		case COMIDA:
			return path_comida;
			break;
		case BASURA:
			return path_basura;
			break;
	}
	log_error(logger_mongo, "Archivo de recurso no encontrado");
	return NULL;
}

char* conseguir_path_recurso_archivo(FILE* archivo) {

	if(archivo == recurso.basura){
		return path_basura;
	}
	if(archivo == recurso.comida){
		return path_comida;
	}
	if(archivo == recurso.oxigeno){
		return path_oxigeno;
	}
	return NULL;

}


FILE* conseguir_archivo(char* path) {

	log_trace(logger_mongo, "INICIO conseguir_archivo");

	if(path == path_comida){
		return recurso.comida;
	}
	if(path == path_basura){
		return recurso.basura;
	}
	if(path == path_oxigeno){
		return recurso.oxigeno;
	}

	else{
		t_bitacora* aux;
		for(int i = 0; i < list_size(bitacoras); i++){
			aux = list_get(bitacoras, i);
			if(comparar_strings(aux->path, path)){
				return aux->bitacora_asociada;
			}
		}
	}

	return NULL;
}

int max (int a, int b) {
	if (a >= b) {
		return a;
	}
	else {
		return b;
	}
}

int min (int a, int b) {
	if (a < b) {
		return a;
	}
	else {
		return b;
	}
}


int tamanio_archivo(char* path) {
	if(es_recurso(path)) {
		log_trace(logger_mongo, "lockear tamaño");
		lockearLectura(path);
	}
	t_config* config = config_create(path);
	int tam_archivo = config_get_int_value(config, "SIZE");
	config_destroy(config);
	if(es_recurso(path)) {
		log_trace(logger_mongo, "unlockear tamaño");
		unlockear(path);
	}
	return tam_archivo;
}

uint32_t cantidad_bloques_recurso(char* path) {
	log_trace(logger_mongo, "lockear cantidad bloques");
	lockearLectura(path);

	t_config* config = config_create(path);
	int cant_bloques = config_get_int_value(config, "BLOCK_COUNT");
	config_destroy(config);

	log_trace(logger_mongo, "unlockear cantidad bloques");
	unlockear(path);

	return cant_bloques;
}

char caracter_llenado_archivo(char* path) {

	if (comparar_strings(path, path_comida)){
		return 'C';
	}

	if (comparar_strings(path, path_basura)){
		return 'B';
	}

	if (comparar_strings(path, path_oxigeno)){
		return 'O';
	}

	return '0';
}

char* md5_archivo(char* path) {
//	log_trace(logger_mongo, "lockear md5");
//	lockearLectura(path);

	int n;
	int largo = strlen(path);
	MD5_CTX c;
	unsigned char digest[16];
	char *out = (char*)malloc(33);

	MD5_Init(&c);

	while (largo > 0) {
		if (largo > 512) {
			MD5_Update(&c, path, 512);
	    } else {
	    	MD5_Update(&c, path, largo);
	    }
		largo -= 512;
	    path += 512;
	}

	MD5_Final(digest, &c);

	for (n = 0; n < 16; ++n) {
		snprintf(&(out[n*2]), 16*2, "%02x", (unsigned int)digest[n]);
	}

//	log_trace(logger_mongo, "unlockear md5");
//	unlockear(path);

	return out;
}

uint32_t obtener_cantidad_bloques(char* path){
	t_config* config = config_create(path);
	log_debug(logger_mongo, "0 obtener_cantidad_bloques");

	char** bloques = config_get_array_value(config, "BLOCKS");
	int cant_bloques = contar_palabras(bloques);

	log_debug(logger_mongo, "FIN obtener_cantidad_bloques");

	return cant_bloques;
}

t_list* obtener_lista_bloques(char* path){
	// ESTA FUNCION DEBE LIBERAR EL RETORNO

	log_trace(logger_mongo, "Obteniendo la lista de bloques de %s", path);
	t_config* config = config_create(path);

	if(!config_has_property(config, "BLOCK_COUNT")){
		log_warning(logger_mongo, "EL path no tiene BLOCK_COUNT, osea es un tripulante");

		char** bloques = config_get_array_value(config, "BLOCKS");
		t_list* lista_bloques = list_create();
		if(bloques[0] == NULL){
			log_error(logger_mongo, "EL path no tiene bloques");
			return lista_bloques;
		}

		int* aux;
		for(int i = 0; i < contar_palabras(bloques); i++){
			aux = malloc(sizeof(int));
			*aux = atoi(bloques[i]);
			list_add(lista_bloques, aux);
		}

		log_trace(logger_mongo, "Returneo tripulante");

		return lista_bloques;
	}

	log_trace(logger_mongo, "lockear bloques inicio");
	lockearLectura(path);

	uint32_t cant_bloques = config_get_int_value(config, "BLOCK_COUNT");

	t_list* lista_bloques = list_create();
	char** bloques = config_get_array_value(config, "BLOCKS");
	if(bloques[0] == NULL){
		log_error(logger_mongo, "EL path no tiene bloques");
		unlockear(path);
		return lista_bloques;
	}

	int* aux;

	for(int i = 0; i < cant_bloques; i++){
		aux = malloc(sizeof(int));
		*aux = atoi(bloques[i]);
		list_add(lista_bloques, aux);
	}

	log_trace(logger_mongo, "unlockear bloques inicio");
	unlockear(path);

	return lista_bloques;
}

void iniciar_archivo_recurso(char* path, int tamanio, int cant_bloques, t_list* lista_bloques){
	log_info(logger_mongo, "lockear escritura recurso");
	lockearEscritura(path);

	log_trace(logger_mongo, "0 iniciar_archivo_recurso");
	set_tam(path, tamanio);
	set_cant_bloques(path, cant_bloques);
	set_bloq(path, lista_bloques);
	char caracter = caracter_llenado_archivo(path);
	set_caracter_llenado(path, caracter);

	t_config* config = config_create(path);
	char* cadena_blocks = config_get_string_value(config, "BLOCKS");
	char* md5 = md5_archivo(cadena_blocks);
	set_md5(path, md5);
	config_destroy(config);

	log_trace(logger_mongo, "FIN iniciar_archivo_recurso");

	log_trace(logger_mongo, "unlockear escritura recurso");
	unlockear(path);
}

void escribir_archivo_tripulante(char* path, uint32_t tamanio, t_list* list_bloques) {
	log_trace(logger_mongo, "INICIO escribir_archivo_tripulante");

//	log_trace(logger_mongo, "tamanio antes de escribir archivo: %i", tamanio_archivo(path)); Rompe si se está creando el archivo
	set_tam(path, tamanio);
	log_trace(logger_mongo, "tamanio despues de escribir archivo: %i", tamanio_archivo(path));

//	t_list* aux = obtener_lista_bloques(path);
//	for(int i = 0; i < list_size(aux); i++)
//	log_trace(logger_mongo, "bloques antes de escribir archivo: %i", (int) list_get(aux, i));
	set_bloq(path, list_bloques);
	t_list* aux = obtener_lista_bloques(path);
	for(int i = 0; i < list_size(aux); i++)
	log_trace(logger_mongo, "bloques antes de escribir archivo: %i", (int) list_get(aux, i));

	log_trace(logger_mongo, "FIN escribir_archivo_tripulante");
}

int es_recurso(char* path) {

	if(comparar_strings(path, path_basura)){
		return 1;
	}
	if(comparar_strings(path, path_oxigeno)){
		return 1;
	}
	if(comparar_strings(path, path_comida)){
		return 1;
	}

	return 0;

}

void asignar_bloque_recurso(char* path, int* pos_libre) {

	uint32_t tamanio = tamanio_archivo(path);
	uint32_t cantidad_bloques = cantidad_bloques_recurso(path);
	log_trace(logger_mongo, "cantidad de bloques: %i, sera aumentada", cantidad_bloques);
	t_list* lista_bloques = obtener_lista_bloques(path);

	list_add(lista_bloques, pos_libre);

	/*// PRINTEO DE TESTEO
	int* aux;
	log_trace(logger_mongo, "Recurso: %s", path);
	for(int i = 0; i< list_size(lista_bloques); i++){
		aux = malloc(sizeof(int));
		aux = list_get(lista_bloques, i);
		log_trace(logger_mongo, "EL recurso tiene asignado : %i", *aux);
	}
	*/

	iniciar_archivo_recurso(path, tamanio, cantidad_bloques + 1, lista_bloques);
}

void asignar_bloque_tripulante(char* path, int* pos_libre, int size_agregado) {

	uint32_t tamanio = tamanio_archivo(path);
	t_list* lista_bloques = obtener_lista_bloques(path);

	log_debug(logger_mongo, "Tamanio de la lista antes de agregar: %i", list_size(lista_bloques));

	list_add(lista_bloques, pos_libre);
	log_debug(logger_mongo, "Tamanio de la lista luego de agregar: %i", list_size(lista_bloques));

//	escribir_archivo_tripulante (path, tamanio + size_agregado, lista_bloques);
	escribir_archivo_tripulante (path, tamanio, lista_bloques);
	log_info(logger_mongo, "Tamanio pre-agregar: %i", tamanio);
	log_info(logger_mongo, "Tamanio agregado: %i", size_agregado);
	log_debug(logger_mongo, "fin asignar_bloque_tripulante");
}

uint32_t bloques_contar(char caracter) {
	int cantidad = 0;

	for(int i = 0 ; i < CANTIDAD_BLOQUES; i++){
		if (directorio.mapa_blocks[i] == caracter && directorio.mapa_blocks[i+1] == caracter) // No haria esta comparacion, bloque puede ser de archivo y estar vacio
			cantidad++;
	}
	return cantidad;
}


char* crear_puntero_a_bitmap(){
	// EL RETORNO SE DEBE LIBERAR
	char* puntero_a_bitmap = malloc(CANTIDAD_BLOQUES / 8);
	fseek(directorio.superbloque, sizeof(uint32_t)*2, SEEK_SET);
	fread(puntero_a_bitmap, CANTIDAD_BLOQUES/8, 1, directorio.superbloque);
	return puntero_a_bitmap;
}

void limpiar_metadata(char* path) {
	iniciar_archivo_recurso(path, 0, 0, NULL);
}


void liberar_bloques(char* path) { //TODO llenar bloque de ','

	t_list* bloques = obtener_lista_bloques(path);
	uint32_t* nro_bloque = malloc(sizeof(uint32_t));

	for(int i = 0; i < list_size(bloques) ; i++) {
		nro_bloque = list_get(bloques, i);
		liberar_bloque(path, *nro_bloque);
		// rellenar_bloque(); // TODO llenar de comas
	}
}

void liberar_bloque(char* path, uint32_t nro_bloque) {
	t_list* bloques = obtener_lista_bloques(path);

	uint32_t* nro_bloque_aux = malloc(sizeof(uint32_t));

	t_bitarray* nuevo_bitmap = obtener_bitmap();

	for(int i = 0; i < list_size(bloques) ; i++) {
		nro_bloque_aux = list_get(bloques, i);
		if (nro_bloque == *nro_bloque_aux) {
			bitarray_clean_bit(nuevo_bitmap, nro_bloque);
			reescribir_superbloque(TAMANIO_BLOQUE, CANTIDAD_BLOQUES, nuevo_bitmap);

			bool quitar_bloque(void* elemento1){
				return (nro_bloque == *((int*) elemento1));
			}

			list_remove_by_condition(bloques, quitar_bloque);

			set_bloq(path, bloques);
			set_cant_bloques(path, cantidad_bloques_recurso(path) - 1);
		}
	}
	// liberar lista
	free(nro_bloque_aux);
}

void set_tam(char* path, int tamanio){

//	uint32_t tamanio_viejo = tamanio_archivo(path); rompe
//	log_trace(logger_mongo, "Tamanio sin actualizar: %i", tamanio_viejo);

	t_config* config = config_create(path);
	config_save_in_file(config, path);
	config_set_value(config, "SIZE", string_itoa(tamanio));
	config_save(config);
	config_destroy(config);

	log_trace(logger_mongo, "Tamanio actualizado: %i", tamanio);
}

void set_bloq(char* path, t_list* lista){

	t_config* config = config_create(path);

	log_debug(logger_mongo, "la lista de bloques es %s", config_get_string_value(config, "BLOCKS"));

	t_list* list_aux;
	if(lista == NULL){
		list_aux = list_create();
	} else{
		t_list* nueva = lista;
		list_aux = list_duplicate(nueva);
	}

	int* aux;
	char* lista_bloques;

	/*
	if(config_has_property(config, "BLOCKS")){

		char** valores_viejos = config_get_array_value(config, "BLOCKS");
		if(valores_viejos != NULL){
			// log_trace(logger_mongo, "valores viejos: %s", config_get_string_value(config, "BLOCKS"));
			for(int i = 0; i < contar_palabras(valores_viejos); i++){
				// log_trace(logger_mongo, "valores_viejos %s", valores_viejos[i]);
				aux = malloc(sizeof(int));
				*aux = atoi(valores_viejos[i]);
				list_add(list_aux, aux);
			}
		} else{
			// log_trace(logger_mongo, "no habian valores viejos");
		}

		// log_error(logger_mongo, "printeando valores viejos");
		// for(int i = 0; i < list_size(list_aux); i++){
			// aux = list_get(list_aux, i);
			// log_error(logger_mongo, "valor %i, %i", i, *aux);
		// }

	}*/

	if(list_aux == NULL || list_is_empty(list_aux)){
	    // log_trace(logger_mongo, "empty");

		lista_bloques = malloc(2 + 1);
		strcpy(lista_bloques, "[]");

	} else {

		// log_trace(logger_mongo, "Not empty %i", list_is_empty(list_aux));
		int comas = max(list_size(list_aux)-1, 0);
		int cant_numeros = 0;

		for(int i = 0; i < list_size(list_aux); i++){
			aux = list_get(list_aux, i);
			cant_numeros += strlen(string_itoa(*aux));
		}

		lista_bloques = malloc(2 + cant_numeros + comas + 1);
		strcpy(lista_bloques, "[");

		// log_error(logger_mongo, "printeando lista");
		// for(int i = 0; i < list_size(list_aux); i++){
			// aux = list_get(list_aux, i);
			// log_error(logger_mongo, "valor %i, %i", i, *aux);
		// }

		for(int i = 0; i < list_size(list_aux); i++){
			aux = list_get(list_aux, i);
			strcat(lista_bloques, string_itoa(*aux));

			if(i+1 < list_size(list_aux)){ // si hay otra repeticion, meto una coma
				strcat(lista_bloques, ",");
			}
		}
		strcat(lista_bloques, "]");

	}

	config_set_value(config, "BLOCKS", lista_bloques);
	// log_debug(logger_mongo, "la lista de bloques queda %s", config_get_string_value(config, "BLOCKS"));

	config_save_in_file(config, path);
	config_destroy(config);

	liberar_lista(list_aux);

	free(lista_bloques);

}


void set_cant_bloques(char* path, int cant){

	t_config* config = config_create(path);
	config_save_in_file(config, path);
	config_set_value(config, "BLOCK_COUNT", string_itoa(cant));
	config_save(config);
	config_destroy(config);

}

void set_caracter_llenado(char* path, char caracter){

	t_config* config = config_create(path);
	config_save_in_file(config, path);

	char* caracter_string = malloc(sizeof(char)*2);
	*caracter_string = caracter;
	*(caracter_string+1) = '\0';

	config_set_value(config, "CARACTER_LLENADO", caracter_string);
	config_save(config);
	config_destroy(config);
	free(caracter_string);

}

void set_md5(char* path, char* md5){

	t_config* config = config_create(path);
	config_save_in_file(config, path);
	config_set_value(config, "MD5_ARCHIVO", md5);
	config_save(config);
	config_destroy(config);

}



