#include "mongo_archivos.h"


// Vars globales
t_log* logger_mongo;
t_config* config_mongo;
t_list* bitacoras;
int existe_oxigeno = 0;
int existe_comida = 0;
int existe_basura = 0;

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

	int filedescriptor_blocks = open(path_blocks, O_RDWR | O_APPEND | O_CREAT, (mode_t) 0777);

	// Trunco los archivos o los creo en modo escritura y lectura
	// Se guarda to.do en un struct para uso en distintas funciones
	directorio.superbloque = fopen(path_superbloque, "w+b");
	directorio.blocks      = fdopen(filedescriptor_blocks, "w+b");

	iniciar_superbloque(directorio.superbloque);
	iniciar_blocks(filedescriptor_blocks);
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

	if((recurso.oxigeno = fopen(path_oxigeno, "r+b")) != NULL){
		log_info(logger_mongo, "Existe oxigeno");
		existe_oxigeno = 1;
	} else{
		log_info(logger_mongo, "No existe oxigeno");
		existe_oxigeno = 0;
	}

	if((recurso.comida = fopen(path_comida, "r+b")) != NULL){
		log_info(logger_mongo, "Existe comida");
		existe_comida = 1;
	} else{
		log_info(logger_mongo, "No existe comida");
		existe_comida = 0;
	}

	if((recurso.basura = fopen(path_basura, "r+b")) != NULL){
		log_info(logger_mongo, "Existe basura");
		existe_basura = 1;
	} else{
		log_info(logger_mongo, "No existe basura");
		existe_basura = 0;
	}

	directorio.superbloque = fopen(path_superbloque, "r+b");
	directorio.blocks      = fdopen(filedescriptor_blocks, "r+b");

	iniciar_blocks(filedescriptor_blocks);
	// mapea y sincroniza
	memcpy(directorio.mapa_blocks, mapa, CANTIDAD_BLOQUES * TAMANIO_BLOQUE);
    msync(mapa, CANTIDAD_BLOQUES * TAMANIO_BLOQUE, MS_ASYNC);

    cargar_bitmap(); // cargando la lista de bloqueados
    // limpiar_cuerpos();
    // rename("/home/utnso/polus/Files/Bitacoras/Tripulante10001.ims", "/home/utnso/polus/Files/Bitacoras/OldTripulante10001.ims");
    //rename("/home/utnso/polus/Files/Bitacoras", "/home/utnso/polus/Files/ASID");

    log_info(logger_mongo, "Inicializados archivos pre existentes.");
}

void limpiar_cuerpos() {
	path_bitacoras = malloc((strlen(path_files)+1) + strlen("/Bitacoras"));
	sprintf(path_bitacoras, "%s/Bitacoras", path_files);
	log_info(logger_mongo, "Limpiando cuerpos");
	log_info(logger_mongo, "Path bitácoras: %s", path_bitacoras);
	int renombrado = 0;

	for (int i = 0; renombrado == 0; i++) {
		char* nuevo_directorio = malloc(strlen(path_bitacoras) + 1);
		strcpy(nuevo_directorio, path_bitacoras);
		char* aux = string_itoa(i);
		strcat(nuevo_directorio, aux);
		DIR* dir = opendir(nuevo_directorio);

		//Si el directorio no existe
		if (!dir) {
			rename(path_bitacoras, nuevo_directorio);

			strncpy(path_bitacoras, path_files, strlen(path_files) + 1);
			path_bitacoras = strcat(path_bitacoras, "/Bitacoras");
			mkdir(path_bitacoras, (mode_t) 0777);
			renombrado = 1;
		}
		free(aux);
		free(nuevo_directorio);
	}
}

void asignar_nuevo_bloque(char* path, int size_agregado) {
	// log_warning(logger_mongo, "sincronizar inicio");
	lockearEscritura(path_blocks);
	log_trace(logger_mongo, "Asignando un nuevo bloque");

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
	    log_trace(logger_mongo, "Actualizado");
	}
	else{
		log_info(logger_mongo, "No hay bloques disponibles en este momento");
	}

	unlockear(path_blocks);
	// log_warning(logger_mongo, "sincronizar fin");

	free(bitmap->bitarray);
	bitarray_destroy(bitmap);

}

int llenar_bloque_recurso(t_list* lista_bloques, int cantidad_deseada, char tipo, char* path) {

	log_trace(logger_mongo, "0 asignar_primer_bloque, cant bloques %i", list_size(lista_bloques));
	int cantidad_alcanzada = 0;

	if(list_is_empty(lista_bloques) && cantidad_deseada != 0){
		log_trace(logger_mongo, "la lista ta vacia");
		asignar_nuevo_bloque(path, 0);
		lista_bloques = get_lista_bloques(path);
	}

	int* aux;

	lockearEscritura(path_blocks);

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
				unlockear(path_blocks);
				return 0;
			}
		}
	}
	unlockear(path_blocks);

	return cantidad_alcanzada - cantidad_deseada;
}

