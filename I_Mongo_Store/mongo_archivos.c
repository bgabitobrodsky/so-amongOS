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

	log_trace(logger_mongo, "Se configuraron los paths.");

	int filedescriptor_blocks = open(path_blocks, O_RDWR | O_APPEND | O_CREAT, (mode_t) 0777);
	int filedescriptor_superbloque = open(path_superbloque, O_RDWR | O_APPEND | O_CREAT, (mode_t) 0777);

	// Trunco los archivos o los creo en modo escritura y lectura
	// Se guarda to.do en un struct para uso en distintas funciones

	directorio.superbloque = fdopen(filedescriptor_superbloque, "w+b");
	directorio.blocks      = fdopen(filedescriptor_blocks, "w+b");

	log_trace(logger_mongo, "Se crearon los archivos de Blocks y Superbloque.");

	iniciar_superbloque_fd(filedescriptor_superbloque);
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

	log_trace(logger_mongo, "Se configuraron los paths.");

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

	int filedescriptor_blocks = open(path_blocks, O_RDWR | O_APPEND | O_CREAT, (mode_t) 0777);
	int filedescriptor_superbloque = open(path_superbloque, O_RDWR | O_APPEND | O_CREAT, (mode_t) 0777);

	directorio.superbloque = fdopen(filedescriptor_superbloque, "r+b");
	directorio.blocks      = fdopen(filedescriptor_blocks, "r+b");

	log_trace(logger_mongo, "Se abrieron los archivos del FileSystem.");

	iniciar_blocks(filedescriptor_blocks);
	// mapea y sincroniza
	memcpy(directorio.mapa_blocks, mapa, CANTIDAD_BLOQUES * TAMANIO_BLOQUE);
    msync(mapa, CANTIDAD_BLOQUES * TAMANIO_BLOQUE, MS_ASYNC);

    directorio.supermapa = (void*) mmap(NULL, sizeof(uint32_t) * 2 + CANTIDAD_BLOQUES / 8, PROT_READ | PROT_WRITE, MAP_SHARED, filedescriptor_superbloque, 0);
    cargar_bitmap(); // cargando la lista de bloqueados
    // limpiar_cuerpos();
    // rename("/home/utnso/polus/Files/Bitacoras/Tripulante10001.ims", "/home/utnso/polus/Files/Bitacoras/OldTripulante10001.ims");
    // rename("/home/utnso/polus/Files/Bitacoras", "/home/utnso/polus/Files/ASID");

    log_info(logger_mongo, "Se inicializaron los archivos pre-existentes.");
}

void limpiar_cuerpos() {
	path_bitacoras = malloc((strlen(path_files)+1) + strlen("/Bitacoras"));
	sprintf(path_bitacoras, "%s/Bitacoras", path_files);
	log_trace(logger_mongo, "Limpiando cuerpos");
	log_trace(logger_mongo, "Path bitácoras: %s", path_bitacoras);
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
		log_trace(logger_mongo, "El bit libre es = %i", *pos_libre);
		//Marco el bit como ocupado
		bitarray_set_bit(bitmap, *pos_libre);

		if (es_recurso(path)){
			log_trace(logger_mongo, "Se asigna un bloque a un recurso");
			asignar_bloque_recurso(path, pos_libre);
		}
		else {
			log_trace(logger_mongo, "Se asigna un bloque a un tripulante");
			asignar_bloque_tripulante(path, pos_libre, size_agregado);
		}

		monitor_lista(sem_lista_bloques_ocupados, (void*) list_add, lista_bloques_ocupados, pos_libre);
		actualizar_bitmap(lista_bloques_ocupados);
	    log_trace(logger_mongo, "Actualizado el bitmap");
	}
	else{
		log_warning(logger_mongo, "No hay bloques disponibles en este momento");
	}

	unlockear(path_blocks);

	free(bitmap->bitarray);
	bitarray_destroy(bitmap);

}

