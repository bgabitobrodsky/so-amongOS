#include "mongo_archivos.h"

// Vars globales
t_log* logger_mongo;
t_config* config_mongo;
t_archivos archivos;
t_list* bitacoras;

void inicializar_archivos(char* path_files) { // TODO: Puede romper
	// Se obtiene el path al archivo oxigeno dentro de la carpeta files
	char* path_oxigeno = malloc((strlen(path_files)+1) + strlen("/Oxigeno.ims"));
	sprintf(path_oxigeno, "%s/Oxigeno.ims", path_files);

	// Se obtiene el path al archivo comida dentro de la carpeta files
	char* path_comida = malloc((strlen(path_files)+1) + strlen("/Comida.ims"));
	sprintf(path_comida, "%s/Comida.ims", path_files);

	// Se obtiene el path al archivo basura dentro de la carpeta files
	char* path_basura = malloc((strlen(path_files)+1) + strlen("/Basura.ims"));
	sprintf(path_basura, "%s/Basura.ims", path_files);

	char* path_directorio = config_get_string_value(config_mongo, "PUNTO_MONTAJE");
	// Se obtiene el path al archivo superbloque dentro de la carpeta files (deberia ser dentro del punto de montaje nomas)
	char* path_superbloque = malloc((strlen(path_directorio)+1) + strlen("/SuperBloque.ims"));
	sprintf(path_superbloque, "%s/SuperBloque.ims", path_directorio);

	// Se obtiene el path al archivo blocks dentro de la carpeta files (deberia ser dentro del punto de montaje nomas)
	char* path_blocks = malloc((strlen(path_directorio)+1) + strlen("/Blocks.ims"));
	sprintf(path_blocks, "%s/Blocks.ims", path_directorio);

    int filedescriptor_blocks = open(path_blocks, O_RDWR | O_APPEND | O_CREAT);
    archivos.path_blocks       = path_blocks; // Actualizar struct

	// Trunco los archivos o los creo en modo escritura y lectura
	// Se guarda todo en un struct para uso en distintas funciones
	archivos.oxigeno     = fopen(path_oxigeno, "w+");
	archivos.comida      = fopen(path_comida, "w+");
	archivos.basura      = fopen(path_basura, "w+");
	archivos.superbloque = fopen(path_superbloque, "w+");
	archivos.blocks      = fdopen(filedescriptor_blocks, "w+");

	// TODO: Escribir metadata inicial para los archivos, darles un bloque

	iniciar_superbloque(archivos.superbloque);
	iniciar_blocks(filedescriptor_blocks); // Actualizar struct

	free(path_oxigeno);
	free(path_comida);
	free(path_basura);
	free(path_superbloque);
}

void inicializar_archivos_preexistentes(char* path_files) { // TODO: Puede romper, actualizar conforme arriba
	// Se obtiene el path al archivo oxigeno dentro de la carpeta files
	char* path_oxigeno = malloc((strlen(path_files)+1) + strlen("/Oxigeno.ims"));
	sprintf(path_oxigeno, "%s/Oxigeno.ims", path_files);

	// Se obtiene el path al archivo comida dentro de la carpeta files
	char* path_comida = malloc((strlen(path_files)+1) + strlen("/Comida.ims"));
	sprintf(path_comida, "%s/Comida.ims", path_files);

	// Se obtiene el path al archivo basura dentro de la carpeta files
	char* path_basura = malloc((strlen(path_files)+1) + strlen("/Basura.ims"));
	sprintf(path_basura, "%s/Basura.ims", path_files);

	// Se obtiene el path al archivo superbloque dentro de la carpeta files (deberia ser dentro del punto de montaje nomas)
	char* path_superbloque = malloc((strlen(path_files)+1) + strlen("/SuperBloque.ims"));
	sprintf(path_superbloque, "%s/SuperBloque.ims", path_files); 

	// Se obtiene el path al archivo blocks dentro de la carpeta files (deberia ser dentro del punto de montaje nomas)
	char* path_blocks = malloc((strlen(path_files)+1) + strlen("/Blocks.ims"));
	sprintf(path_blocks, "%s/Blocks.ims", path_files);

    int filedescriptor_blocks = open(path_blocks, O_RDWR | O_APPEND | O_CREAT);
    archivos.path_blocks       = path_blocks; // Actualizar struct

	// Abro los archivos en modo escritura y lectura (deben existir archivos)
	// Se guarda todo en un struct para uso en distintas funciones
	archivos.oxigeno     = fopen(path_oxigeno, "r+");
	archivos.comida      = fopen(path_comida, "r+");
	archivos.basura      = fopen(path_basura, "r+");
	archivos.superbloque = fopen(path_superbloque, "r+");
	archivos.blocks      = fdopen(filedescriptor_blocks, "r+");

	// TODO: Verificar si esta mappeado
	iniciar_blocks(filedescriptor_blocks); // Actualizar struct

	free(path_oxigeno);
	free(path_comida);
	free(path_basura);
	free(path_superbloque);
}

