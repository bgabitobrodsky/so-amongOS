#include "paginacion.h"
#define TAMANIO_PAGINA config_get_int_value(config, "TAMANIO_PAGINA")


int completar_pagina(pagina* pagina, int tamano, tabla_paginas* tabla){
	int espacio_ocupado = pagina->tamano_ocupado;
	int espacio_necesario = espacio_ocupado + tamano;

	// me alcanza con esa pagina?
	if(espacio_necesario <= TAMANIO_PAGINA){
		pagina->tamano_ocupado = espacio_necesario;
		return 0;
	}
	pagina->tamano_ocupado = TAMANIO_PAGINA;
	int espacio_faltante = espacio_necesario - TAMANIO_PAGINA;

	return espacio_faltante;
}

void* rescatar_de_paginas(tabla_paginas* tabla, int dl, int tam){
	pagina* pagina; 
	int faltante = tam;
	void* data = malloc(tam);
	int numero_primer_pagina = dl / TAMANIO_PAGINA;
	int offset = dl % TAMANIO_PAGINA;

	if(offset > 0){
		pagina = list_get(tabla->paginas, numero_primer_pagina);
		marco = pagina->puntero_marco;
		faltante -= rescatar_de_marco(marco,data,offset,tam);
	}
	while(faltante > 0){
		pagina = list_get(tabla->paginas, ++numero_primer_pagina);
		faltante -= rescatar_de_marco(pagina->puntero_marco, data + tam - faltante, 0, faltante);
	}

	return data;
}

int rescatar_de_marco(marco* marco, void* data, int offset, int tam){
	//esta funcion retorna lo que de rescató de memoria
	int espacio_necesario = offset + tam;
	int tam_rescate = MIN(tam, TAMANIO_PAGINA-offset);

	memcpy(data, memoria_principal + marco->base + offset, tam_rescate);

	return tam_rescate;
}



void agregar_paginas_segun_tamano(tabla_paginas* tabla, void* data, int tamano){
	int cant_marcos_completos = cantidad_marcos_completos(tamano);

	for(int i = 0; i < cant_marcos_completos; i++){
		agregar_pagina(tabla, TAMANIO_PAGINA);			
	}

	int sub_tam = MIN(tam,TAMANIO_PAGINA);
	agregar_pagina(tabla, data, );
	
	memcpy(destino + desplazamiento, data + desplazamiento, MIN(tam,TAMANIO_PAGINA));
	if(tam > TAMANIO_PAGINA){
		desplazamiento = MIN(tam,TAMANIO_PAGINA);
		tam -= TAMANIO_PAGINA;
	}

	int tam_marco_incompleto = ocupa_marco_incompleto(tamano);

	if(tam_marco_incompleto > 0){
		agregar_pagina(tabla, tam_marco_incompleto);			
	}



}

void agregar_pagina(tabla_paginas* tabla, int tamano){

	marco* marco = asignar_marco();
	// void* puntero_a_tareas = memcpy(memoria_principal + marco_tareas->base, archivo_tareas->texto, tamanio_tareas); TODO
	pagina* pagina = crear_pagina(marco, tamano);
	list_add(tabla->paginas,pagina);
}


pagina* pagina_incompleta(tabla_paginas* tabla){
	int size = list_size(tabla->paginas);
	pagina* ultima_pagina = list_get(tabla->paginas, size - 1);
	if(ultima_pagina->tamano_ocupado <= TAMANIO_PAGINA)
		return ultima_pagina;
	return NULL;
}


pagina* crear_pagina(marco* marco, int ocupa){
	pagina* pagina = malloc(sizeof(pagina));
    marco->libre = false;
	pagina->puntero_marco = marco;
	pagina->tamano_ocupado = ocupa;
	log_info(logger,"Se crea página para el marco de base %d, que ocupa %d", marco->base, ocupa);
	return pagina;
}


int cantidad_marcos_completos(int tam){
    log_info(logger,"Calculando cantidad de marcos para el tamaño %d", tam);
	int marcos = tam / TAMANIO_PAGINA;  
	return marcos;
}

int ocupa_marco_incompleto(int tam){
	int parte_ocupada = tam % TAMANIO_PAGINA; 
	return parte_ocupada;
}

void liberar_marco(int num_marco){
    marco* x = list_get(marcos, num_marco);
    x->libre = true;
    log_info(logger, "Se libera el marco %d", num_marco);

}

marco* crear_marco(int base, bool libre){
    marco* nuevo_marco = malloc(sizeof(marco));
    nuevo_marco->base = base;
    nuevo_marco->libre = libre;

    return nuevo_marco;
}

tabla_paginas* crear_tabla_paginas(uint32_t pid){
	tabla_paginas* nueva_tabla = malloc(sizeof(tabla_paginas));
    log_info(logger,"se creo tabla de paginas de pid %d", pid);
	nueva_tabla->paginas = list_create();
	char spid[4];
	sprintf(spid, "%d", pid);
    dictionary_put(tablas,spid,nueva_tabla);
    log_debug(logger,"Tabla de pid: %d creada",pid);
    
	return nueva_tabla;
}


marco* buscar_marco_libre(){
    log_info(logger,"Buscando un marco libre");
    int size = list_size(marcos);
	for(int i=0; i<size; i++){
        marco* x = list_get(marcos, i);
        if(x->libre == true ){
            log_info(logger, "Marco libre encontrado (base: %d)", x->base);
            return x;
        }
    }
    log_warning(logger, "No se encontró marco");
    return NULL;
}

marco* asignar_marco(){

	marco* marco_libre = buscar_marco_libre();
	if(marco_libre != NULL){
		//Si el marco es del tamano justo, no tengo que reordenar
		marco_libre->libre = false;
		log_info(logger,"Marco asignado (base:%d)", marco_libre->base);
		return marco_libre;
	}
	else{
		//TODO
	}
}


void guardar_en_tabla(tabla_paginas* tabla, void* data, int tam){
	int desplazamiento = 0;
	
	memcpy(destino + desplazamiento, data + desplazamiento, MIN(tam,TAMANIO_PAGINA));
	if(tam > TAMANIO_PAGINA){
		desplazamiento = MIN(tam,TAMANIO_PAGINA);
		tam -= TAMANIO_PAGINA;
	}



}


// TEST

void imprimir_paginas(int pid){
    tabla_paginas* t_pag = buscar_tabla(pid);
    t_list* paginas = t_pag->paginas;

    printf("\n<------ PAGINAS de tabla %d -----------\n", pid);

    for(int i=0; i<list_size(paginas); i++) {
        pagina* s = list_get(paginas, i);
        printf("pagina %d base: %d, tamaño ocupado: %d \n", i + 1 , s->puntero_marco->base, s->tamano_ocupado);
    }
    printf("------------------->\n");

}