int llenar_bloque_recurso(int cantidad_deseada, char tipo, char* path) {
	sem_wait(&sem_llenar_bloque_recurso);
	log_trace(logger_mongo, "Llenando el bloque de recurso");
	t_list* lista_bloques = get_lista_bloques(path);
	log_trace(logger_mongo, "Cant bloques %i", list_size(lista_bloques));
	int cantidad_alcanzada = 0;

	if(list_is_empty(lista_bloques) && cantidad_deseada != 0){
		log_trace(logger_mongo, "Se quiere destruir la lista.1");
		list_destroy(lista_bloques);
		log_trace(logger_mongo, "la lista de bloques esta vacia");
		asignar_nuevo_bloque(path, 0);
		lista_bloques = get_lista_bloques(path);
	}

	int* aux;

	for(int i = 0; i < list_size(lista_bloques); i++){
		aux = list_get(lista_bloques, i);
		log_trace(logger_mongo, "Se esta agregando en bloque %i", *aux);

		for(int j = 0; j < TAMANIO_BLOQUE; j++){

			if (*(directorio.mapa_blocks + *aux * TAMANIO_BLOQUE + j) == ',') {
				*(directorio.mapa_blocks + *aux * TAMANIO_BLOQUE + j) = tipo;

				log_trace(logger_mongo, "Se agrego char en posicion %i", j);

				cantidad_alcanzada++;
			}

			if (cantidad_alcanzada == cantidad_deseada) {
				log_trace(logger_mongo, "Se llego a la cantidad deseada.");
				log_trace(logger_mongo, "Se quiere destruir la lista.2");
				matar_lista(lista_bloques);
				sem_post(&sem_llenar_bloque_recurso);
				return 0;
			}
		}
	}

	sem_post(&sem_llenar_bloque_recurso);
	log_trace(logger_mongo, "Se quiere destruir la lista.3");
	matar_lista(lista_bloques);
	return cantidad_alcanzada - cantidad_deseada;
}

int quitar_ultimo_bloque_libre(int cantidad_deseada, char tipo) {

	sem_wait(&sem_quitar_ultimo_bloque_libre);
	log_trace(logger_mongo, "Quitando del bloque de recurso");

	int cantidad_alcanzada = 0;
	int* aux;

	char* path = tipo_a_path(tipo);
	t_list* lista_bloques = get_lista_bloques(path);
	log_trace(logger_mongo, "Cant bloques %i", list_size(lista_bloques));

	for(int i = (list_size(lista_bloques) - 1); i >= 0 ; i--){

		aux = list_get(lista_bloques, i);

		for(int j = 0; j < TAMANIO_BLOQUE; j++){
			if (*(directorio.mapa_blocks + (*aux + 1) * TAMANIO_BLOQUE - j) == tipo) {
				*(directorio.mapa_blocks + (*aux + 1) * TAMANIO_BLOQUE - j) = ',';
				cantidad_alcanzada++;

				if(j == TAMANIO_BLOQUE-1){
					liberar_bloque(path, *aux);
				}
			}

			if (cantidad_alcanzada == cantidad_deseada) {
				sem_post(&sem_quitar_ultimo_bloque_libre);
				matar_lista(lista_bloques);
				return 0;
			}
		}
	}

	sem_post(&sem_quitar_ultimo_bloque_libre);
	matar_lista(lista_bloques);
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
	log_trace(logger_mongo, "Por alterar archivo recurso.");
	log_trace(logger_mongo, "Codigo = %i, Cantidad = %i", codigo_archivo, cantidad);

	if (cantidad > 0){

		char* path = conseguir_path_recurso_codigo(codigo_archivo);
		if (!existe_archivo(codigo_archivo)) {
			log_trace(logger_mongo, "Inicializando archivos recurso");

				switch(codigo_archivo) {
				case OXIGENO:
					existe_oxigeno = 1;
					recurso.oxigeno = fopen(path_oxigeno, "w+b");
					log_info(logger_mongo, "Se creo el archivo de oxigeno.");
					break;
				case COMIDA:
					existe_comida = 1;
					recurso.comida  = fopen(path_comida, "w+b");
					log_info(logger_mongo, "Se creo el archivo de comida.");
					break;
				case BASURA:
					existe_basura = 1;
					recurso.basura  = fopen(path_basura, "w+b");
					log_info(logger_mongo, "Se creo el archivo de basura.");
					break;
				}

				iniciar_archivo_recurso(path, 0, 0, NULL);
			}

		agregar(codigo_archivo, cantidad);
		log_info(logger_mongo, "Se agregaron %i unidades a %s.", cantidad, conseguir_tipo(conseguir_char(codigo_archivo)));
	}
	else {

		if (!existe_archivo(codigo_archivo)) {
			log_warning(logger_mongo, "El archivo del que se quiere quitar no existe.");
		}
		else {

			if (codigo_archivo == BASURA) {
				descartar_basura();
			}
			else {
				quitar(codigo_archivo, cantidad * (-1));
				log_info(logger_mongo, "Se quitaron %i unidades a %s.", (cantidad * (-1)) , conseguir_tipo(conseguir_char(codigo_archivo)));
			}
		}

	}
}

