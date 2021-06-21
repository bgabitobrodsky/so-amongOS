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


void agregar_paginas_segun_tamano(tabla_paginas* tabla, int tamano){
	int cant_marcos_completos = cantidad_marcos_completos(tamano);

	for(int i = 0; i < cant_marcos_completos; i++){
		agregar_pagina(tabla, TAMANIO_PAGINA);			
	}

	int tam_marco_incompleto = ocupa_marco_incompleto(tamano);

	if(marco_incompleto != 0){
		agregar_pagina(tabla, tam_marco_incompleto);			
	}
}

void agregar_pagina( tabla_paginas* tabla, int tamano){
	marco* marco = asignar_marco();
	// void* puntero_a_tareas = memcpy(memoria_principal + marco_tareas->base, archivo_tareas->texto, tamanio_tareas); TODO
	pagina* pagina = crear_pagina(marco, tamano);
	list_add(tabla->paginas,pagina);
}


pagina* crear_pagina(marco* marco, int ocupa){
	pagina* pagina = malloc(sizeof(pagina));
	pagina->puntero_marco = marco_tareas;
	pagina->tamano_ocupado = tam;

	return pagina;
}


int cantidad_marcos_completos(int tam){
	int marcos = tam/TAMANIO_PAGINA  
	return marcos;
}

int ocupa_marco_incompleto(int tam){
	int parte_ocupada = tam % TAMANIO_PAGINA 
	return parte_ocupada;
}

void liberar_marco(int num_marco){
    marco* x = list_get(marcos, num_marco);
    x->libre = true;
    log_info(logger, "Se libera el marco %d", num_marco);

}

marco* crear_marco(int base, bool libre){
    marco* nueva_marco = malloc(sizeof(marco));
    nueva_marco->base = base;
    nueva_marco->libre = libre;

    return nueva_marco;
}

marco* buscar_marco_libre(){
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