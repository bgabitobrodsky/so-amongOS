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
	int cont = 0;

		while(1){
			if(comando[cont] != NULL)
				cont++;
			else
				break;
		}

	if(!strncmp(comando[0], "INICIAR_PATOTA", 14)){
		iniciar_patota(cont, comando);
	}

	free(leido);
	free(comando);

}

void iniciar_patota(int argc, char* argv[]){
	printf("PATOTA: cantidad de tripulantes %s, url: %s \n", argv[1], argv[2]);
	if(argc > 2){
		printf("Posiciones\n");
		for(int i = 3; i <= argc; i++){
			if(argv[i]==NULL)
				argv[i] = "0|0";
			printf("POSICION %d: %s \n", i-2, argv[i]);
		}
	}
	getchar();
}

