#include "generales.h"

t_tarea* crear_tarea(char* string_tarea){

	char** palabras = string_split(string_tarea, " ");

	if(contar_palabras(palabras) == 1){

		// es tarea generica
		char** tarea_s = string_split(palabras[0], ";");
		t_tarea* tarea = malloc(sizeof(t_tarea));
		tarea->nombre = malloc(strlen(tarea_s[0]) + 1);
		strcpy(tarea->nombre, tarea_s[0]);
		tarea->largo_nombre = strlen(tarea_s[0]);
		tarea->coord_x = atoi(tarea_s[1]);
		tarea->coord_y = atoi(tarea_s[2]);
		tarea->duracion = atoi(tarea_s[3]);
		liberar_puntero_doble(palabras);
		liberar_puntero_doble(tarea_s);
		return tarea;

	}
	else{

		char** tarea_s = string_split(palabras[1], ";");

		t_tarea* tarea = malloc(sizeof(t_tarea));
		tarea->nombre = malloc(strlen(palabras[0]) + 1);
		strcpy(tarea->nombre, palabras[0]);
		tarea->largo_nombre = strlen(palabras[0]);
		tarea->parametro = atoi(tarea_s[0]);
		tarea->coord_x = atoi(tarea_s[1]);
		tarea->coord_y = atoi(tarea_s[2]);
		tarea->duracion = atoi(tarea_s[3]);
		liberar_puntero_doble(palabras);
		liberar_puntero_doble(tarea_s);
		return tarea;

	}

}

int comparar_strings(char* str, char* str2) {

	return !strncmp(str, str2, strlen(str2));

}

void liberar_puntero_doble(char** palabras){

	int contador = 0;

	while (palabras[contador] != NULL) {
		contador++;
	}

	for(int i = 0; i<contador; i++){
		free(palabras[i]);
	}
	free(palabras);

}


char* leer_archivo_entero(char* path){
	// NOTA: esta funcion necesita LIBERAR el RETORNO

	FILE* archivo = fopen(path, "r");
	char* contenido;

	if (archivo == NULL){
		printf("Archivo inexistente.\n");
		return NULL;
	}
	else{
		fseek(archivo, 0, SEEK_END);
		int tamanio_archivo = ftell(archivo);
		fseek(archivo, 0, SEEK_SET);
		contenido = malloc(tamanio_archivo + 1);
		fread(contenido, 1, tamanio_archivo, archivo);
		contenido[tamanio_archivo] = '\0';
	}

	fclose(archivo);
	return contenido;

}

void monitor_cola_push(pthread_mutex_t semaforo, t_queue* cola, void* elemento_a_insertar) {
	// NO tiene retorno

	pthread_mutex_lock(&semaforo);
	queue_push(cola, elemento_a_insertar);
	pthread_mutex_unlock(&semaforo);

}

void* monitor_cola_pop(pthread_mutex_t semaforo, t_queue* cola) {
	// TIENE RETORNO

	void* aux;

	pthread_mutex_lock(&semaforo);
	if(!queue_is_empty(cola)){
		aux = queue_pop(cola);
	}
	else{
		return (void*) -1;
	}
	pthread_mutex_unlock(&semaforo);

	return aux;

}

void* monitor_cola_pop_or_peek(pthread_mutex_t semaforo, void*(*operacion)(t_queue*), t_queue* cola) {
	// TIENE RETORNO
	// Funciona usa para pop/peek/clean

	void* aux;

	pthread_mutex_lock(&semaforo);
	if(!queue_is_empty(cola)){
		aux = operacion(cola);
	}
	else{
		return (void*) -1;
	}
	pthread_mutex_unlock(&semaforo);

	return aux;

}

void* monitor_lista(pthread_mutex_t semaforo, void*(*operacion)(t_list*, void*), t_list* lista, void* elemento) {
	// TIENE RETORNO

	void* aux;

	pthread_mutex_lock(&semaforo);
	aux = operacion(lista, elemento);
	pthread_mutex_unlock(&semaforo);

	return aux;

}

int contar_palabras (char** palabras){

	int contador = 0;

    while (palabras[contador] != NULL && *palabras[contador] != '\0') {
        contador++;
    }
    return contador;

}

char* fecha_y_hora() {

    time_t tiempo = time(NULL);
    struct tm tiempoLocal = *localtime(&tiempo);
    static char fecha_Hora[70];
    char *formato = "%d-%m-%Y %H:%M:%S";
    int bytesEscritos = strftime(fecha_Hora, sizeof fecha_Hora, formato, &tiempoLocal);

    if (bytesEscritos != 0) {
        return fecha_Hora;
    }
    else {
        return "Error formateando fecha";
    }

}

int esta_en_lista(t_list* lista, int elemento){

    bool contiene(void* elemento1){
    	return sonIguales(elemento, *((int*) elemento1));
    }
    int a = list_any_satisfy(lista, contiene);
    return a;

}

int sonIguales(int elemento1, int elemento2){

    return elemento1 == elemento2;

}

