#include "mongo_archivos.h"

// Vars globales
t_log* logger_mongo;
t_config* config_mongo;
t_archivos archivos;
t_list* bitacoras;

void inicializar_archivos(char* path_files) { // TODO: Puede romper
	char* path_oxigeno = malloc((strlen(path_files)+1) + strlen("/Oxigeno.ims"));
	sprintf(path_oxigeno, "%s/Oxigeno.ims", path_files);

	char* path_comida = malloc((strlen(path_files)+1) + strlen("/Comida.ims"));
	sprintf(path_comida, "%s/Comida.ims", path_files);

	char* path_basura = malloc((strlen(path_files)+1) + strlen("/Basura.ims"));
	sprintf(path_basura, "%s/Basura.ims", path_files);

	char* path_superbloque = malloc((strlen(path_files)+1) + strlen("/SuperBloque.ims"));
	sprintf(path_superbloque, "%s/SuperBloque.ims", path_files); // TODO: Implementar cosas con el superbloque

	char* path_blocks = malloc((strlen(path_files)+1) + strlen("/Blocks.ims"));
	sprintf(path_blocks, "%s/Blocks.ims", path_files); // TODO: Implementar cosas con el block

	int filedescriptor_oxigeno     = open(path_oxigeno, O_RDWR | O_APPEND | O_CREAT); // TODO: Ver que son esas constantes
	int filedescriptor_comida      = open(path_comida, O_RDWR | O_APPEND | O_CREAT);   
	int filedescriptor_basura      = open(path_basura, O_RDWR | O_APPEND | O_CREAT);
    int filedescriptor_superbloque = open(path_superbloque, O_RDWR | O_APPEND | O_CREAT);
    int filedescriptor_blocks      = open(path_blocks, O_RDWR | O_APPEND | O_CREAT);

	FILE* file_oxigeno     = fdopen(filedescriptor_oxigeno, "r+");
	FILE* file_comida      = fdopen(filedescriptor_comida, "r+");
	FILE* file_basura      = fdopen(filedescriptor_basura, "r+");
    FILE* file_superbloque = fdopen(filedescriptor_superbloque, "r+");
    FILE* file_blocks      = fdopen(filedescriptor_blocks, "r+");

	archivos.oxigeno      = file_oxigeno;
	archivos.comida       = file_comida;
	archivos.basura       = file_basura;
    archivos.superbloque  = file_superbloque;
    archivos.blocks       = file_blocks;
	archivo.path_blocks   = path_blocks; // Actualizar struct

	iniciar_superbloque(file_superbloque);
	iniciar_blocks(file_blocks, filedescriptor_blocks); // Actualizar struct

	free(path_oxigeno);
	free(path_comida);
	free(path_basura);
	free(path_superbloque);
}

void inicializar_archivos_preexistentes(char* path_files) { // TODO: Puede romper, actualizar conforme arriba
	char* path_oxigeno = malloc((strlen(path_files)+1) + strlen("/Oxigeno.ims"));
	sprintf(path_oxigeno, "%s/Oxigeno.ims", path_files);

	char* path_comida = malloc((strlen(path_files)+1) + strlen("/Comida.ims"));
	sprintf(path_comida, "%s/Comida.ims", path_files);

	char* path_basura = malloc((strlen(path_files)+1) + strlen("/Basura.ims"));
	sprintf(path_basura, "%s/Basura.ims", path_files);

	char* path_superbloque = malloc((strlen(path_files)+1) + strlen("/SuperBloque.ims"));
	sprintf(path_superbloque, "%s/SuperBloque.ims", path_files); 

	char* path_blocks = malloc((strlen(path_files)+1) + strlen("/Blocks.ims"));
	sprintf(path_blocks, "%s/Blocks.ims", path_files);

	int filedescriptor_oxigeno     = open(path_oxigeno, O_RDWR | O_APPEND | O_CREAT); // TODO: Ver que son esas constantes
	int filedescriptor_comida      = open(path_comida, O_RDWR | O_APPEND | O_CREAT);   
	int filedescriptor_basura      = open(path_basura, O_RDWR | O_APPEND | O_CREAT);
    int filedescriptor_superbloque = open(path_superbloque, O_RDWR | O_APPEND | O_CREAT);
    int filedescriptor_blocks      = open(path_blocks, O_RDWR | O_APPEND | O_CREAT);

	FILE* file_oxigeno     = fdopen(filedescriptor_oxigeno, "r+");
	FILE* file_comida      = fdopen(filedescriptor_comida, "r+");
	FILE* file_basura      = fdopen(filedescriptor_basura, "r+");
    FILE* file_superbloque = fdopen(filedescriptor_superbloque, "r+");
    FILE* file_blocks      = fdopen(filedescriptor_blocks, "r+");

	archivos.oxigeno      = file_oxigeno;
	archivos.comida       = file_comida;
	archivos.basura       = file_basura;
    archivos.superbloque  = file_superbloque;
    archivos.blocks       = file_blocks;
	archivos.path_blocks  = path_blocks;
	
	iniciar_blocks(file_blocks, filedescriptor_blocks); // Revisar si mmap tiene efecto

	free(path_oxigeno);
	free(path_comida);
	free(path_basura);
	free(path_superbloque);
}

