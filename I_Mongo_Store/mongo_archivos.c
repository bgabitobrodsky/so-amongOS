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

	// Se obtiene el path al archivo superbloque dentro de la carpeta files (deberia ser dentro del punto de montaje nomas)
	char* path_superbloque = malloc((strlen(path_files)+1) + strlen("/SuperBloque.ims"));
	sprintf(path_superbloque, "%s/SuperBloque.ims", path_files); // TODO: Implementar cosas con el superbloque

	// Se obtiene el path al archivo blocks dentro de la carpeta files (deberia ser dentro del punto de montaje nomas)
	char* path_blocks = malloc((strlen(path_files)+1) + strlen("/Blocks.ims"));
	sprintf(path_blocks, "%s/Blocks.ims", path_files);

    int filedescriptor_blocks = open(path_blocks, O_RDWR | O_APPEND | O_CREAT);
	archivo.path_blocks       = path_blocks; // Actualizar struct

	// Abro los archivos o los creo en modo escritura y lectura (se borran contenidos previos, salvarlos)
	// Se guarda todo en un struct para uso en distintas funciones
	archivos.oxigeno     = fopen(path_oxigeno, "w+");
	archivos.comida      = fopen(path_comida, "w+");
	archivos.basura      = fopen(path_basura, "w+");
	archivos.superbloque = fopen(path_superbloque, "w+");
	archivos.blocks      = fdopen(filedescriptor_blocks, "w+");

	iniciar_superbloque(archivos.superbloque);
	iniciar_blocks(archivos.blocks , filedescriptor_blocks); // Actualizar struct

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
	sprintf(path_superbloque, "%s/SuperBloque.ims", path_files); // TODO: Implementar cosas con el superbloque

	// Se obtiene el path al archivo blocks dentro de la carpeta files (deberia ser dentro del punto de montaje nomas)
	char* path_blocks = malloc((strlen(path_files)+1) + strlen("/Blocks.ims"));
	sprintf(path_blocks, "%s/Blocks.ims", path_files);

    int filedescriptor_blocks = open(path_blocks, O_RDWR | O_APPEND | O_CREAT);
	archivo.path_blocks       = path_blocks; // Actualizar struct

	// Abro los archivos o los creo en modo escritura y lectura (deben existir archivos)
	// Se guarda todo en un struct para uso en distintas funciones
	archivos.oxigeno     = fopen(path_oxigeno, "r+");
	archivos.comida      = fopen(path_comida, "r+");
	archivos.basura      = fopen(path_basura, "r+");
	archivos.superbloque = fopen(path_superbloque, "r+");
	archivos.blocks      = fdopen(filedescriptor_blocks, "r+");

	iniciar_blocks(archivos.blocks , filedescriptor_blocks); // Actualizar struct

	free(path_oxigeno);
	free(path_comida);
	free(path_basura);
	free(path_superbloque);
}

void alterar(int codigo_archivo, int cantidad) {  
	if (cantidad >= 0){
		agregar(cantidad, conseguir_char(codigo_archivo));
		log_info(logger_mongo, "Se agregaron %s unidades a %s.\n", string_itoa(cantidad), conseguir_tipo(conseguir_char(codigo_archivo)));
	}
	else{
		quitar(cantidad, conseguir_char(codigo_archivo));
		log_info(logger_mongo, "Se quitaron %s unidades a %s.\n", string_itoa(cantidad), conseguir_tipo(conseguir_char(codigo_archivo)));
	}
}

int asignar_primer_bloque_libre(uint32_t* lista_bloques, uint32_t cant_bloques, int cantidad_deseada; char tipo) { // ESPANTOSO, fijarse si funca, puede explotar por ser un void* (desplazamiento numerico tiene que ser bytes para que funque)
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
}

void agregar(FILE* archivo, int cantidad, char tipo) { // Puede que haya que hacer mallocs previos
	pthread_mutex_lock(&mutex_blocks); // Declarar mutex

	FILE* archivo = conseguir_archivo_char(tipo);

	fseek(archivo, sizeof("SIZE="), SEEK_SET);
	uint32_t tamanio_archivo;
	fread(&tamanio_archivo, sizeof(uint32_t), 1, archivo);

	fseek(archivo, sizeof("BLOCK_COUNT="), SEEK_CUR); 
	uint32_t cant_bloques;
	fread(&cant_bloques, sizeof(uint32_t), 1, archivo);

	fseek(archivo, sizeof("BLOCKS="), SEEK_CUR);
	uint32_t* lista_bloques = malloc(sizeof(uint32_t) * cant_bloques); // Esto deberia reventar mas fuerte que Hiroshima
	fread(lista_bloques, sizeof(uint32_t), &cant_bloques, archivo);

	// TODO: En vez de recibir char por parametro, buscarlo en metadata

	int offset = asignar_primer_bloque_libre(lista_bloques, cant_bloques, cantidad, tipo);

	if (offset < 0) { // Falto agregar cantidad, dada por offset
		// TODO: Asignar bloque nuevo y asignar ahi
	}
	else if (offset < 100) { // No paso bloques
		msync(archivos.mapa_blocks, offset + 1);
		sleep(config_get_int_value(config_mongo, TIEMPO_SINCRONIZACION));
	}
	else if (offset > 100) { // Se paso bloques
		int cant_bloques = offset / 100;
		offset = offset % 100;
		
		msync(archivos.mapa_blocks, cant_bloques * TAMANIO_BLOQUE + offset + 1); // Cambiar macro por lo de Superbloque
		sleep(config_get_int_value(config_mongo, TIEMPO_SINCRONIZACION));
	}

	// TODO: Actualizar MD5 correspondiente, podria estar aparte y tener mutex propio

    pthread_mutex_unlock(&mutex_blocks);
}

void quitar(FILE* archivo, char* path, int cantidad, char tipo) { // Puede explotar en manejo de fopens, revisar
	pthread_mutex_lock(&mutex_blocks); 

	// TODO: Debera escribir NULL donde estaba el char en el mapa, en ultima posicion posible	

    pthread_mutex_unlock(&mutex_blocks);
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

char conseguir_char(int codigo) {
	switch(codigo) {
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

int max (int a, int b) {
	if (a >= b) {
		return a;
	}
	else {
		return b;
	}
}