int quitar_ultimo_bloque_libre(t_list* lista_bloques, int cantidad_deseada, char tipo) {

	log_trace(logger_mongo, "INICIO quitar_ultimo_bloque, cant bloques %i", list_size(lista_bloques));

	int cantidad_alcanzada = 0;
	int* aux;

	char* path = tipo_a_path(tipo);

	for(int i = (list_size(lista_bloques) - 1); i >= 0 ; i--){

		aux = list_get(lista_bloques, i);

		for(int j = 0; j < TAMANIO_BLOQUE; j++){
			lockearEscritura(path_blocks);
			if (*(directorio.mapa_blocks + (*aux + 1) * TAMANIO_BLOQUE - j) == tipo) {
				*(directorio.mapa_blocks + (*aux + 1) * TAMANIO_BLOQUE - j) = ',';
				cantidad_alcanzada++;

				if(j == TAMANIO_BLOQUE-1){
					liberar_bloque(path, *aux);
				}
			}
			unlockear(path_blocks);

			if (cantidad_alcanzada == cantidad_deseada) {
				return 0;
			}
		}
	}
	
	return cantidad_alcanzada - cantidad_deseada;
}

int existe_archivo(int codigo_archivo) {

	switch(codigo_archivo) {
		case OXIGENO:
			return existe_oxigeno;
		case COMIDA:
			return existe_comida;
		case BASURA:
			return existe_basura;
	}
	return -1;
}

void alterar(int codigo_archivo, int cantidad) {  
	log_trace(logger_mongo, "INICIO alterar, codigo = %i cantidad = %i", codigo_archivo, cantidad);

	char* path = conseguir_path_recurso_codigo(codigo_archivo);
	if (!existe_archivo(codigo_archivo)) {
		log_trace(logger_mongo, "Inicializando archivos recurso");
		switch(codigo_archivo) {
			case OXIGENO:
				existe_oxigeno = 1;
				recurso.oxigeno = fopen(path_oxigeno, "w+b");
				log_trace(logger_mongo, "Se creo el archivo de oxigeno");
				break;
			case COMIDA:
				existe_comida = 1;
				recurso.comida  = fopen(path_comida, "w+b");
				log_trace(logger_mongo, "Se creo el archivo de comida");
				break;
			case BASURA:
				existe_basura = 1;
				recurso.basura  = fopen(path_basura, "w+b");
				log_trace(logger_mongo, "Se creo el archivo de basura");
				break;
		}
		iniciar_archivo_recurso(path, 0, 0, NULL);
	}

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

	log_trace(logger_mongo, "Nuestro path es %s", path);

	t_list* lista_bloques = get_lista_bloques(path);
	char tipo = caracter_llenado_archivo(path);

	int offset = llenar_bloque_recurso(lista_bloques, cantidad, tipo, path);

	if (offset < 0) { // Falto agregar cantidad, dada por offset
		asignar_nuevo_bloque(path, 0);
		agregar(codigo_archivo, offset * (-1)); // Recursividad con la cantidad que falto
	}

	uint32_t tam_archivo = tamanio_archivo(path);
	uint32_t cant_bloques = cantidad_bloques_recurso(path);
	lista_bloques = get_lista_bloques(path);

	iniciar_archivo_recurso(path, tam_archivo + cantidad + offset, cant_bloques, lista_bloques);

	log_trace(logger_mongo, "FIN agregar, cantidad agregada: %i", cantidad);

}

void quitar(int codigo_archivo, int cantidad) {
	log_trace(logger_mongo, "INICIO quitar");
	char* path = conseguir_path_recurso_codigo(codigo_archivo);

	uint32_t tam_archivo = tamanio_archivo(path);
	uint32_t cant_bloques = cantidad_bloques_recurso(path);
	t_list* lista_bloques = get_lista_bloques(path);
	char tipo = caracter_llenado_archivo(path);

	quitar_ultimo_bloque_libre(lista_bloques, cantidad, tipo);

	iniciar_archivo_recurso(path, tam_archivo - cantidad, cant_bloques, lista_bloques);

	log_warning(logger_mongo, "FIN quitar");
}

char* tipo_a_path(char tipo){
	switch(tipo) {
		case 'O':
			return path_oxigeno;
			break;
		case 'C':
			return path_comida;
			break;
		case 'B':
			return path_basura;
			break;
	}
	return NULL;
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
		// log_warning(logger_mongo, "lockear tamaño");
		lockearLectura(path);
	}
	t_config* config = config_create(path);
	int tam_archivo = config_get_int_value(config, "SIZE");
	config_destroy(config);
	if(es_recurso(path)) {
		// log_warning(logger_mongo, "unlockear tamaño");
		unlockear(path);
	}
	return tam_archivo;
}

