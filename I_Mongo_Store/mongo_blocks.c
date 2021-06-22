#include "mongo_blocks.h"

void iniciar_superbloque(FILE* archivo) { // No se destruye bitarray
    uint32_t block_size = TAMANIO_BLOQUE; // Bytes
    uint32_t size = CANTIDAD_BLOQUES;
    void* puntero_a_bits = malloc(TAMANIO_BLOQUE * 8); //De javi: con calloc te inicializa el puntero con todos 0
    t_bitarray* bitmap = bitarray_create_with_mode(puntero_a_bits, TAMANIO_BLOQUE * 8, LSB_FIRST); //De javi: ¿No sería CANTIDAD_BLOQUES?
    //t_bitarray* bitmap = bitarray_create_with_mode(bitmap->bitarray, CANTIDAD_BLOQUES, LSB_FIRST);

    for(int i; i < CANTIDAD_BLOQUES; i++) {
        bitarray_clean_bit(bitmap, i);
    }

    fwrite("BITE_SIZE=", strlen("BITE_SIZE="), 1, archivo);
    fwrite(&block_size, sizeof(uint32_t), 1, archivo);

    fwrite("BLOCKS=", strlen("BLOCKS="), 1, archivo);
    fwrite(&size, sizeof(uint32_t), 1, archivo);

    fwrite(bitmap, sizeof(bitmap), 1, archivo);
    //Puede ser
    //fwrite(bitmap->bitarray, sizeof(char/CHAR_BIT), CANTIDAD_BLOQUES, archivo);
}

void iniciar_blocks(int filedescriptor_blocks) {
    fseek(archivos.superbloque, strlen("BITE_SIZE="), SEEK_SET);
    uint32_t block_size;
    fread(&block_size, sizeof(uint32_t), 1, archivos.superbloque);

    fseek(archivos.superbloque, strlen("BLOCKS="), SEEK_CUR);
    uint32_t size;
    fread(&size, sizeof(uint32_t), 1, archivos.superbloque);

    unsigned char* mapa = (char*) mmap(NULL, block_size * size, PROT_NONE, MAP_SHARED, filedescriptor_blocks, 0); // Revisar flags

    memcpy(archivos.mapa_blocks, mapa, sizeof(mapa)); 
}

void inicializar_bloque(int numero_bloque) { // Inicializa bloques de recursos con whitespace, para funciones de agregado y quitado

    for (int i; i < (TAMANIO_BLOQUE); i++) {
        *(archivos.mapa_blocks +  TAMANIO_BLOQUE * numero_bloque + i) = '';
    }

    msync(archivos.mapa_blocks, (numero_bloque + 1) * TAMANIO_BLOQUE, MS_ASYNC);

}