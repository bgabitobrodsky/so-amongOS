#include "mongo_blocks.h"

void iniciar_superbloque(FILE* archivo) { // No se destruye bitarray
    uint32_t block_size = 64; // Bytes
    uint32_t size = 64;
    void* puntero_a_bits = malloc(TAMANIO_BLOQUE * 8); //De javi: con calloc te inicializa el puntero con todos 0
    t_bitarray* bitmap = bitarray_create_with_mode(puntero_a_bits, TAMANIO_BLOQUE * 8, LSB_FIRST); //De javi: ¿No sería CANTIDAD_BLOQUES?

    for(int i; i < CANTIDAD_BLOQUES; i++) {
        bitarray_clean_bit(bitmap, i);
    }

    fwrite("BITE_SIZE=", strlen("BITE_SIZE="), 1, archivo);
    fwrite(&block_size, sizeof(uint32_t), 1, archivo);

    fwrite("BLOCKS=", strlen("BLOCKS="), 1, archivo);
    fwrite(&size, sizeof(uint32_t), 1, archivo);

    fwrite(bitmap, sizeof(bitmap), 1, archivo);

    fflush(archivo);
}

void iniciar_blocks(int filedescriptor_blocks) {
    uint32_t block_size = obtener_tamanio_bloque();
    uint32_t size = obtener_cantidad_bloques();

    unsigned char* mapa = (char*) mmap(NULL, block_size * size, PROT_NONE, MAP_SHARED, filedescriptor_blocks, 0); // Revisar flags

    memcpy(directorio.mapa_blocks, mapa, sizeof(mapa));
}

void inicializar_bloque(int numero_bloque) { // Inicializa bloques de recursos con whitespace, para funciones de agregado y quitado

    for (int i; i < (TAMANIO_BLOQUE); i++) {
        *(directorio.mapa_blocks +  TAMANIO_BLOQUE * numero_bloque + i) = ' ';
    }

    msync(directorio.mapa_blocks, (numero_bloque + 1) * TAMANIO_BLOQUE, MS_ASYNC);

}

void inicializar_mapa() {
	//TODO
}

int obtener_tamanio_bloque() {
    fseek(directorio.superbloque, strlen("BITE_SIZE="), SEEK_SET);
    uint32_t block_size;
    fread(&block_size, sizeof(uint32_t), 1, directorio.superbloque);

    return block_size;
}

int obtener_cantidad_bloques() {
    obtener_tamanio_bloque();

    fseek(directorio.superbloque, strlen("BLOCKS=["), SEEK_CUR);
    uint32_t size;
    fread(&size, sizeof(uint32_t), 1, directorio.superbloque);

    return size;
}

void reescribir_superbloque(int tamanio, int cantidad, t_bitarray* bitmap) {
    fclose(directorio.superbloque);
    directorio.superbloque = fopen(path_superbloque, "w+");

    fwrite("BITE_SIZE=", strlen("BITE_SIZE="), 1, archivo);
    fwrite(&tamanio, sizeof(uint32_t), 1, archivo);

    fwrite("BLOCKS=", strlen("BLOCKS="), 1, archivo);
    fwrite(&cantidad, sizeof(uint32_t), 1, archivo);

    fwrite(bitmap, sizeof(bitmap), 1, archivo);

    fflush(archivo);
}