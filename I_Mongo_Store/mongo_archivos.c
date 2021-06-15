#include "mongo_archivos.h"

void inicializar_archivos(char* path_files) { // TODO: Puede romper, implementar archivos de metadata
	char* path_oxigeno;
	sprintf(path_oxigeno, "%s/Oxigeno.ims", path_files);

	char* path_comida;
	sprintf(path_comida, "%s/Comida.ims", path_files);

	char* path_basura;
	sprintf(path_basura, "%s/Basura.ims", path_files);

	char* path_superbloque;
	sprintf(path_superbloque, "%s/SuperBloque.ims", path_files); // TODO: Implementar cosas con el superbloque

	char* path_blocks;
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

	archivos.oxigeno     = file_oxigeno;
	archivos.comida      = file_comida;
	archivos.basura      = file_basura;
    archivos.superbloque = file_superbloque;
    archivos.blocks      = file_blocks;

	free(path_oxigeno);
	free(path_comida);
	free(path_basura);
	free(path_superbloque);
	free(path_blocks);
}

void alterar(int codigo_archivo, int cantidad) { 
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
}

void alterar(int codigo_archivo, int cantidad) {  // Alternativa mas prolija, revisar si funciona
	if (cantidad >= 0)
		agregar(conseguir_archivo(codigo_archivo), cantidad, conseguir_char(codigo_archivo));
	else
		quitar(conseguir_archivo(codigo_archivo), cantidad, conseguir_char(codigo_archivo));
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
}

pthread_mutex_t conseguir_semaforo(char tipo) {
    if (tipo == 'O')
        return mutex_oxigeno;
    if (tipo == 'C')
        return mutex_comida;
    if (tipo == 'B')
        return mutex_basura; 
}

void agregar(FILE* archivo, int cantidad, char tipo) {
	pthread_mutex_lock(conseguir_semaforo(tipo));
    for(int i = 0; i < cantidad; i++) {
		putc(tipo, archivo);
	}
    pthread_mutex_unlock(conseguir_semaforo(tipo));
}

void agregar_unlocked(FILE* archivo, int cantidad, char tipo) {
    for(int i = 0; i < cantidad; i++) {
		putc(tipo, archivo);
	}
}

void quitar(FILE* archivo, int cantidad, char tipo) {
	char c;
	int contador = 0;

    pthread_mutex_lock(conseguir_semaforo(tipo));
	for (c = getc(archivo); c != EOF; c = getc(archivo))
        contador++;

	int nueva_cantidad = max(contador + cantidad, 0); // Cantidad es negativo en este caso
    fclose(archivo);
	fopen(archivo, "w"); // Reseteo archivo
	fclose(archivo);
	fopen(archivo, "r+"); // Lo reabro con r+ para no joder otras funciones, revisar
    
    agregar_unlocked(archivo, nueva_cantidad, tipo);
    pthread_mutex_unlock(conseguir_semaforo(tipo));
}