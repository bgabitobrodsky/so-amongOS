#include "mongo_blocks.h"
#include "comms/generales.h"

t_log* logger_mongo;
t_config* config_mongo;
t_directorio directorio;
t_recurso recurso;
t_list* bitacoras;

char* path_directorio;
char* path_files;
char* path_bitacoras;
char* path_oxigeno;
char* path_comida;
char* path_basura;
char* path_superbloque;
char* path_blocks;


void iniciar_superbloque(FILE* archivo) { // No se destruye bitarray
	log_trace(logger_mongo, "entro");
    uint32_t block_size = 64; // Bytes
    uint32_t size = 64;
    void* puntero_a_bits = malloc(size); //De javi: con calloc te inicializa el puntero con todos 0
    t_bitarray* bitmap = bitarray_create_with_mode(puntero_a_bits, size/8, LSB_FIRST); // SE DIVIDE POR OCHO PORQUE EL SIZE ES EN BYTES, PONER 1 SIGNIFICA CREAR UN BITARRAY DE 8 BITS

    for(int i = 0; i < size; i++) {
 	   bitarray_clean_bit(bitmap, i);
    }

    log_trace(logger_mongo, "pre write");

    uint32_t size_aux = 63;

    fwrite(&block_size, sizeof(uint32_t), 1, archivo);

    fwrite(&size, sizeof(uint32_t), 1, archivo);

    fflush(archivo);

    char* arc = leer_archivo_entero(path_superbloque);

    log_info(logger_mongo, "tamanio: %i, %s", strlen(arc), arc);

    fwrite(bitmap, sizeof(bitmap), 1, archivo);

    log_trace(logger_mongo, "post-write");

    fflush(archivo);
}

void iniciar_blocks(int filedescriptor_blocks) {
    log_trace(logger_mongo, "0");

    uint32_t block_size = (uint32_t) obtener_tamanio_bloque();

    uint32_t size = (uint32_t) obtener_cantidad_bloques();

    log_trace(logger_mongo, "1");

    char* mapa = (char*) mmap(NULL, block_size * size, PROT_READ | PROT_WRITE, MAP_SHARED, filedescriptor_blocks, 0); // Revisar flags

    if(mapa == MAP_FAILED){
        log_trace(logger_mongo, "FAIL");
    }

    log_trace(logger_mongo, "2");

    directorio.mapa_blocks = malloc(block_size * size);

    log_trace(logger_mongo, "block size * size %i", (int) block_size * size);
    log_trace(logger_mongo, "block size: %i", (int) block_size);
    log_trace(logger_mongo, "size: %i", (int) size);

    memcpy(directorio.mapa_blocks, mapa, block_size * size);

    log_trace(logger_mongo, "3");

    inicializar_mapa();
}

void inicializar_bloque(int numero_bloque) { // Inicializa bloques de recursos con whitespace, para funciones de agregado y quitado

    for (int i; i < (TAMANIO_BLOQUE); i++) {
        *(directorio.mapa_blocks +  TAMANIO_BLOQUE * numero_bloque + i) = '\t';
    }

    msync(directorio.mapa_blocks, (numero_bloque + 1) * TAMANIO_BLOQUE, MS_ASYNC);

}

void inicializar_mapa() {
	for (int i; i < CANTIDAD_BLOQUES; i++) {
        inicializar_bloque(i);
    }
}

uint32_t obtener_tamanio_bloque() {
    uint32_t block_size;
    fseek(directorio.superbloque, 0, SEEK_SET);
    fread(&block_size, sizeof(uint32_t), 1, directorio.superbloque);

    return block_size;
}

uint32_t obtener_cantidad_bloques() {
    obtener_tamanio_bloque();

    uint32_t size;
    fread(&size, sizeof(uint32_t), 1, directorio.superbloque);

    return size;
}

t_bitarray* obtener_bitmap() {
	int cant_bloques = obtener_cantidad_bloques();
	char* puntero_a_bitmap = calloc(cant_bloques / 8, 1);

	t_bitarray* bitmap = bitarray_create_with_mode(puntero_a_bitmap, cant_bloques, LSB_FIRST);
	fread(puntero_a_bitmap, 1, cant_bloques/8, directorio.superbloque);

	free(puntero_a_bitmap);

	return bitmap;
}

void reescribir_superbloque(int tamanio, int cantidad, t_bitarray* bitmap) {
    fclose(directorio.superbloque);
    directorio.superbloque = fopen(path_superbloque, "w+");

    fwrite(&tamanio, sizeof(uint32_t), 1, directorio.superbloque);
    fwrite(&cantidad, sizeof(uint32_t), 1, directorio.superbloque);
    fwrite(bitmap, sizeof(bitmap), 1, directorio.superbloque);

    fflush(directorio.superbloque);
}

t_bitarray* actualizar_bitmap(int* lista_bloques_ocupados) {
    void* puntero_a_bits = calloc(CANTIDAD_BLOQUES / 8, 1);
    t_bitarray* bitmap = bitarray_create_with_mode(puntero_a_bits, CANTIDAD_BLOQUES / 8, LSB_FIRST);

    for(int i = 0; i < CANTIDAD_BLOQUES; i++) {
    	if(contiene_generico(lista_bloques_ocupados, i))
    		bitarray_set_bit(bitmap, i);
    }
	return bitmap;
}

int contiene_generico(int* lista, int valor) {
	 int i;
	    for(i = 0; i < sizeof(lista) / sizeof(lista[0]); i++) {
	        if(lista[i] == valor)
	            return 1;
	    }
	    return 0;
}
