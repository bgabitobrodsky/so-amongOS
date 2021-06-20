#include "segmentacion.h"

#define CRITERIO_SELECCION config_get_string_value(config, "CRITERIO_SELECCION")

void ordenar_segmentos(){
    bool segmento_anterior(segmento* segmento_antes, segmento* segmento_despues) {
        return segmento_antes->base < segmento_despues->base;
    }
	log_info(logger,"Comienzo a ordenar los segmentos");
    list_sort(segmentos, (void*) segmento_anterior);
	log_info(logger,"Segmentos ordenados");
    return;
}

void liberar_segmento(int base){
    for(int i = 0; i<list_size(segmentos);i++){
        segmento* x = list_get(segmentos, i);
        if(x->base == base) {
            x->libre = true;
            log_info(logger, "Se elimina el segmento con base %d", x->base);
        }
    }
    ordenar_segmentos();
}

void compactacion(){
    log_debug(logger, "Se comienza la compactacion");
    int size = list_size(segmentos);
    for(int i=0; i<size;i++){
        segmento* segmento_libre = list_get(segmentos, i);
        if(segmento_libre->libre){
            for(int z = i+1; z < size; z++){
                segmento* segmento_ocupado = list_get(segmentos, z);
                if(!segmento_ocupado->libre){

                    // Movemos primero la memoria real
                    /*memcpy(TAMANIO_MEMORIA + segmento_libre->base,
                           segmento_ocupado->mensaje->puntero_a_memoria,
                           segmento_ocupado->mensaje->tam);
                    segmento_ocupado->mensaje->puntero_a_memoria = TAMANIO_MEMORIA + segmento_libre->base;
					*/

                    // Despues acomodamos las estrucuras
                    segmento_ocupado->base = segmento_libre->base;
                    segmento_libre->base += segmento_ocupado->tam;

                    ordenar_segmentos();
                    unificar_segmentos_libres();
                    size = list_size(segmentos);
                    break;
                }
            }
        }
    }
    return;
}

// Recorro la tabla, si encuentro dos segmentos libres consecutivos los uno
void unificar_segmentos_libres(){
    int size = list_size(segmentos);
    for(int i=0; i<size-1; i++){

        segmento* una_segmento = list_get(segmentos, i);
        segmento* siguiente_segmento = list_get(segmentos, i + 1);

        if (una_segmento->libre && siguiente_segmento->libre){
            una_segmento->tam += siguiente_segmento->tam;
            list_remove(segmentos, i+1);
            free(siguiente_segmento);
            size = list_size(segmentos);
            i = 0;
        }
    }
    return;
}

segmento* crear_segmento(int base, int tam, bool libre){
    segmento* nuevo_segmento = malloc(sizeof(segmento));
    nuevo_segmento->base = base;
    nuevo_segmento->tam = tam;
    nuevo_segmento->libre = libre;

    return nuevo_segmento;
}

segmento* buscar_segmento_libre(int tam){
	if (strcmp(CRITERIO_SELECCION, "FF") == 0) {
        log_debug(logger, "Empieza busqueda FirstFit");
        return first_fit(tam);
    } else if (strcmp(CRITERIO_SELECCION, "BF") == 0) {
        log_debug(logger, "Empieza busqueda BestFit");
        return best_fit(tam);
    } else {
        log_error(logger, "Metodo de asignacion desconocido");
        exit(EXIT_FAILURE);
    }
}

segmento* first_fit(int tam){
    int size = list_size(segmentos);
    for(int i=0; i<size; i++){
        segmento* x = list_get(segmentos, i);
        if(x->libre == true && tam <= x->tam){
            log_info(logger, "Segmento libre encontrado (base: %d)", x->base);
            return x;
        }
    }
    log_warning(logger, "No se encontró segmento");
    return NULL;
}

segmento* best_fit(int tam){
	//TODO: Se puede mejorar la algoritmia, ahora recorre todo 2 veces, podria ir seleccionando al mejor candidato mientras recorre la primera vez
    int size = list_size(segmentos);
    t_list* candidatos = list_create();
    for(int i=0; i<size; i++){
        segmento* x = list_get(segmentos, i);
        if(x->libre == true && tam <= x->tam){
            log_info(logger, "Segmento libre con suficiente espacio encontrado (base: %d)", x->base);
            if(tam == x->tam){
				log_info(logger, "Mejor segmento encontrado (base:%d)", x->base);
				return x;
			}
            list_add(candidatos, x);
        }
    }
    log_debug(logger, "Buscando el mejor segmento");
    int candidatos_size = list_size(candidatos);
    if(candidatos_size != 0){
        segmento* best_fit;
        int best_fit_diff = 999999;
        for(int i=0; i<candidatos_size; i++){
            segmento* y = list_get(candidatos, i);
            int diff = y->tam - tam;
            if(best_fit_diff > diff){
                best_fit_diff = diff;
                best_fit = y;
            }
        }
        log_info(logger, "Mejor segmento encontrado (base:%d)", best_fit->base);
        return best_fit;
    }else{
        log_warning(logger, "No hay segmentos disponibles");
        return NULL;
    }
}

segmento* asignar_segmento(int tam){
	segmento* segmento_libre = buscar_segmento_libre(tam);
	if(segmento_libre != NULL){
		//Si el segmento es del tamaño justo, no tengo que reordenar
		if(segmento_libre->tam == tam){
			segmento_libre->libre = false;
			log_info(logger,"Segmento asignado (base:%d)", segmento_libre->base);
			return segmento_libre;
		}
		//Si no tengo que dividir el segmento
		else{
			segmento* nuevo_segmento = crear_segmento(segmento_libre->base,tam,false);
			list_add(segmentos,nuevo_segmento);
			segmento_libre->base += tam;
			segmento_libre->tam -= tam;
			log_info(logger,"Segmento asignado (base:%d)", nuevo_segmento->base);
			//Ordeno los segmentos por base ascendente
			ordenar_segmentos();

			return nuevo_segmento;
		}
	}else{
		//TODO
	}
}