uint32_t cantidad_bloques_recurso(char* path) {
	// log_warning(logger_mongo, "lockear cantidad bloques");
	lockearLectura(path);

	t_config* config = config_create(path);
	int cant_bloques = config_get_int_value(config, "BLOCK_COUNT");
	config_destroy(config);

	// log_warning(logger_mongo, "unlockear cantidad bloques");
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

	return out;
}

uint32_t obtener_cantidad_bloques(char* path){
	lockearLectura(path);
	// log_warning(logger_mongo, "Lockeo cantidad bloques recurso");

	t_config* config = config_create(path);

	char** bloques = config_get_array_value(config, "BLOCKS");
	int cant_bloques = contar_palabras(bloques);
	config_destroy(config);
	liberar_puntero_doble(bloques);

	unlockear(path);
	// log_warning(logger_mongo, "Unlockeo cantidad bloques recurso");

	return cant_bloques;
}

t_list* get_lista_bloques(char* path){
	// ESTA FUNCION DEBE LIBERAR EL RETORNO

	log_trace(logger_mongo, "Obteniendo la lista de bloques de %s", path);
	if(es_recurso(path)){
		lockearLectura(path);
	}

	t_config* config = config_create(path);
	t_list* lista_bloques = list_create();

	char** bloques = config_get_array_value(config, "BLOCKS");

	if(bloques[0] == NULL){

		log_error(logger_mongo, "EL path no tiene bloques");
		config_destroy(config);
		liberar_puntero_doble(bloques);
		if(es_recurso(path)){
			unlockear(path);
		}
		return lista_bloques;
	}

	int* aux;

	for(int i = 0; i < contar_palabras(bloques); i++){
		aux = malloc(sizeof(int));
		*aux = atoi(bloques[i]);
		list_add(lista_bloques, aux);
	}

	// log_warning(logger_mongo, "unlockear bloques");
	config_destroy(config);
	liberar_puntero_doble(bloques);

	if(es_recurso(path)){
		unlockear(path);
	}

	log_error(logger_mongo, "Retorno la lista de bloques");
	return lista_bloques;
}

void iniciar_archivo_recurso(char* path, int tamanio, int cant_bloques, t_list* lista_bloques){

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

}

void escribir_archivo_tripulante(char* path, uint32_t tamanio, t_list* list_bloques) {

	set_tam(path, tamanio);
	set_bloq(path, list_bloques);

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
	t_list* lista_bloques = get_lista_bloques(path);

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
	t_list* lista_bloques = get_lista_bloques(path);

	// log_debug(logger_mongo, "Tamanio de la lista antes de agregar: %i", list_size(lista_bloques));
	list_add(lista_bloques, pos_libre);
	// log_debug(logger_mongo, "Tamanio de la lista luego de agregar: %i", list_size(lista_bloques));

	escribir_archivo_tripulante (path, tamanio, lista_bloques);
}

uint32_t bloques_contar(char caracter) {

//	log_trace(logger_mongo, "Contando ando");
	int cantidad = 0;

	char* path = tipo_a_path(caracter);
//	log_trace(logger_mongo, "tengo el path");
	t_list* bloques = get_lista_bloques(path);
//	log_trace(logger_mongo, "tengo los bloques");
	int* aux;

	// log_warning(logger_mongo, "Lockeo blocks para contar bloques");
	lockearLectura(path_blocks);
//	log_trace(logger_mongo, "lockeado el path_blocks");

	for(int i = 0 ; i < list_size(bloques); i++){
		aux = list_get(bloques, i);
		for(int j = 0 ; j < TAMANIO_BLOQUE; j++){
			if (*(directorio.mapa_blocks + *aux * TAMANIO_BLOQUE + j) == caracter){
				cantidad++;
			}
		}

	}

	// log_warning(logger_mongo, "Unlockeo blocks para contar bloques");
	unlockear(path_blocks);
//	log_trace(logger_mongo, "Ya conté");
	return cantidad;
}


char* crear_puntero_a_bitmap(){
	// EL RETORNO SE DEBE LIBERAR
	char* puntero_a_bitmap = malloc(CANTIDAD_BLOQUES / 8);
	// log_warning(logger_mongo, "Lockeo superbloque para crear puntero a bitmap");
	lockearLectura(path_superbloque);
	fseek(directorio.superbloque, sizeof(uint32_t)*2, SEEK_SET);
	fread(puntero_a_bitmap, CANTIDAD_BLOQUES/8, 1, directorio.superbloque);
	unlockear(path_superbloque);
	// log_warning(logger_mongo, "Unlockeo superbloque para crear puntero a bitmap");
	return puntero_a_bitmap;
}

