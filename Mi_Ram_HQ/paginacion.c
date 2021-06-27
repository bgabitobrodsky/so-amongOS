#include "paginacion.h"
#define TAMANIO_PAGINA config_get_int_value(config, "TAMANIO_PAGINA")


int completar_pagina(pagina* pagina, void* data, int tam){
	//esta función devuelve lo que logró guardar
	int ocupado = pagina->tamano_ocupado;
	int disponible = TAMANIO_PAGINA - espacio_ocupado;
	marco* marco = pagina->puntero_marco;

	// me alcanza con esa pagina?
	if(disponible + tam <= TAMANIO_PAGINA){
		pagina->tamano_ocupado += tam;
		memcpy(memoria_principal + marco->base + ocupado, data, tam);
		return tam; // pude guardar todo
	}

	pagina->tamano_ocupado = TAMANIO_PAGINA;
	memcpy(memoria_principal + marco->base + ocupado, data, disponible);
	return disponible; // pude guardar solo el tamaño disponible 
}


int agregar_paginas_segun_tamano(tabla_paginas* tabla, void* data, int tam){
	// esta función devuelve la dirección lógica de lo que se guardó
	int direccion_logica;
	int progreso = 0;
	int numero_pagina;
	int offset;

	pagina* ultima_pagina = pagina_incompleta(tabla_paginas* tabla);
	if(ultima_pagina != NULL){ // si hay una pagina incompleta
		numero_pagina = list_size(tabla->paginas) - 1;
		offset = ultima_pagina->tamano_ocupado;
		progreso += completar_pagina(ultima_pagina, data, tam);
	}else{
		numero_pagina = list_size(tabla->paginas);
		offset = 0;
	}
	direccion_logica = numero_pagina * TAMANIO_PAGINA + offset;

	while(progreso < tam){
		progreso += agregar_pagina(tabla, data, tam - progreso);
	}

	return direccion_logica;	
}


int agregar_pagina(tabla_paginas* tabla, void* data, int tam){
	// retorna lo que se logró guardar
	marco* marco = asignar_marco();
	pagina* pagina = crear_pagina(marco);
	list_add(tabla->paginas,pagina);

	if(tam <= TAMANIO_PAGINA){
		memcpy(memoria_principal + marco->base, data, tam);
		pagina->tamano_ocupado = tam;
		return tam;
	}else{
		memcpy(memoria_principal + marco->base, data, TAMANIO_PAGINA);
		pagina->tamano_ocupado = TAMANIO_PAGINA;
		return TAMANIO_PAGINA;
	}
}


pagina* pagina_incompleta(tabla_paginas* tabla){
	int size = list_size(tabla->paginas);
	if(size > 0){
		pagina* ultima_pagina = list_get(tabla->paginas, size - 1);
		if(ultima_pagina->tamano_ocupado <= TAMANIO_PAGINA)
			return ultima_pagina;
	}
	return NULL;
}


pagina* crear_pagina(marco* marco){
	pagina* pagina = malloc(sizeof(pagina));
    marco->libre = false;
	pagina->puntero_marco = marco;
	log_info(logger,"Se crea página para el marco de base %d", marco->base);
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
