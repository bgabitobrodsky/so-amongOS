/*
 ============================================================================
 Name        : Discordiador.c
 Author      : Rey de fuego
 Version     : 1
 Copyright   : Mala yuyu
 Description : El discordiador
 ============================================================================
 */

#include "Discordiador.h"

int main(int argc, char *argv[]) {

	/*
	pthread_t nombreHilo;
	pthread_create(&nombreHilo, NULL, (void*) funcionAEjecutar, NULL);
	*/


	while(1){
		leer_consola();
	}


	return EXIT_SUCCESS;
}

void leer_consola(){
	char* leido = readline(">>>");
	char** comando= string_split(leido, " ");
	int argc = (int) (sizeof comando / sizeof(comando[0])) - 1;

	printf("%s", comando[0]);
	printf("%s", strlen(comando[1]));
	printf("%d", argc);
	/*if(strncmp(comando[0], "INICIAR_PATOTA", 15)){
		iniciar_patota(argc);
	}*/
	free(leido);
	free(comando);

}

void iniciar_patota(int argc, char* argv[]){

}