// Verificar bitmap de superbloque
// Elegir el primer libre
// Ocuparlo en bitmap
// Se lo asigna a archivo
// Llenar metadata con nuevos datos (distinguir entre tripu y recursos)

void asignar_nuevo_bloque(FILE* archivo) {
	void* mapa = archivos.superbloque;
	fseek(mapa, strlen("BLOCK_SIZE="), SEEK_SET);
	uint32_t tamanio_bloque;
	fread(&tamanio_bloque, sizeof(uint32_t), 1, mapa);

	fseek(mapa, strlen("BLOCK_COUNT="), SEEK_CUR);
	uint32_t cant_bloques;
	fread(&cant_bloques, sizeof(uint32_t), 1, mapa);

	t_bitarray* bitmap = malloc(sizeof(t_bitarray));
	bitmap = bitarray_create_with_mode(bitmap->bitarray, CANTIDAD_BLOQUES, LSB_FIRST);
	fread(bitmap->bitarray, sizeof(1/CHAR_BIT), CANTIDAD_BLOQUES, mapa); //CHAR_BIT: represents the number of bits in a char
	int bit_libre = -1;

	//Recorro todas las pociciones del bitarray
	for (int i = 0; i < CANTIDAD_BLOQUES; i++){
		//Entra si el bit del bitmap está en 0 y no se encontro bit_libre (< 0). Se puede mejorar
		if(!bitarray_test_bit(bitmap, i) && bit_libre  < 0)
			bit_libre = i;
	}

	//Si había un bloque libre
	if(bit_libre > 0) {
		//Marco el bit como ocupado
		bitarray_set_bit(bitmap, bit_libre);

		//Asigno el bloque a un archivo
		asignar_bloque(archivo, bit_libre); //TODO --> Falta que al bloque se le asigne el archivo correspondiente

		//Actualizo la metadata del archivo
		uint32_t tamanio = tamanio_archivo(archivo);
		uint32_t cantidad_bloques = cantidad_bloques_archvio(archivo) + 1;
		uint32_t* lista_bloques = lista_bloques_archvio(archivo);
		lista_bloques[cantidad_bloques] = bit_libre; //Agrega el nuevo bloque al final de la lista de bloques preexistente
		//t_list lista_bloques = lista_bloques_archvio(archivo);
		//list_add(lista_bloques, num_bloque);
		char caracter_llenado = caracter_llenado_archivo(archivo);
		char* md5 = md5_archivo(archivo);

		escribir_archivo(archivo, tamanio, cantidad_bloques, lista_bloques, caracter_llenado, md5);
	}
	//Si no había un bloque libre
	else
		log_info(logger_mongo, "No hay bloques disponibles en este momento"); //TODO ¿Qué hago?
}