/* void alterar(int codigo_archivo, int cantidad) {
	switch(codigo_archivo) { 
		case OXIGENO:
			if (cantidad >= 0) 
				agregar(archivos.oxigeno, cantidad, 'O');
			else
				quitar(archivos.oxigeno, cantidad, 'O');
			break;
		case COMIDA: 
			if (cantidad >= 0) 
				agregar(archivos.comida, cantidad, 'C');
			else
				quitar(archivos.comida, cantidad, 'C');
			break;
		case BASURA: 
			if (cantidad >= 0) 
				agregar(archivos.basura, cantidad, 'B');
			else
				quitar(archivos.basura, cantidad, 'B');
			break;
	}
} */

void alterar(int codigo_archivo, int cantidad) {  // Alternativa mas prolija, revisar si funciona
	if (cantidad >= 0){
		agregar(cantidad, conseguir_char(codigo_archivo));
		log_info(logger_mongo, "Se agregaron %s unidades a %s.\n", string_itoa(cantidad), conseguir_tipo(conseguir_char(codigo_archivo)));
	}
	else{
		quitar(cantidad, conseguir_char(codigo_archivo));
		log_info(logger_mongo, "Se quitaron %s unidades a %s.\n", string_itoa(cantidad), conseguir_tipo(conseguir_char(codigo_archivo)));
	}
}

int asignar_primer_bloque_libre(uint32_t* lista_bloques[], uint32_t* cant_bloques, int cantidad_deseada; char tipo) { // ESPANTOSO, fijarse si funca, puede explotar por ser un void* (desplazamiento numerico tiene que ser bytes para que funque)
	void* mapa = archivos.mapa_blocks;
	int cantidad_alcanzada = 0;

	for(int j = 0; j < cant_bloques; j++) {
		for (int i = 0; tipo != *(mapa + lista_bloques[j] * TAMANIO_BLOQUE + i + 1); i++) { // Cambiar Macro por revision al Superbloque
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

	// TODO: Buscar bloques en Blocks
	FILE* archivo = conseguir_archivo_char(tipo);

	fseek(archivo, sizeof("SIZE="), SEEK_SET);
	uint32_t* tamanio_archivo;
	fread(tamanio_archivo, sizeof(uint32_t), 1, archivo);

	fseek(archivo, sizeof("BLOCK_COUNT="), SEEK_CUR);
	uint32_t* cant_bloques;
	fread(cant_bloques, sizeof(uint32_t), 1, archivo);

	fseek(archivo, sizeof("BLOCKS="), SEEK_CUR);
	uint32_t* lista_bloques[] = malloc(sizeof(uint32_t) * cant_bloques); // Esto deberia reventar mas fuerte que Hiroshima
	fread(lista_bloques, sizeof(uint32_t), *cant_bloques, archivo);

	int offset = asignar_primer_bloque_libre(lista_bloques, cant_bloques, cantidad, tipo);

	if (offset == -1) {
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
	char c;
	int contador = 0;

	pthread_mutex_lock(&mutex_blocks); 

	// TODO: Debera escribir NULL donde estaba el char en el mapa, en ultima posicion	

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
