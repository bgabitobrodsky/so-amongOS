#include "mongo_archivos.h"

// Vars globales
t_log* logger_mongo;
t_config* config_mongo;
t_archivos archivos;
t_list* bitacoras;

void inicializar_archivos(char* path_files) { // TODO: Puede romper, implementar archivos de metadata
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

	// TODO: Verificacion y setteo del SuperBloque
	// TODO: Verificacion y operaciones sobre Blocks

	archivos.oxigeno     = fopen(path_oxigeno, "w+");
	archivos.comida      = fopen(path_comida, "w+");
	archivos.basura      = fopen(path_basura, "w+");
	archivos.superbloque = fopen(path_superbloque, "w+");
	archivos.blocks      = fopen(path_blocks, "w+");

	archivos.path_oxigeno = path_oxigeno;
	archivos.path_comida  = path_comida;
	archivos.path_comida  = path_comida;
	archivos.path_basura  = path_basura;

	free(path_superbloque);
	free(path_blocks);
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
		agregar(conseguir_archivo(codigo_archivo), cantidad, conseguir_char(codigo_archivo));
		log_info(logger_mongo, "Se agregaron %s unidades a %s.\n", string_itoa(cantidad), conseguir_tipo(conseguir_char(codigo_archivo)));
	}
	else{
		quitar(conseguir_archivo(codigo_archivo), conseguir_path(codigo_archivo), cantidad, conseguir_char(codigo_archivo));
		log_info(logger_mongo, "Se quitaron %s unidades a %s.\n", string_itoa(cantidad), conseguir_tipo(conseguir_char(codigo_archivo)));
	}
}

void agregar(FILE* archivo, int cantidad, char tipo) {
	//pthread_mutex_lock(conseguir_semaforo(tipo));
	fflush(archivo);
    for(int i = 0; i < cantidad; i++) {
		putc(tipo, archivo);
	}
    fflush(archivo);
    //pthread_mutex_unlock(conseguir_semaforo(tipo));
}

void agregar_unlocked(FILE* archivo, int cantidad, char tipo) {
	fflush(archivo);
    for(int i = 0; i < cantidad; i++) {
		putc(tipo, archivo);
	}
    fflush(archivo);
}

void quitar(FILE* archivo, char* path, int cantidad, char tipo) { // Puede explotar en manejo de fopens, revisar
	char c;
	int contador = 0;

	log_info(logger_mongo, "path: %s", path);
	//pthread_mutex_lock(conseguir_semaforo(tipo));
	for (c = getc(archivo); c != EOF; c = getc(archivo))
        contador++;

	int nueva_cantidad = max(contador + cantidad, 0); // Cantidad es negativo en este caso
    fclose(archivo); //Lo reseteo. Testear
	archivo = fopen(path, "w+"); // Lo reabro con w+ para no joder otras funciones, revisar
    
    agregar_unlocked(archivo, nueva_cantidad, tipo);
    //pthread_mutex_unlock(conseguir_semaforo(tipo));
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

char* conseguir_path(int codigo) {
	switch(codigo) {
		case OXIGENO:
			return archivos.path_oxigeno;
			break;
		case COMIDA:
			return archivos.path_comida;
			break;
		case BASURA:
			return archivos.path_basura;
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

pthread_mutex_t* conseguir_semaforo(char tipo) {
    if (tipo == 'O')
        return &mutex_oxigeno;
    if (tipo == 'C')
        return &mutex_comida;
    if (tipo == 'B')
        return &mutex_basura;
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