tabla_segmentos* crear_tabla_segmentos(uint32_t pid){
	tabla_segmentos* nueva_tabla = malloc(sizeof(tabla_segmentos));
	nueva_tabla->segmentos_tcb = list_create();
	list_add(indices,crear_indice(pid, (void*) nueva_tabla));
	return nueva_tabla;
}


void test_segmentos(){
	segmento* seg = asignar_segmento(sizeof(char[2]));
	segmento* seg2 = asignar_segmento(sizeof(char));
	segmento* seg3 = asignar_segmento(sizeof(char));
	liberar_segmento(0);
	liberar_segmento(3);
	compactacion();
	segmento* seg4 = asignar_segmento(sizeof(char));
	print_segmentos_info();
}

void test_tabla_segmentos(){
	log_info(logger,"Se crea la tabla de segmentos pid: 1");
	tabla_segmentos* tabla1 = crear_tabla_segmentos(1);
	log_info(logger,"Se crea segmento para pcb 1");
	tabla1->segmento_pcb = asignar_segmento(8);
	log_info(logger,"Segmento para pcb creado 1, base: %d", tabla1->segmento_pcb->base);
	log_info(logger,"Se crea segmento para tareas 1");
	tabla1->segmento_tareas = asignar_segmento(45);
	log_info(logger,"Segmento para tareas creado 1, base: %d", tabla1->segmento_tareas->base);

	list_add(indices,crear_indice(1, (void*) tabla1));

	log_info(logger,"Se crea la tabla de segmentos pid: 2");
	tabla_segmentos* tabla2 = crear_tabla_segmentos(2);
	log_info(logger,"Se crea segmento para pcb 2");
	tabla2->segmento_pcb = asignar_segmento(8);
	log_info(logger,"Segmento para pcb creado 2, base: %d", tabla2->segmento_pcb->base);
	log_info(logger,"Se crea segmento para tareas 2");
	tabla2->segmento_tareas = asignar_segmento(21);
	log_info(logger,"Segmento para tareas creado 2, base: %d", tabla2->segmento_tareas->base);

	list_add(indices,crear_indice(2, (void*) tabla2));
	
	tabla_segmentos* t_seg = buscar_tabla(2);
	log_info(logger,"Base del segmento de tareas de la tabla 2: %d",t_seg->segmento_tareas->base);

	print_segmentos_info();
	print_tablas_segmentos_info();
}

void test_gestionar_tarea(int pid){
	t_archivo_tareas* archivo = malloc(sizeof(t_archivo_tareas));
	archivo->texto = "GENERAR_OXIGENO 12;2;3;5\nGENERAR_OXIGENO 12;2;3;5\nGENERAR_OXIGENO 12;2;3;5";
	archivo->largo_texto = 74;
	archivo->pid = pid;

	gestionar_tareas(archivo);
    free(archivo);
	print_segmentos_info();
	print_tablas_segmentos_info();
}

void test_gestionar_tcb(){
	test_gestionar_tarea(1);

    t_TCB* tcb = malloc(sizeof(t_TCB));
    tcb->TID = 10001;
    tcb->coord_x = 1;
    tcb->coord_y = 2;
    tcb->estado_tripulante = 'N';
    gestionar_tcb(tcb);
    free(tcb);

    t_TCB* tcb2 = malloc(sizeof(t_TCB));
    tcb2->TID = 10002;
    tcb2->coord_x = 5;
    tcb2->coord_y = 2;
    tcb2->estado_tripulante = 'N';
    gestionar_tcb(tcb2);
    free(tcb2);

	print_segmentos_info();
	print_tablas_segmentos_info();
    t_TCB* tcb_recuperado = buscar_tcb_por_tid(10002);
    log_debug(logger,"La coord_x del segundo tripulante buscado por tid es: %d", tcb_recuperado->coord_x);
    t_list* lista_tcbs = buscar_tcbs_por_pid(1);
    t_TCB* tcb_recuperado2 = (t_TCB*) list_get(lista_tcbs,1);
    log_debug(logger,"La coord_x del segundo tripulante buscado por pid es: %d", tcb_recuperado2->coord_x);
}

void print_segmentos_info() {
    int size = list_size(segmentos);
    printf("\n<------ SEGMENTOS -----------\n");
    for(int i=0; i<size; i++) {
        segmento *s = list_get(segmentos, i);
        printf("base: %d, tam: %d, libre: %s\n", s->base, s->tam, s->libre ? "true" : "false");
    }
    printf("------------------->\n");
}

void print_tablas_segmentos_info(){
	int size = list_size(indices);
	printf("\n<----- TABLAS DE SEGMENTOS ---------------------\n");
	for(int i = 0; i < size; i++) {
        indice_tabla* index = list_get(indices, i);
		tabla_segmentos* tabla = (tabla_segmentos*) index->tabla;
		printf("Tabla pid: %d\n", index->pid);
		printf("\t Segmento PCB:\n");
		printf("\t\t base: %d\n",tabla->segmento_pcb->base);
		printf("\t\t tam: %d\n",tabla->segmento_pcb->tam);
		printf("\t Segmento Tarea:\n");
		printf("\t\t base: %d\n",tabla->segmento_tareas->base);
		printf("\t\t tam: %d\n",tabla->segmento_tareas->tam);
    }
	printf("------------------>\n");
}