void descartar_basura() {
	// Liberar los bloques de basura
	/*liberar_bloques(path_basura);
	// Eliminar el archivo
	fclose(recurso.basura);
	remove(path_basura);
	existe_basura = 0;
	log_info(logger_mongo, "Se elimino el archivo Basura.");*/
}

void agregar(int codigo_archivo, int cantidad) { // Puede que haya que hacer mallocs previos
	log_trace(logger_mongo, "Por agregar a archivo recurso.");
	log_trace(logger_mongo, "Codigo: %i, Cantidad %i", codigo_archivo, cantidad);

	char* path = conseguir_path_recurso_codigo(codigo_archivo);
	log_trace(logger_mongo, "El path del recurso es %s", path);

	char tipo = caracter_llenado_archivo(path);

	int offset = llenar_bloque_recurso(cantidad, tipo, path);

	if (offset < 0) { // Falto agregar cantidad, dada por offset
		asignar_nuevo_bloque(path, 0);
		agregar(codigo_archivo, offset * (-1)); // Recursividad con la cantidad que falto
	}
	// log_trace(logger_mongo, "Se intenta matar lista");
	// matar_lista(lista_bloques);
	// log_trace(logger_mongo, "Se mato lista");

	iniciar_archivo_recurso2(path, cantidad + offset, 0);

	log_trace(logger_mongo, "Se agregaron: %i", cantidad);
}

void quitar(int codigo_archivo, int cantidad) {
	log_trace(logger_mongo, "Por quitar a archivo recurso.");
	log_trace(logger_mongo, "Codigo: %i, Cantidad %i", codigo_archivo, cantidad);

	char* path = conseguir_path_recurso_codigo(codigo_archivo);
	log_trace(logger_mongo, "El path del recurso es %s", path);

	char tipo = caracter_llenado_archivo(path);

	int resultado = quitar_ultimo_bloque_libre(cantidad, tipo);

	if (resultado != 0) {
		log_info(logger_mongo, "Se intento quitar mas de lo ya existente en el archivo.");
		iniciar_archivo_recurso(path, 0, 0, NULL);
		return;
	}

	iniciar_archivo_recurso2(path, -cantidad, 1);
	log_trace(logger_mongo, "Se quitaron: %i", cantidad);
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
		lockearLectura(path);
	}

	t_config* config = config_create(path);
	int tam_archivo = config_get_int_value(config, "SIZE");
	config_destroy(config);

	if(es_recurso(path)) {
		unlockear(path);
	}

	return tam_archivo;
}

int cantidad_bloques_recurso(char* path) {

	lockearLectura(path);

	t_config* config = config_create(path);
	int cant_bloques = config_get_int_value(config, "BLOCK_COUNT");
	config_destroy(config);

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
	char* out = (char*)malloc(33);

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

	t_config* config = config_create(path);
	char** bloques = config_get_array_value(config, "BLOCKS");
	int cant_bloques = contar_palabras(bloques);
	config_destroy(config);
	liberar_puntero_doble(bloques);

	unlockear(path);

	return cant_bloques;
}

