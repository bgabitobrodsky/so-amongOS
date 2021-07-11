#include "mongo_blocks.h"
#include "comms/generales.h"

#include <fcntl.h>

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

char* mapa;

void iniciar_superbloque(FILE* archivo) { // No se destruye bitarray
	// TODO: CAMBIAR uint8_t por UINT32_T, y usar por mmap

	log_trace(logger_mongo, "entro");
    uint8_t block_size = 64; // Bytes
    uint8_t size = 64;
    void* puntero_a_bits = malloc(size/8);
    t_bitarray* bitmap = bitarray_create_with_mode(puntero_a_bits, size/8, LSB_FIRST); // SE DIVIDE POR OCHO PORQUE EL SIZE ES EN BYTES, PONER 1 SIGNIFICA CREAR UN BITARRAY DE 8 BITS

    for(int i = 0; i < size; i++) {
 	   bitarray_clean_bit(bitmap, i);
    }

    log_trace(logger_mongo, "pre write");

    fwrite(&block_size, sizeof(uint8_t), 1, archivo);

    fwrite(&size, sizeof(uint8_t), 1, archivo);

    fflush(archivo);
    // lineas de debug, NO SIRVEN, usar xxd -b SuperBloque.ims
    char* arc = leer_archivo_entero(path_superbloque);
    log_info(logger_mongo, "tamanio: %i, %s", strlen(arc), arc);

    if(fwrite(bitmap->bitarray, bitmap->size, 1, archivo) > 0){
    	log_trace(logger_mongo, "bien");
    }else{
    	log_trace(logger_mongo, "mal");
    }

    log_trace(logger_mongo, "post-write");

    fflush(archivo);
}

void iniciar_blocks(int filedescriptor_blocks) {
    log_trace(logger_mongo, "0");

    uint8_t block_size = obtener_tamanio_bloque();
    log_trace(logger_mongo, "OBTENIENDO TAMANIO");

    uint8_t size = obtener_cantidad_bloques();

    log_trace(logger_mongo, "1");

    mapa = (void*) mmap(NULL, block_size * size, PROT_READ | PROT_WRITE, MAP_SHARED, filedescriptor_blocks, 0); // Revisar flags

    if(mapa == MAP_FAILED){
        log_trace(logger_mongo, "FAIL");
    }

    log_trace(logger_mongo, "2");

    directorio.mapa_blocks = malloc(block_size * size);

    log_trace(logger_mongo, "block size * size %i", block_size * size);
    log_trace(logger_mongo, "block size: %i", block_size);
    log_trace(logger_mongo, "size: %i", size);

    posix_fallocate(filedescriptor_blocks, 0, block_size * size);

    memcpy(directorio.mapa_blocks, mapa, block_size * size);

    log_trace(logger_mongo, "3");

}

void inicializar_mapa() {
	log_trace(logger_mongo, "iniciando mapita");
	log_trace(logger_mongo, "CANTIDAD_BLOQUES %i", CANTIDAD_BLOQUES);

	for (int i = 0; i < CANTIDAD_BLOQUES * TAMANIO_BLOQUE ; i++) {
        *(directorio.mapa_blocks + i) = '\t';
	}

	memcpy(mapa, directorio.superbloque, CANTIDAD_BLOQUES * TAMANIO_BLOQUE);
    msync(mapa, CANTIDAD_BLOQUES * TAMANIO_BLOQUE, MS_ASYNC);

	log_trace(logger_mongo, "terminando mapita");
}

uint8_t obtener_tamanio_bloque() {
	uint8_t block_size;
    fseek(directorio.superbloque, 0, SEEK_SET);
    fread(&block_size, sizeof(uint8_t), 1, directorio.superbloque);

    return block_size;
}

uint8_t obtener_cantidad_bloques() {
    obtener_tamanio_bloque();

    uint8_t size;
    fread(&size, sizeof(uint8_t), 1, directorio.superbloque);

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