void limpiar_metadata(char* path) {
	iniciar_archivo_recurso(path, 0, 0, NULL);
}


void liberar_bloques(char* path) {

	t_list* bloques = get_lista_bloques(path);
	int* nro_bloque;
	for(int i = 0; i < list_size(bloques) ; i++) {
		nro_bloque = list_get(bloques, i);
		liberar_bloque(path, *nro_bloque);
		blanquear_bloque(*nro_bloque);
	}
}

void blanquear_bloque(int bloque){
	// log_warning(logger_mongo, "lockeo path blocks");
	lockearEscritura(path_blocks);
	for(int i = 0; i < TAMANIO_BLOQUE; i++){
		*(directorio.mapa_blocks + bloque * TAMANIO_BLOQUE + i) = ',';
	}
	unlockear(path_blocks);
	// log_warning(logger_mongo, "unlockeo path blocks");
}

void liberar_bloque(char* path, uint32_t nro_bloque) {
	t_list* bloques = get_lista_bloques(path);

	uint32_t* nro_bloque_aux;
	t_bitarray* nuevo_bitmap = obtener_bitmap();

	for(int i = 0; i < list_size(bloques) ; i++) {
		nro_bloque_aux = list_get(bloques, i);
		if (nro_bloque == *nro_bloque_aux) {
			bitarray_clean_bit(nuevo_bitmap, nro_bloque);

			reescribir_bitmap(nuevo_bitmap);

			bool quitar_bloque(void* elemento1){
				return (nro_bloque == *((int*) elemento1));
			}

			list_remove_by_condition(bloques, quitar_bloque);

			if(es_recurso(path)){
				set_bloq(path, bloques);
				set_cant_bloques(path, list_size(bloques) - 1);
			}
		}
	}

	bool quitar_bloque_de_lista(void* bloque){
		return *((int*) bloque) == nro_bloque;
	}

	list_remove_by_condition(lista_bloques_ocupados, quitar_bloque_de_lista);
	// liberar lista
	free(nuevo_bitmap->bitarray);
	bitarray_destroy(nuevo_bitmap);
}

void set_tam(char* path, int tamanio){

	lockearEscritura(path);
	// log_warning(logger_mongo, "Lockeo set_tam");

	t_config* config = config_create(path);
	config_save_in_file(config, path);
	char* aux = string_itoa(tamanio);
	config_set_value(config, "SIZE", aux);
	free(aux);
	config_save(config);
	config_destroy(config);

	unlockear(path);
	// log_warning(logger_mongo, "Unlockeo set_tam");
}

void set_bloq(char* path, t_list* lista){

	if(es_recurso(path)){
		lockearEscritura(path);
	}

	// log_warning(logger_mongo, "Lockeo set_bloq");

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
			char* string_aux = string_itoa(*aux);
			cant_numeros += strlen(string_aux);
			free(string_aux);
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
			char* string_aux = string_itoa(*aux);
			strcat(lista_bloques, string_aux);
			free(string_aux);

			if(i+1 < list_size(list_aux)){
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
	if(es_recurso(path)){
		unlockear(path);
	}
	// log_warning(logger_mongo, "Unlockeo set_bloq");

}


void set_cant_bloques(char* path, int cant){

	lockearEscritura(path);
	// log_warning(logger_mongo, "Lockeo set_cant_bloques");

	t_config* config = config_create(path);
	config_save_in_file(config, path);
	config_set_value(config, "BLOCK_COUNT", string_itoa(cant));
	config_save(config);
	config_destroy(config);

	unlockear(path);
	// log_warning(logger_mongo, "Unlockeo set_cant_bloques");

}

void set_caracter_llenado(char* path, char caracter){
	lockearEscritura(path);
	t_config* config = config_create(path);
	config_save_in_file(config, path);

	char* caracter_string = malloc(sizeof(char)*2);
	*caracter_string = caracter;
	*(caracter_string+1) = '\0';

	config_set_value(config, "CARACTER_LLENADO", caracter_string);
	config_save(config);
	config_destroy(config);
	free(caracter_string);
	unlockear(path);
}

void set_md5(char* path, char* md5){

	lockearEscritura(path);
	// log_warning(logger_mongo, "Lockeo set_md5");

	t_config* config = config_create(path);
	config_save_in_file(config, path);
	config_set_value(config, "MD5_ARCHIVO", md5);
	config_save(config);
	config_destroy(config);

	unlockear(path);
	// log_warning(logger_mongo, "Unlockeo set_md5");
}