/*int asignar_primer_bloque_libre(uint32_t* lista_bloques, uint32_t cant_bloques, int cantidad_deseada, char tipo) { // ESPANTOSO, fijarse si funca, puede explotar por ser un void* (desplazamiento numerico tiene que ser bytes para que funque)
	void* mapa = archivos.mapa_blocks;
	int cantidad_alcanzada = 0;

	for(int j = 0; j < cant_bloques; j++) {
		for (int i = 0; tipo != *(mapa + lista_bloques[j] * TAMANIO_BLOQUE + i + 1) && *(mapa + lista_bloques[j] * TAMANIO_BLOQUE + i + 1) != NULL; i++) { // Cambiar Macro por revision al Superbloque
			
			if (*(mapa + lista_bloques[j] * TAMANIO_BLOQUE + i) == NULL) { 
				*(mapa + lista_bloques[j] * TAMANIO_BLOQUE + i) = tipo;
				cantidad_alcanzada++;
			}

			if (cantidad_alcanzada == cantidad_deseada) {
				return j * 100 + i;
			}
		}
	}
	
	return cantidad_alcanzada - cantidad_deseada;
}*/

/*int quitar_ultimo_bloque_libre(uint32_t* lista_bloques, uint32_t cant_bloques, int cantidad_deseada, char tipo) {
	void* mapa = archivos.mapa_blocks;
	int cantidad_alcanzada = 0;

	for(int j = cant_bloques; j < 0; j--) {
		for (int i = TAMANIO_BLOQUE; tipo != *(mapa + (lista_bloques[j] + 1) * TAMANIO_BLOQUE - i - 1) && *(mapa + (lista_bloques[j] + 1) * TAMANIO_BLOQUE - i - 1) != NULL; i--) { // Cambiar Macro por revision al Superbloque
			
			if (*(mapa + (lista_bloques[j] + 1) * TAMANIO_BLOQUE - i) == tipo) { 
				*(mapa + (lista_bloques[j] + 1) * TAMANIO_BLOQUE - i) = NULL;
				cantidad_alcanzada++;
			}

			if (cantidad_alcanzada == cantidad_deseada) {
				return j * 100 + i;
			}
		}
	}
	
	return cantidad_alcanzada - cantidad_deseada;
}*/

/*void actualizar_MD5(FILE* archivo) {
	fseek(archivo, strlen("SIZE="), SEEK_SET);
	uint32_t tamanio_archivo;
	fread(&tamanio_archivo, sizeof(uint32_t), 1, archivo);

	fseek(archivo, strlen("BLOCK_COUNT="), SEEK_CUR); 
	uint32_t cant_bloques;
	fread(&cant_bloques, sizeof(uint32_t), 1, archivo);

	fseek(archivo, strlen("BLOCKS="), SEEK_CUR);
	uint32_t* lista_bloques = malloc(sizeof(uint32_t) * cant_bloques); // Esto deberia reventar mas fuerte que Hiroshima
	fread(lista_bloques, sizeof(uint32_t), &cant_bloques, archivo);

	fseek(archivo, strlen("CARACTER_LLENADO="), SEEK_CUR);
	char tipo;
	fread(&tipo, sizeof(char), 1, archivo);

	freopen(archivo, "w+");

	char* size = "SIZE=";
	char* block_count = "BLOCK_COUNT=";
	char* blocks = "BLOCKS=";
	char* caracter = "CARACTER_LLENADO=";
	char* md5 = "MD5_ARCHIVO=";
	char* md5_dato = crear_md5();

	fwrite(&size, strlen(size), 1, archivo);
	fwrite(&tamanio_archivo, sizeof(uint32_t), 1, archivo);
	fwrite(&block_count, strlen(block_count), 1, archivo);
	fwrite(&cant_bloques, sizeof(uint32_t), 1, archivo);
	fwrite(&blocks, strlen(blocks), 1, archivo);
	fwrite(lista_bloques, sizeof(uint32_t), &cant_bloques, archivo);
	fwrite(&caracter, strlen(caracter), 1, archivo);
	fwrite(&tipo, sizeof(char), 1, archivo);
	fwrite(&md5, strlen(md5), 1, archivo);
	fwrite(&md5_dato, strlen(md5_dato), 1, archivo);

	free(size);
	free(block_count);
	free(blocks);
	free(caracter);
	free(md5);
	free(md5_dato);
}*/