t_list* get_lista_bloques(char* path){
	// ESTA FUNCION DEBE LIBERAR EL RETORNO

	log_trace(logger_mongo, "Obteniendo la lista de bloques de %s", path);

	if(es_recurso(path)){
		lockearLectura(path);
	}

	log_trace(logger_mongo, "Pre config_create");
	t_config* config = config_create(path);
	log_trace(logger_mongo, "Pre list_create");
	t_list* lista_bloques = list_create();

	log_trace(logger_mongo, "config_get_array_value");
	char** bloques = config_get_array_value(config, "BLOCKS");

	if(bloques[0] == NULL){

		log_trace(logger_mongo, "EL path dado no tiene bloques.");
		config_destroy(config);
		liberar_puntero_doble(bloques);

		if(es_recurso(path)){
			unlockear(path);
		}

		return lista_bloques;
	}

	int* aux;

	log_trace(logger_mongo, "for");
	for(int i = 0; i < contar_palabras(bloques); i++){
		aux = malloc(sizeof(int));
		*aux = atoi(bloques[i]);
		list_add(lista_bloques, aux);
	}

	log_trace(logger_mongo, "post for");
	config_destroy(config);
	liberar_puntero_doble(bloques);

	log_trace(logger_mongo, "if");
	if(es_recurso(path)){
		unlockear(path);
	}

	log_trace(logger_mongo, "Se devuelve la lista de bloques");
	return lista_bloques;
}

void iniciar_archivo_recurso2(char* path, int tamanio, int cant_bloques_a_agregar) {

	log_trace(logger_mongo, "Inicio iniciar_archivo_recurso2");
	log_warning(logger_mongo, "RECURSO");
	if(tamanio >= 0)
		agregar_tam(path, tamanio);
	else
		quitar_tam(path, tamanio);

	int cant_bloques = cantidad_bloques_recurso(path);
	set_cant_bloques(path, cant_bloques + cant_bloques_a_agregar);
	t_list* lista_bloques = get_lista_bloques(path);
	set_bloq(path, lista_bloques);

	char caracter = caracter_llenado_archivo(path);
	log_warning(logger_mongo, "POST LLENADO");
	set_caracter_llenado(path, caracter);

	/*if(cant_bloques != 0){
		log_warning(logger_mongo, "PRE LOCKEAR");
		lockearLectura(path);

		log_warning(logger_mongo, "PRE CONFIG");
		log_warning(logger_mongo, "CONFIG path = %s", path);
		t_config* config = config_create(path);
		log_warning(logger_mongo, "POST  CONFIG");

		log_warning(logger_mongo, "PRE CADENA BLOCKS");
		char* cadena_blocks = config_get_string_value(config, "BLOCKS");
		log_warning(logger_mongo, "POST CADENA BLOCKS");

		unlockear(path);
		log_warning(logger_mongo, "POST UNLOCKEAR");

		log_warning(logger_mongo, "PRE CONCATENAR");
		char* cadena_aux = concatenar_numeros(cadena_blocks);
		log_warning(logger_mongo, "POST CONCATENAR");

		log_warning(logger_mongo, "PRE MD5");
		char* md5 = md5_archivo(cadena_aux);
		log_warning(logger_mongo, "POST MD5");

		log_warning(logger_mongo, "PRE SET MD5");
		set_md5(path, md5);
		log_warning(logger_mongo, "POST SET");

		log_warning(logger_mongo, "PRE FREE MD5");
		free(md5);
		log_warning(logger_mongo, "PRE FREE AUX");
		free(cadena_aux);
		log_warning(logger_mongo, "POST FREE AUX");
		config_destroy(config);
	} else {
		// Esto no deberia pasar, ya no inicializamos archivos vacios, pero pendiente de revision
		log_error(logger_mongo, "MD5 INDEFINIDO");
		set_md5(path, "INDEFINIDO"); // Que no se setee si no tiene bloques
	}*/

	log_trace(logger_mongo, "pre matar_lista_ir2");
	matar_lista(lista_bloques);
	log_trace(logger_mongo, "post matar_lista_ir2");

	log_warning(logger_mongo, "POST RECURSO");
	log_trace(logger_mongo, "Fin  iniciar_archivo_recurso2");
}

