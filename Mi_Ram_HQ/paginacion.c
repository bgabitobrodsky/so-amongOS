#include "paginacion.h"
#define TAMANIO_PAGINA config_get_int_value(config, "TAMANIO_PAGINA")
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

void* rescatar_de_paginas(tabla_paginas* tabla, int dl, int tam){
	void* data = malloc(tam);
	pagina* pagina; 
	int faltante = tam;
	int numero_pagina = dl / TAMANIO_PAGINA;
	int offset = dl % TAMANIO_PAGINA;
	log_info(logger, "Se empieza a buscar en la pagina %d con offset %d, tam %d", numero_pagina, offset, tam);
	if(offset > 0){
		pagina = list_get(tabla->paginas, numero_pagina);
		faltante -= rescatar_de_marco(pagina->puntero_marco, data + tam - faltante, offset, faltante);
		numero_pagina++;
	}
	while(faltante > 0){
		pagina = list_get(tabla->paginas, numero_pagina);
		faltante -= rescatar_de_marco(pagina->puntero_marco, data + tam - faltante, 0, faltante);
		numero_pagina++;
	}

	return data;
}

int rescatar_de_marco(marco* marco, void* data, int offset, int tam){
	//esta funcion retorna lo que de rescató de memoria
	int tam_rescate = MIN(tam, TAMANIO_PAGINA - offset);
	log_info(logger, "Se rescatan %d bytes del marco con base %d, con offset %d, tam %d", tam_rescate, marco->base,offset,tam);
	memcpy(data, memoria_principal + marco->base + offset, tam_rescate);

	return tam_rescate;
}

int completar_pagina(pagina* pagina, void* data, int tam){
	//esta función devuelve lo que logró guardar
	int ocupado = pagina->tamano_ocupado;
	int disponible = TAMANIO_PAGINA - ocupado;
	marco* marco = pagina->puntero_marco;

	// me alcanza con esa pagina?
	if(disponible - tam >= 0){
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
	int dl;
	int progreso = 0;
	// para calcular la dl
	int numero_pagina;
	int offset;

	pagina* ultima_pagina = pagina_incompleta(tabla);
	if(ultima_pagina != NULL){ // si hay una pagina incompleta
		offset = ultima_pagina->tamano_ocupado;
		numero_pagina = list_size(tabla->paginas) - 1;
		progreso += completar_pagina(ultima_pagina, data, tam);
		log_info(logger, "Se completó la última página, progreso: %d / %d bytes", progreso, tam);
	}else{
		numero_pagina = list_size(tabla->paginas);
		offset = 0;
	}
	dl = numero_pagina * TAMANIO_PAGINA + offset;

	while(progreso < tam){
		progreso += agregar_pagina(tabla, data + progreso, tam - progreso);
		log_info(logger, "Progreso: %d / %d bytes", progreso, tam);
	}
	return dl;	
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
		if(ultima_pagina->tamano_ocupado < TAMANIO_PAGINA)
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


// TEST

void imprimir_paginas(int pid){
    tabla_paginas* tabla = buscar_tabla(pid);
	int size = list_size(tabla->paginas);

    printf("\n<------ PAGINAS de tabla %d -----------\n", pid);
    for(int i = 0; i < size; i++) {
        pagina* s = list_get(tabla->paginas, i);
        printf("pagina %d base: %d, tamaño ocupado: %d \n", i, s->puntero_marco->base, s->tamano_ocupado);
    }
    printf("------------------->\n");

}

void test_gestionar_tareas_paginacion(){
	t_archivo_tareas* arc = malloc(sizeof(t_archivo_tareas));
	arc->texto = "GENERAR_OXIGENO 10;1;1;1";
	arc->largo_texto = 25;
	arc->pid = 1;
	gestionar_tareas(arc);
    free(arc);

	imprimir_paginas(1);
	tabla_paginas* tabla = (tabla_paginas*) buscar_tabla(1);
	
	t_PCB* pcb = (t_PCB*) rescatar_de_paginas(tabla, tabla->dl_pcb, sizeof(t_PCB));
	log_info(logger,"El pid era: %d", pcb->PID);
	free(pcb);

	char* tareas = (char*) rescatar_de_paginas(tabla, 0, tabla->dl_pcb);
	log_info(logger,"Tareas: %s", tareas);
	free(tareas);

}