void alterar(int codigo_archivo, int cantidad) {  
	if (cantidad >= 0){
		agregar(conseguir_archivo(codigo_archivo), cantidad);
		log_info(logger_mongo, "Se agregaron %s unidades a %s.\n", string_itoa(cantidad), conseguir_tipo(conseguir_char(codigo_archivo)));
	}
	else{
		quitar(conseguir_archivo(codigo_archivo), conseguir_path(codigo_archivo), cantidad, conseguir_char(codigo_archivo));
		log_info(logger_mongo, "Se quitaron %s unidades a %s.\n", string_itoa(cantidad), conseguir_tipo(conseguir_char(codigo_archivo)));
	}
}

void agregar(FILE* archivo, int cantidad) { // Puede que haya que hacer mallocs previos
	pthread_mutex_lock(&mutex_blocks); // Declarar mutex

	fseek(archivo, strlen("SIZE="), SEEK_SET);
	uint32_t tamanio_archivo;
	fread(&tamanio_archivo, sizeof(uint32_t), 1, archivo);

	fseek(archivo, strlen("BLOCK_COUNT="), SEEK_CUR); 
	uint32_t cant_bloques;
	fread(&cant_bloques, sizeof(uint32_t), 1, archivo);

	fseek(archivo, strlen("BLOCKS="), SEEK_CUR);
	uint32_t* lista_bloques = malloc(sizeof(uint32_t) * cant_bloques); // Esto deberia reventar mas fuerte que Hiroshima
	fread(lista_bloques, sizeof(uint32_t), cant_bloques, archivo);

	fseek(archivo, strlen("CARACTER_LLENADO="), SEEK_CUR);
	char tipo;
	fread(&tipo, sizeof(char), 1, archivo);

	int offset = asignar_primer_bloque_libre(lista_bloques, cant_bloques, cantidad, tipo);

	if (offset < 0) { // Falto agregar cantidad, dada por offset
		asignar_nuevo_bloque(archivo);
		agregar(archivo, offset * -1); // Recursividad con la cantidad que falto
	}
	else if (offset < 100) { // No paso bloques. ¿No sería CANTIDAD_BLOQUES?
		msync(archivos.mapa_blocks, offset + 1, MS_ASYNC); //Falta el flag, puse MS_ASYNC, ni idea cual va. Link: https://man7.org/linux/man-pages/man2/msync.2.html
		sleep(config_get_int_value(config_mongo, "TIEMPO_SINCRONIZACION"));
	}
	else if (offset > 100) { // Se paso bloques
		int cant_bloques_local = offset / 100;
		offset = offset % 100;
		
					        //Acá iría cant_bloques_local?
		msync(archivos.mapa_blocks, cant_bloques * TAMANIO_BLOQUE + offset + 1, MS_ASYNC); // Cambiar macro por lo de Superbloque. Falta el flag, puse MS_ASYNC, ni idea cual va. Link: https://man7.org/linux/man-pages/man2/msync.2.html
		sleep(config_get_int_value(config_mongo, "TIEMPO_SINCRONIZACION"));
	}

	actualizar_MD5(archivo);

    pthread_mutex_unlock(&mutex_blocks);
}