void iniciar_archivo_recurso(char* path, int tamanio, int cant_bloques, t_list* lista_bloques){

	log_trace(logger_mongo, "Inicio iniciar_archivo_recurso");
	log_warning(logger_mongo, "RECURSO");
	set_tam(path, tamanio);
	set_cant_bloques(path, cant_bloques);
	set_bloq(path, lista_bloques);

	char caracter = caracter_llenado_archivo(path);
	log_warning(logger_mongo, "POST LLENADO");
	set_caracter_llenado(path, caracter);

	if(cant_bloques != 0){
		log_warning(logger_mongo, "PRE LOCKEAR");
		lockearLectura(path);
		log_warning(logger_mongo, "PRE CONFIG");
		log_warning(logger_mongo, "CONFIG path = %s", path);
		t_config* config = config_create(path);
		log_warning(logger_mongo, "POST  CONFIG");
		log_warning(logger_mongo, "PRE CADENA BLOCKS");
		char* cadena_blocks = config_get_string_value(config, "BLOCKS");
		log_warning(logger_mongo, "POST CADENA BLOCKS");
		unlockear(path);
		log_warning(logger_mongo, "POST UNLOCKEAR");
		log_warning(logger_mongo, "PRE CONCATENAR");
		char* cadena_aux = concatenar_numeros(cadena_blocks);
		log_warning(logger_mongo, "POST CONCATENAR");
		log_warning(logger_mongo, "PRE MD5");
		char* md5 = md5_archivo(cadena_aux);
		log_warning(logger_mongo, "POST MD5");
		log_warning(logger_mongo, "PRE SET MD5");
		set_md5(path, md5);
		log_warning(logger_mongo, "POST SET");
		log_warning(logger_mongo, "PRE FREE MD5");
		free(md5);
		log_warning(logger_mongo, "PRE FREE AUX");
		free(cadena_aux);
		log_warning(logger_mongo, "POST FREE AUX");
		config_destroy(config);
	} else {
		// Esto no deberia pasar, ya no inicializamos archivos vacios, pero pendiente de revision
		log_error(logger_mongo, "MD5 INDEFINIDO");
		set_md5(path, "INDEFINIDO"); // Que no se setee si no tiene bloques
	}

	log_warning(logger_mongo, "POST RECURSO");
	log_trace(logger_mongo, "Fin  iniciar_archivo_recurso");
}

char* concatenar_numeros(char* cadena) {

	int cantidad_numeros = 0;
	log_warning(logger_mongo, "PRE FOR");

	for(int i = 0; i < strlen(cadena); i++) {
		if (isdigit(cadena[i])) {
			cantidad_numeros++;
		}
	}

	log_warning(logger_mongo, "POST FOR Y PRE MALLOC");

	char* cadena_aux = malloc((sizeof(char) * cantidad_numeros) + 1);
	log_warning(logger_mongo, "POST MALLOC PRE CADENA");

	log_trace(logger_mongo, "Cadena es %s", cadena);

	int j = 0;
	for(int i = 0; i < strlen(cadena); i++) {
		if (isdigit(cadena[i])) {
			cadena_aux[j] = cadena[i];
			j++;
		}
	}
	cadena_aux[j] = '\0';

	log_trace(logger_mongo, "Cadena aux es %s", cadena_aux);

	return cadena_aux;
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
	log_trace(logger_mongo, "Cantidad de bloques inicial es %i, sera aumentada.", cantidad_bloques);
	t_list* lista_bloques = get_lista_bloques(path);

	// NO CAMBIEN ESTO, DEJENLO COMO ESTA
	int* aux = malloc(sizeof(int));
	*aux = *pos_libre;
	list_add(lista_bloques, aux);

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
	matar_lista(lista_bloques);
}

void asignar_bloque_tripulante(char* path, int* pos_libre, int size_agregado) {

	uint32_t tamanio = tamanio_archivo(path);
	t_list* lista_bloques = get_lista_bloques(path);

	log_debug(logger_mongo, "Tamanio de la lista antes de agregar: %i", list_size(lista_bloques));
	int* aux = malloc(sizeof(int));
	*aux = *pos_libre;
	list_add(lista_bloques, aux);
	log_debug(logger_mongo, "Tamanio de la lista luego de agregar: %i", list_size(lista_bloques));

	escribir_archivo_tripulante (path, tamanio, lista_bloques);
	matar_lista(lista_bloques);
}

uint32_t bloques_contar(char caracter) {

 	log_trace(logger_mongo, "Por contar cantidad de bloques.");
	int cantidad = 0;

	char* path = tipo_a_path(caracter);
	t_list* bloques = get_lista_bloques(path);
	int* aux;

	// lockearLectura(path_blocks);

	for(int i = 0 ; i < list_size(bloques); i++){
		aux = list_get(bloques, i);
		for(int j = 0 ; j < TAMANIO_BLOQUE; j++){
			if (*(directorio.mapa_blocks + *aux * TAMANIO_BLOQUE + j) == caracter){
				cantidad++;
			}
		}

	}

	log_trace(logger_mongo, "Se contaron bloques, son %i", cantidad);

	matar_lista(bloques);

	// unlockear(path_blocks);

	return cantidad;
}


char* crear_puntero_a_bitmap(){

	// EL RETORNO SE DEBE LIBERAR
	char* puntero_a_bitmap = malloc(CANTIDAD_BLOQUES / 8);

	lockearLectura(path_superbloque);
	fseek(directorio.superbloque, sizeof(uint32_t)*2, SEEK_SET);
	fread(puntero_a_bitmap, CANTIDAD_BLOQUES/8, 1, directorio.superbloque);
	unlockear(path_superbloque);

	return puntero_a_bitmap;
}

char* crear_puntero_a_bitmap_fd(){

	// EL RETORNO SE DEBE LIBERAR
	char* puntero_a_bitmap = malloc(CANTIDAD_BLOQUES / 8);

	lockearLectura(path_superbloque);
    memcpy(puntero_a_bitmap, directorio.supermapa + 8, CANTIDAD_BLOQUES/8);
	unlockear(path_superbloque);

	return puntero_a_bitmap;
}

void limpiar_metadata(char* path) {
	iniciar_archivo_recurso(path, 0, 0, NULL);
}


void liberar_bloques(char* path) {

	log_info(logger_mongo, "Se liberan los bloques asociados al path %s", path);

	t_list* bloques = get_lista_bloques(path);
	int* nro_bloque;

	for(int i = 0; i < list_size(bloques) ; i++) {
		nro_bloque = list_get(bloques, i);
		liberar_bloque(path, *nro_bloque);
		blanquear_bloque(*nro_bloque);
	}

	matar_lista(bloques);
}

void blanquear_bloque(int bloque){

	log_trace(logger_mongo, "Se llena de centinelas el bloque %i", bloque);

	// lockearEscritura(path_blocks);

	for(int i = 0; i < TAMANIO_BLOQUE; i++){
		*(directorio.mapa_blocks + bloque * TAMANIO_BLOQUE + i) = ',';
	}

	// unlockear(path_blocks);
}