void quitar(FILE* archivo, char* path, int cantidad, char tipo) { // Puede explotar en manejo de fopens, revisar
	pthread_mutex_lock(&mutex_blocks);

	fseek(archivo, strlen("SIZE="), SEEK_SET);
	uint32_t tamanio_archivo;
	fread(&tamanio_archivo, sizeof(uint32_t), 1, archivo);

	fseek(archivo, strlen("BLOCK_COUNT="), SEEK_CUR); 
	uint32_t cant_bloques;
	fread(&cant_bloques, sizeof(uint32_t), 1, archivo);

	fseek(archivo, strlen("BLOCKS="), SEEK_CUR);
	uint32_t* lista_bloques = malloc(sizeof(uint32_t) * cant_bloques); // Esto deberia reventar mas fuerte que Hiroshima
	fread(lista_bloques, sizeof(uint32_t), cant_bloques, archivo);

	fseek(archivo, strlen("CARACTER_LLENADO="), SEEK_CUR);
	fread(&tipo, sizeof(char), 1, archivo);

	int offset = quitar_ultimo_bloque_libre(lista_bloques, cant_bloques, cantidad * -1, tipo);

	if (offset < 0) { // Se quiso quitar mas de lo existente, no hace nada (queda para comprension)
	}
	else if (offset < 100) { // No paso bloques
		msync(archivos.mapa_blocks, lista_bloques[cant_bloques - 1] * TAMANIO_BLOQUE + 1, MS_ASYNC); //Falta el flag, puse MS_ASYNC, ni idea cual va. Link: https://man7.org/linux/man-pages/man2/msync.2.html
		sleep(config_get_int_value(config_mongo, "TIEMPO_SINCRONIZACION"));
	}
	else if (offset > 100) { // Se paso bloques
		int cant_bloques_local = offset / 100;
		offset = offset % 100;
		
		msync(archivos.mapa_blocks, lista_bloques[(cant_bloques_local - 1)] * TAMANIO_BLOQUE + offset + 1, MS_ASYNC); // Cambiar macro por lo de Superbloque. Falta el flag, puse MS_ASYNC, ni idea cual va. Link: https://man7.org/linux/man-pages/man2/msync.2.html
		sleep(config_get_int_value(config_mongo, "TIEMPO_SINCRONIZACION"));
	}

	actualizar_MD5(archivo);

    pthread_mutex_unlock(&mutex_blocks);
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

char* conseguir_path(int codigo_archivo) {
	char* punto_monaje = PUNTO_MONTAJE;
	char* path_file = malloc(strlen(punto_monaje) + 1 + strlen("/Files/123456.ims"));
	switch(codigo_archivo) {
	case BASURA:
		sprintf(path_file, "%s/Files/Basura.ims", punto_monaje);
		return path_file;
		break;
	case COMIDA:
		sprintf(path_file, "%s/Files/Comida.ims", punto_monaje);
		return path_file;
		break;
	case OXIGENO:
		path_file = realloc(path_file, (strlen(punto_monaje) + 1 + strlen("/Files/123456.ims")));
		sprintf(path_file, "%s/Files/Oxigeno.ims", punto_monaje);
		return path_file;
		break;
	}
	return "No encontrado";
}

FILE* conseguir_archivo_char(char tipo) {
	if (tipo == 'O')
        return archivos.oxigeno;
    if (tipo == 'C')
        return archivos.comida;
    if (tipo == 'B')
        return archivos.basura;
    return NULL;
}

FILE* conseguir_archivo(int codigo) {
	switch(codigo) {
		case OXIGENO:
			return archivos.oxigeno;
			break;
		case COMIDA:
			return archivos.comida;
			break;
		case BASURA:
			return archivos.basura;
			break;
	}
	return NULL;
}

char* conseguir_nombre(FILE* archivo) {
	int codigo_archivo;
	char caracter = caracter_llenado_archivo(archivo);

	switch(caracter) {
	case 'O':
		codigo_archivo = OXIGENO;
		break;

	case 'B':
		codigo_archivo = BASURA;
		break;
	case 'C':
		codigo_archivo = COMIDA;
		break;
	}

	return conseguir_path(codigo_archivo);
}

int max (int a, int b) {
	if (a >= b) {
		return a;
	}
	else {
		return b;
	}
}

char* crear_md5() { // String de 32
	char* md5 = malloc(sizeof(char) * 32);

	for (int i = 0; i < 32; i++){
		md5[i] = char_random();
	}

	return md5;
}

char char_random() {
	int seleccion = rand() % 2;

	switch (seleccion) {
		case 0:
			return (char) (rand() % 9 + 48); // Devuelve un numero por ASCII
			break;
		case 1:
			return (char) (rand() % 26 + 65); // Devuelve un alfa por ASCII
			break;
	}
	return '\0';
}

uint32_t tamanio_archivo(FILE* archivo) {
	fseek(archivo, strlen("SIZE="), SEEK_SET);
	uint32_t tamanio_archivo;
	fread(&tamanio_archivo, sizeof(uint32_t), 1, archivo);

	return tamanio_archivo;
}

uint32_t cantidad_bloques_archvio(FILE* archivo) {
	fseek(archivo, strlen("SIZE="), SEEK_SET);
	uint32_t tamanio_archivo;
	fread(&tamanio_archivo, sizeof(uint32_t), 1, archivo);

	fseek(archivo, strlen("BLOCK_COUNT="), SEEK_CUR);
	uint32_t cant_bloques;
	fread(&cant_bloques, sizeof(uint32_t), 1, archivo);

	return cant_bloques;
}

uint32_t* lista_bloques_archvio(FILE* archivo) {
	fseek(archivo, strlen("SIZE="), SEEK_SET);
	uint32_t tamanio_archivo;
	fread(&tamanio_archivo, sizeof(uint32_t), 1, archivo);

	fseek(archivo, strlen("BLOCK_COUNT="), SEEK_CUR);
	uint32_t cant_bloques;
	fread(&cant_bloques, sizeof(uint32_t), 1, archivo);

	fseek(archivo, strlen("BLOCKS="), SEEK_CUR);
	uint32_t* lista_bloques = malloc(sizeof(uint32_t) * cant_bloques); // Esto deberia reventar mas fuerte que Hiroshima
	fread(lista_bloques, sizeof(uint32_t), cant_bloques, archivo);

	return lista_bloques;
}

/*t_list* lista_bloques_archvio(FILE* archivo) { //TODO ver si tratamos a la listas de bloques como uint32_t* o t_list
	fseek(archivo, strlen("SIZE="), SEEK_SET);
	uint32_t tamanio_archivo;
	fread(&tamanio_archivo, sizeof(uint32_t), 1, archivo);

	fseek(archivo, strlen("BLOCK_COUNT="), SEEK_CUR);
	uint32_t cant_bloques;
	fread(&cant_bloques, sizeof(uint32_t), 1, archivo);

	fseek(archivo, strlen("BLOCKS="), SEEK_CUR);
	uint32_t* lista_bloques = malloc(sizeof(uint32_t) * cant_bloques); // Esto deberia reventar mas fuerte que Hiroshima
	fread(lista_bloques, sizeof(uint32_t), cant_bloques, archivo);

	t_list* t_lista_bloques = list_create();
	for(int i = 0; i < cant_bloques; i++) {
		list_add(t_lista_bloques, lista_bloques[i]);
	}

	return t_lista_bloques;
}*/

char caracter_llenado_archivo(FILE* archivo) {
	fseek(archivo, strlen("SIZE="), SEEK_SET);
	uint32_t tamanio_archivo;
	fread(&tamanio_archivo, sizeof(uint32_t), 1, archivo);

	fseek(archivo, strlen("BLOCK_COUNT="), SEEK_CUR);
	uint32_t cant_bloques;
	fread(&cant_bloques, sizeof(uint32_t), 1, archivo);

	fseek(archivo, strlen("BLOCKS="), SEEK_CUR);
	uint32_t* lista_bloques = malloc(sizeof(uint32_t) * cant_bloques); // Esto deberia reventar mas fuerte que Hiroshima
	fread(lista_bloques, sizeof(uint32_t), cant_bloques, archivo);

	char caracter_llenado;
	fseek(archivo, strlen("CARACTER_LLENADO="), SEEK_CUR);
	fread(&caracter_llenado, sizeof(char), 1, archivo);

	return caracter_llenado;
}

char* md5_archivo(FILE* archivo) {
	fseek(archivo, strlen("SIZE="), SEEK_SET);
	uint32_t tamanio_archivo;
	fread(&tamanio_archivo, sizeof(uint32_t), 1, archivo);

	fseek(archivo, strlen("BLOCK_COUNT="), SEEK_CUR);
	uint32_t cant_bloques;
	fread(&cant_bloques, sizeof(uint32_t), 1, archivo);

	fseek(archivo, strlen("BLOCKS="), SEEK_CUR);
	uint32_t* lista_bloques = malloc(sizeof(uint32_t) * cant_bloques); // Esto deberia reventar mas fuerte que Hiroshima
	fread(lista_bloques, sizeof(uint32_t), cant_bloques, archivo);

	fseek(archivo, strlen("CARACTER_LLENADO="), SEEK_CUR);
	char tipo;
	fread(&tipo, sizeof(char), 1, archivo);

	fseek(archivo, strlen("MD5_ARCHIVO="), SEEK_CUR);
	char* md5;
	//fread(&md5, sizeof(char), N, archivo) //TODO --> De Javi: No entiendo muy bien que hay en el MD5

	return md5;
}

void escribir_archivo(FILE* archivo, uint32_t tamanio, uint32_t cantidad_bloques, uint32_t* list_bloques, char caracter_llenado, char* md_5) {
	fseek(archivo, strlen("SIZE="), SEEK_SET);
	uint32_t tamanio_archivo;
	fread(&tamanio_archivo, sizeof(uint32_t), 1, archivo);

	fseek(archivo, strlen("BLOCK_COUNT="), SEEK_CUR);
	uint32_t cant_bloques;
	fread(&cant_bloques, sizeof(uint32_t), 1, archivo);

	fseek(archivo, strlen("BLOCKS="), SEEK_CUR);
	uint32_t* lista_bloques = malloc(sizeof(uint32_t) * cant_bloques); // Esto deberia reventar mas fuerte que Hiroshima
	fread(lista_bloques, sizeof(uint32_t), cant_bloques, archivo);

	fseek(archivo, strlen("CARACTER_LLENADO="), SEEK_CUR);
	char tipo;
	fread(&tipo, sizeof(char), 1, archivo);

	char* nombre_archivo = conseguir_nombre(archivo);
	freopen(nombre_archivo, "w+", archivo);

	char* size = "SIZE=";
	char* block_count = "BLOCK_COUNT=";
	char* blocks = "BLOCKS=";
	char* caracter = "CARACTER_LLENADO=";
	char* md5 = "MD5_ARCHIVO=";

	fwrite(&size, strlen(size), 1, archivo);
	fwrite(&tamanio, sizeof(uint32_t), 1, archivo);
	fwrite(&block_count, strlen(block_count), 1, archivo);
	fwrite(&cantidad_bloques, sizeof(uint32_t), 1, archivo);
	fwrite(&blocks, strlen(blocks), 1, archivo);
	fwrite(list_bloques, sizeof(uint32_t), cant_bloques, archivo);
	fwrite(&caracter, strlen(caracter), 1, archivo);
	fwrite(&caracter_llenado, sizeof(char), 1, archivo);
	fwrite(&md5, strlen(md5), 1, archivo);
	fwrite(&md_5, strlen(md_5), 1, archivo);

	free(size);
	free(block_count);
	free(blocks);
	free(caracter);
	free(md5);
}