void liberar_bloque(char* path, uint32_t nro_bloque) {
	t_list* bloques = get_lista_bloques(path);

	uint32_t* nro_bloque_aux;
	t_bitarray* nuevo_bitmap = obtener_bitmap();

	for(int i = 0; i < list_size(bloques) ; i++) {

		nro_bloque_aux = list_get(bloques, i);

		if (nro_bloque == *nro_bloque_aux) {
			bitarray_clean_bit(nuevo_bitmap, nro_bloque);

			reescribir_bitmap_fd(nuevo_bitmap);

			bool quitar_bloque(void* elemento1){
				return (nro_bloque == *((int*) elemento1));
			}

			int* aux = list_remove_by_condition(bloques, quitar_bloque);
			free(aux);

			if(es_recurso(path)){
				set_bloq(path, bloques);
				set_cant_bloques(path, list_size(bloques) - 1);
			}
		}
	}

	bool quitar_bloque(void* elemento1){
		return (nro_bloque == *((int*) elemento1));
	}

	int* aux = monitor_lista(sem_lista_bloques_ocupados, (void*) list_remove_by_condition, lista_bloques_ocupados, (void*) quitar_bloque);
	free(aux);

	matar_lista(bloques);
	free(nuevo_bitmap->bitarray);
	bitarray_destroy(nuevo_bitmap);
}

void agregar_tam(char* path, int tamanio) {
	lockearEscritura(path);

	t_config* config = config_create(path);
	config_save_in_file(config, path);
	int tamanio_viejo = config_get_int_value(config, "SIZE");
	tamanio_viejo += tamanio;
	char* aux = string_itoa(tamanio_viejo);
	config_set_value(config, "SIZE", aux);
	free(aux);
	config_save(config);
	config_destroy(config);

	unlockear(path);
}

void quitar_tam(char* path, int tamanio) {
	lockearEscritura(path);

	t_config* config = config_create(path);
	config_save_in_file(config, path);
	int tamanio_viejo = config_get_int_value(config, "SIZE");
	tamanio_viejo += tamanio;
	char* aux = string_itoa(tamanio_viejo);
	config_set_value(config, "SIZE", aux);
	free(aux);
	config_save(config);
	config_destroy(config);

	unlockear(path);
}

void set_tam(char* path, int tamanio){

	lockearEscritura(path);

	t_config* config = config_create(path);
	config_save_in_file(config, path);
	char* aux = string_itoa(tamanio);
	config_set_value(config, "SIZE", aux);
	free(aux);
	config_save(config);
	config_destroy(config);

	unlockear(path);
}

void set_bloq(char* path, t_list* lista){

	if(es_recurso(path)){
		lockearEscritura(path);
	}

	t_config* config = config_create(path);

	log_debug(logger_mongo, "la lista de bloques a escribir en archivo es %s", config_get_string_value(config, "BLOCKS"));

	t_list* list_aux;

	if(lista == NULL){
		list_aux = list_create();
	}

	else {
		t_list* nueva = lista;
		list_aux = list_duplicate(nueva);
	}

	int* aux;
	char* lista_bloques;

	if(list_aux == NULL || list_is_empty(list_aux)){
	    // log_trace(logger_mongo, "La lista esta vacia");

		lista_bloques = malloc(2 + 1);
		strcpy(lista_bloques, "[]");

	}
	else {
		// log_trace(logger_mongo, "La lista no es vacia.");

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

		/* log_trace(logger_mongo, "printeando lista");
		for(int i = 0; i < list_size(list_aux); i++){
			aux = list_get(list_aux, i);
			log_error(logger_mongo, "valor %i, %i", i, *aux);
		} */

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
	log_trace(logger_mongo, "La lista de bloques se escribio como %s", config_get_string_value(config, "BLOCKS"));

	config_save_in_file(config, path);
	config_destroy(config);

	liberar_lista(list_aux);

	free(lista_bloques);

	if(es_recurso(path)){
		unlockear(path);
	}
}


void set_cant_bloques(char* path, int cant){

	lockearEscritura(path);

	t_config* config = config_create(path);
	config_save_in_file(config, path);
	char* aux = string_itoa(cant);
	config_set_value(config, "BLOCK_COUNT", aux);
	free(aux);
	config_save(config);
	config_destroy(config);

	unlockear(path);
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

	t_config* config = config_create(path);
	config_save_in_file(config, path);
	config_set_value(config, "MD5_ARCHIVO", md5);
	config_save(config);
	config_destroy(config);

	unlockear(path);
}
