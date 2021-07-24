#include "segmentacion.h"

#define ESQUEMA_MEMORIA config_get_string_value(config, "ESQUEMA_MEMORIA")
#define CRITERIO_SELECCION config_get_string_value(config, "CRITERIO_SELECCION")

// FFFFFFFFFFFFFFFFFFFFFFUUUUUUUU     UUUUUUUUNNNNNNNN        NNNNNNNN        CCCCCCCCCCCCCIIIIIIIIII     OOOOOOOOO     NNNNNNNN        NNNNNNNNEEEEEEEEEEEEEEEEEEEEEE   SSSSSSSSSSSSSSS 
// F::::::::::::::::::::FU::::::U     U::::::UN:::::::N       N::::::N     CCC::::::::::::CI::::::::I   OO:::::::::OO   N:::::::N       N::::::NE::::::::::::::::::::E SS:::::::::::::::S
// F::::::::::::::::::::FU::::::U     U::::::UN::::::::N      N::::::N   CC:::::::::::::::CI::::::::I OO:::::::::::::OO N::::::::N      N::::::NE::::::::::::::::::::ES:::::SSSSSS::::::S
// FF::::::FFFFFFFFF::::FUU:::::U     U:::::UUN:::::::::N     N::::::N  C:::::CCCCCCCC::::CII::::::IIO:::::::OOO:::::::ON:::::::::N     N::::::NEE::::::EEEEEEEEE::::ES:::::S     SSSSSSS
//   F:::::F       FFFFFF U:::::U     U:::::U N::::::::::N    N::::::N C:::::C       CCCCCC  I::::I  O::::::O   O::::::ON::::::::::N    N::::::N  E:::::E       EEEEEES:::::S            
//   F:::::F              U:::::D     D:::::U N:::::::::::N   N::::::NC:::::C                I::::I  O:::::O     O:::::ON:::::::::::N   N::::::N  E:::::E             S:::::S            
//   F::::::FFFFFFFFFF    U:::::D     D:::::U N:::::::N::::N  N::::::NC:::::C                I::::I  O:::::O     O:::::ON:::::::N::::N  N::::::N  E::::::EEEEEEEEEE    S::::SSSS         
//   F:::::::::::::::F    U:::::D     D:::::U N::::::N N::::N N::::::NC:::::C                I::::I  O:::::O     O:::::ON::::::N N::::N N::::::N  E:::::::::::::::E     SS::::::SSSSS    
//   F:::::::::::::::F    U:::::D     D:::::U N::::::N  N::::N:::::::NC:::::C                I::::I  O:::::O     O:::::ON::::::N  N::::N:::::::N  E:::::::::::::::E       SSS::::::::SS  
//   F::::::FFFFFFFFFF    U:::::D     D:::::U N::::::N   N:::::::::::NC:::::C                I::::I  O:::::O     O:::::ON::::::N   N:::::::::::N  E::::::EEEEEEEEEE          SSSSSS::::S 
//   F:::::F              U:::::D     D:::::U N::::::N    N::::::::::NC:::::C                I::::I  O:::::O     O:::::ON::::::N    N::::::::::N  E:::::E                         S:::::S
//   F:::::F              U::::::U   U::::::U N::::::N     N:::::::::N C:::::C       CCCCCC  I::::I  O::::::O   O::::::ON::::::N     N:::::::::N  E:::::E       EEEEEE            S:::::S
// FF:::::::FF            U:::::::UUU:::::::U N::::::N      N::::::::N  C:::::CCCCCCCC::::CII::::::IIO:::::::OOO:::::::ON::::::N      N::::::::NEE::::::EEEEEEEE:::::ESSSSSSS     S:::::S
// F::::::::FF             UU:::::::::::::UU  N::::::N       N:::::::N   CC:::::::::::::::CI::::::::I OO:::::::::::::OO N::::::N       N:::::::NE::::::::::::::::::::ES::::::SSSSSS:::::S
// F::::::::FF               UU:::::::::UU    N::::::N        N::::::N     CCC::::::::::::CI::::::::I   OO:::::::::OO   N::::::N        N::::::NE::::::::::::::::::::ES:::::::::::::::SS 
// FFFFFFFFFFF                 UUUUUUUUU      NNNNNNNN         NNNNNNN        CCCCCCCCCCCCCIIIIIIIIII     OOOOOOOOO     NNNNNNNN         NNNNNNNEEEEEEEEEEEEEEEEEEEEEE SSSSSSSSSSSSSSS

void ordenar_segmentos(){
    log_info(logger, "Ordenar segmentos");
    bool segmento_anterior(segmento* segmento_antes, segmento* segmento_despues){
        return segmento_antes->base < segmento_despues->base;
    }
    list_sort(segmentos, (void*) segmento_anterior);
}

void compactacion(){
    log_debug(logger, "[COMP]: Se comienza la compactacion");
    bloquear_lista_segmentos();
    unificar_segmentos_libres();
    int size = list_size(segmentos);
    for(int i = 0; i < size; i++){
        size = list_size(segmentos);
        segmento* segmento_libre = list_get(segmentos, i);
        if(segmento_libre->libre){
            log_trace(logger, "[COMP]: Encontre un segmento libre, base: %d, tam: %d", segmento_libre->base, segmento_libre->tam);
            for(int z = i + 1; z < size; z++){
                segmento* segmento_ocupado = list_get(segmentos, z);
                if(segmento_ocupado == NULL){
                    log_error(logger, "Segmento nulo");
                    break;
                }
                if(!segmento_ocupado->libre){
                    log_trace(logger, "[COMP]: Encontre un segmento ocupado, base: %d", segmento_ocupado->base);
                    bloquear_segmento(segmento_ocupado);
                    // Tengo que acomodar los fuckings punteros a memoria de las estructuras
                    if(segmento_ocupado->tipo == S_PCB){
                        log_trace(logger, "[COMP]: Es un segmento de PCB");
                        // en caso de ser un seg. de pcb tengo que actualizar el puntero a pcb de sus tcb
                        t_PCB* pcb = (t_PCB*) (memoria_principal + segmento_ocupado->base);
                        t_list* tcbs = buscar_tcbs_por_pid(pcb->PID);

                        // Movemos primero la memoria real
                        memcpy(memoria_principal + segmento_libre->base,
                            memoria_principal + segmento_ocupado->base,
                            segmento_ocupado->tam);

                        // Despues acomodamos las estrucuras
                        segmento_ocupado->base = segmento_libre->base;
                        segmento_libre->base += segmento_ocupado->tam;

                        void updater_tcb_pcb(void* un_tcb){
                            t_TCB* tcb = (t_TCB*) un_tcb;
                            log_trace(logger, "[COMP]: Muevo el puntero a PCB de TID: %d, -%dbytes", tcb->TID, segmento_libre->tam);
                            tcb->puntero_a_pcb -= segmento_libre->tam;
                            desbloquear_segmento_por_tid(tcb->TID);
                        }
                        list_iterate(tcbs,updater_tcb_pcb);

                    }else if(segmento_ocupado->tipo == S_TAREAS){
                        log_trace(logger, "[COMP]: Es un segmento de tareas");
                        // en caso de ser un seg. de tareas, tengo que actualizar el: pcb y los tcbs, pucha :(
                        tabla_segmentos* tabla;
                        int pid;

                        void buscador_tabla_por_tareas(char* spid, void* una_tabla){
                            tabla_segmentos* t = (tabla_segmentos*) una_tabla;
                            if(t->segmento_tareas->base == segmento_ocupado->base){
                                tabla = t;
                                pid = atoi(spid);
                            }
                        }
                        dictionary_iterator(tablas, buscador_tabla_por_tareas);

                        t_PCB* pcb = (t_PCB*) (memoria_principal + (tabla->segmento_pcb->base));
                        log_trace(logger, "[COMP]: Muevo el puntero a tareas de PID: %d, -%dbytes", pcb->PID, segmento_libre->tam);
                        pcb->direccion_tareas -= segmento_libre->tam;
                        
                        // Movemos primero la memoria real
                        memcpy(memoria_principal + segmento_libre->base,
                            memoria_principal + segmento_ocupado->base,
                            segmento_ocupado->tam);

                        // Despues acomodamos las estrucuras
                        segmento_ocupado->base = segmento_libre->base;
                        segmento_libre->base += segmento_ocupado->tam;

                        t_list* tcbs = buscar_tcbs_por_pid(pid);
                        
                        void updater_tcb_tareas(void* un_tcb){
                            t_TCB* tcb = (t_TCB*) un_tcb;
                            log_trace(logger, "[COMP]: Muevo el puntero a tareas de TID: %d, -%dbytes", tcb->TID, segmento_libre->tam);
                            if((void*)tcb->siguiente_instruccion != NULL){
                                tcb->siguiente_instruccion -= segmento_libre->tam;
                            }
                            desbloquear_segmento_por_tid(tcb->TID);
                        }
                        list_iterate(tcbs,updater_tcb_tareas);

                        void desbloqueador_de_tcb(void* un_tcb){
                            t_TCB* tcb = (t_TCB*) un_tcb;
                            desbloquear_segmento_por_tid(tcb->TID);
                        }
                        list_iterate(tcbs,desbloqueador_de_tcb);

                    }else if(segmento_ocupado->tipo == S_TCB){
                        log_trace(logger, "[COMP]: Es un segmento de TCB");
                        //por suerte si se mueve un segmento de un tcb no tengo que hacer nada yeiii :D
                        // Movemos primero la memoria real
                        memcpy(memoria_principal + segmento_libre->base,
                            memoria_principal + segmento_ocupado->base,
                            segmento_ocupado->tam);

                        segmento_ocupado->base = segmento_libre->base;
                        segmento_libre->base += segmento_ocupado->tam;

                    }
                    desbloquear_segmento(segmento_ocupado);
                    ordenar_segmentos();
                    unificar_segmentos_libres();
                    break;
                }else{
                    log_trace(logger, "[COMP]: No encontre segmento ocupado");
                }
            }
        }
    }
    //ordenar_segmentos();
    //unificar_segmentos_libres();
    desbloquear_lista_segmentos();
    log_debug(logger, "[COMP]: Compactación terminada");
    return;
}

// Recorro la tabla, si encuentro dos segmentos libres consecutivos los uno
void unificar_segmentos_libres(){
    log_info(logger, "Unifico segmentos libres");
    int size = list_size(segmentos);
    for(int i = 0; i < size-1; i++){
        segmento* seg1 = list_get(segmentos, i);
        segmento* seg2 = list_get(segmentos, i + 1);
        if (seg1->libre && seg2->libre){
            seg1->tam += seg2->tam;
            list_remove(segmentos, i + 1);
            free(seg2);
            size = list_size(segmentos);
            i--;
        }
    }
}

void unificar_dos_segmentos_libres(int i,int z){
    int size = list_size(segmentos);
    if(i + 1 < size){
        log_trace(logger, "[COMP]: Unifico 2 segmentos libres");
        segmento* seg1 = list_get(segmentos, i);
        segmento* seg2 = list_get(segmentos, z);
        seg1->tam += seg2->tam;
        list_remove(segmentos, z);
        desbloquear_segmento(seg2);
        free(seg2);
    }
}


segmento* crear_segmento(int base, int tam, bool libre){
    segmento* nuevo_segmento = malloc(sizeof(segmento));
    nuevo_segmento->base = base;
    nuevo_segmento->tam = tam;
    nuevo_segmento->libre = libre;
    pthread_mutex_init(&(nuevo_segmento->mutex),NULL);

    return nuevo_segmento;
}

segmento* buscar_segmento_libre(int tam){
    segmento* segmento;
    bloquear_lista_segmentos();
    log_info(logger,"Buscando segmento libre");
	if (strcmp(CRITERIO_SELECCION, "FF") == 0) {
        segmento = first_fit(tam);
    } else if (strcmp(CRITERIO_SELECCION, "BF") == 0) {
        segmento = best_fit(tam);
    } else {
        log_error(logger, "Metodo de asignacion desconocido");
        exit(EXIT_FAILURE);
    }
    desbloquear_lista_segmentos();
    if(segmento == NULL)
        log_warning(logger,"No se encontró segmento libre");
    return segmento;
}

segmento* first_fit(int tam){
    int size = list_size(segmentos);
    for(int i=0; i<size; i++){
        segmento* x = list_get(segmentos, i);
        if(x->libre == true && tam <= x->tam){
            return x;
        }
    }
    return NULL;
}

segmento* best_fit(int tam){
    int size = list_size(segmentos);
    t_list* candidatos = list_create();
    for(int i=0; i<size; i++){
        segmento* x = list_get(segmentos, i);
        if(x->libre == true && tam <= x->tam){
            if(tam == x->tam){
                list_destroy(candidatos);
				return x;
			}
            list_add(candidatos, x);
        }
    }
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
        list_destroy(candidatos);
        return best_fit;
    }else{
        list_destroy(candidatos);
        return NULL;
    }
}

segmento* asignar_segmento(int tam){

	log_info(logger, "Asignando segmento");
    segmento* segmento_libre = buscar_segmento_libre(tam);
	if(segmento_libre != NULL){
        intento_asignar_segmento = 0;
		if(segmento_libre->tam == tam){
			segmento_libre->libre = false;
			return segmento_libre;
		}else{
		//Si no tengo que dividir el segmento
			segmento* nuevo_segmento = crear_segmento(segmento_libre->base,tam,false);
			list_add(segmentos,nuevo_segmento);
			segmento_libre->base += tam;
			segmento_libre->tam -= tam;
			ordenar_segmentos();
			return nuevo_segmento;
		}
	}else{
        if(intento_asignar_segmento == 1){
            intento_asignar_segmento = 0;
            log_error(logger,"No hay mas memoria bro");
            return NULL;
        }
        intento_asignar_segmento = 1;
        compactacion();
        return asignar_segmento(tam);
	}
}

tabla_segmentos* crear_tabla_segmentos(int pid){
    char spid[4];
	sprintf(spid, "%d", pid);
	if(!dictionary_has_key(tablas, spid)){
        tabla_segmentos* nueva_tabla = malloc(sizeof(tabla_segmentos));
        nueva_tabla->segmento_pcb = NULL;
        nueva_tabla->segmento_tareas = NULL;
        nueva_tabla->segmentos_tcb = list_create();
        pthread_mutex_init(&(nueva_tabla->mutex), NULL);
        bloquear_lista_tablas();
        dictionary_put(tablas,spid,nueva_tabla);
        desbloquear_lista_tablas();
        log_debug(logger,"Tabla creada PID: %d",pid);
        return nueva_tabla;
    }else{
        log_error(logger, "Ya hay una tabla creada con PID: %d", pid);
        return buscar_tabla(pid);
    }
}

void matar_tabla_segmentos(int pid){
    log_info(logger, "Eliminando la patota PID: %d", pid);

    char spid[4];
    sprintf(spid, "%d", pid);
    
    void table_destroyer(void* una_tabla){
        tabla_segmentos* tabla = (tabla_segmentos*) una_tabla;

        if(tabla->segmento_pcb != NULL){
            tabla->segmento_pcb->libre = true;
            desbloquear_segmento(tabla->segmento_pcb);
        }
        if(tabla->segmento_tareas != NULL){
            tabla->segmento_tareas->libre = true;
            desbloquear_segmento(tabla->segmento_tareas);
        }
        int size = list_size(tabla->segmentos_tcb);
        if(size > 0){
            void tcb_destroyer(void* un_segmento){
                segmento* seg = (segmento*) un_segmento;
                seg->libre = true;
                desbloquear_segmento(seg);
            }
            list_destroy_and_destroy_elements(tabla->segmentos_tcb,tcb_destroyer);
        }else{
            list_destroy(tabla->segmentos_tcb);
        }
        free(tabla);
    }
    
    bloquear_lista_tablas();
    dictionary_remove_and_destroy(tablas,spid,table_destroyer);
    desbloquear_lista_tablas();
}

//   SSSSSSSSSSSSSSS EEEEEEEEEEEEEEEEEEEEEEMMMMMMMM               MMMMMMMM               AAA               FFFFFFFFFFFFFFFFFFFFFF     OOOOOOOOO     RRRRRRRRRRRRRRRRR        OOOOOOOOO        SSSSSSSSSSSSSSS 
//  SS:::::::::::::::SE::::::::::::::::::::EM:::::::M             M:::::::M              A:::A              F::::::::::::::::::::F   OO:::::::::OO   R::::::::::::::::R     OO:::::::::OO    SS:::::::::::::::S
// S:::::SSSSSS::::::SE::::::::::::::::::::EM::::::::M           M::::::::M             A:::::A             F::::::::::::::::::::F OO:::::::::::::OO R::::::RRRRRR:::::R  OO:::::::::::::OO S:::::SSSSSS::::::S
// S:::::S     SSSSSSSEE::::::EEEEEEEEE::::EM:::::::::M         M:::::::::M            A:::::::A            FF::::::FFFFFFFFF::::FO:::::::OOO:::::::ORR:::::R     R:::::RO:::::::OOO:::::::OS:::::S     SSSSSSS
// S:::::S              E:::::E       EEEEEEM::::::::::M       M::::::::::M           A:::::::::A             F:::::F       FFFFFFO::::::O   O::::::O  R::::R     R:::::RO::::::O   O::::::OS:::::S            
// S:::::S              E:::::E             M:::::::::::M     M:::::::::::M          A:::::A:::::A            F:::::F             O:::::O     O:::::O  R::::R     R:::::RO:::::O     O:::::OS:::::S            
//  S::::SSSS           E::::::EEEEEEEEEE   M:::::::M::::M   M::::M:::::::M         A:::::A A:::::A           F::::::FFFFFFFFFF   O:::::O     O:::::O  R::::RRRRRR:::::R O:::::O     O:::::O S::::SSSS         
//   SS::::::SSSSS      E:::::::::::::::E   M::::::M M::::M M::::M M::::::M        A:::::A   A:::::A          F:::::::::::::::F   O:::::O     O:::::O  R:::::::::::::RR  O:::::O     O:::::O  SS::::::SSSSS    
//     SSS::::::::SS    E:::::::::::::::E   M::::::M  M::::M::::M  M::::::M       A:::::A     A:::::A         F:::::::::::::::F   O:::::O     O:::::O  R::::RRRRRR:::::R O:::::O     O:::::O    SSS::::::::SS  
//        SSSSSS::::S   E::::::EEEEEEEEEE   M::::::M   M:::::::M   M::::::M      A:::::AAAAAAAAA:::::A        F::::::FFFFFFFFFF   O:::::O     O:::::O  R::::R     R:::::RO:::::O     O:::::O       SSSSSS::::S 
//             S:::::S  E:::::E             M::::::M    M:::::M    M::::::M     A:::::::::::::::::::::A       F:::::F             O:::::O     O:::::O  R::::R     R:::::RO:::::O     O:::::O            S:::::S
//             S:::::S  E:::::E       EEEEEEM::::::M     MMMMM     M::::::M    A:::::AAAAAAAAAAAAA:::::A      F:::::F             O::::::O   O::::::O  R::::R     R:::::RO::::::O   O::::::O            S:::::S
// SSSSSSS     S:::::SEE::::::EEEEEEEE:::::EM::::::M               M::::::M   A:::::A             A:::::A   FF:::::::FF           O:::::::OOO:::::::ORR:::::R     R:::::RO:::::::OOO:::::::OSSSSSSS     S:::::S
// S::::::SSSSSS:::::SE::::::::::::::::::::EM::::::M               M::::::M  A:::::A               A:::::A  F::::::::FF            OO:::::::::::::OO R::::::R     R:::::R OO:::::::::::::OO S::::::SSSSSS:::::S
// S:::::::::::::::SS E::::::::::::::::::::EM::::::M               M::::::M A:::::A                 A:::::A F::::::::FF              OO:::::::::OO   R::::::R     R:::::R   OO:::::::::OO   S:::::::::::::::SS 
//  SSSSSSSSSSSSSSS   EEEEEEEEEEEEEEEEEEEEEEMMMMMMMM               MMMMMMMMAAAAAAA                   AAAAAAAFFFFFFFFFFF                OOOOOOOOO     RRRRRRRR     RRRRRRR     OOOOOOOOO      SSSSSSSSSSSSSSS   

void bloquear_lista_segmentos(){
    pthread_mutex_lock(&m_lista_segmentos);
    log_trace(logger,"[SEM]: Bloqueo lista de segmentos");
}

void desbloquear_lista_segmentos(){
    pthread_mutex_unlock(&m_lista_segmentos);
    log_trace(logger,"[SEM]: Desloqueo lista de segmentos");
}

void liberar_segmento(segmento* segmento){
    bloquear_segmento(segmento);
    segmento->libre = true;
    desbloquear_segmento(segmento);
    ordenar_segmentos();
}

segmento* buscar_segmento_por_tid(int tid){
    int pid = tid / 10000;
    tabla_segmentos* tabla = (tabla_segmentos*) buscar_tabla(pid);
    if(tabla == NULL)
        return NULL;
    bool buscador(void* un_segmento){
        segmento* seg_tcb = (segmento*) un_segmento;
        t_TCB* tcb = memoria_principal + seg_tcb->base;
        return tcb->TID == tid;
    }
    return list_find(tabla->segmentos_tcb, buscador);
}

void bloquear_segmento(segmento* segmento){
    if(strcmp(ESQUEMA_MEMORIA, "SEGMENTACION") == 0)
        pthread_mutex_lock(&(segmento->mutex));
    log_trace(logger,"[SEM]: Bloqueo segmento base: %d",segmento->base);
}

void desbloquear_segmento(segmento* segmento){
    if(strcmp(ESQUEMA_MEMORIA, "SEGMENTACION") == 0)
        pthread_mutex_unlock(&(segmento->mutex));
    log_trace(logger,"[SEM]: Desbloqueo segmento base: %d",segmento->base);
}

void bloquear_segmento_por_tid(int tid){
    if(strcmp(ESQUEMA_MEMORIA, "SEGMENTACION") == 0){
        segmento* segmento_tcb = buscar_segmento_por_tid(tid);
        if(segmento_tcb != NULL){
            bloquear_segmento(segmento_tcb);
        }
    }
}

void desbloquear_segmento_por_tid(int tid){
    if(strcmp(ESQUEMA_MEMORIA, "SEGMENTACION") == 0){
        segmento* segmento_tcb = buscar_segmento_por_tid(tid);
        if(segmento_tcb != NULL){
            desbloquear_segmento(segmento_tcb);
        }
    }
}

/*
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
	
	printf("\n<----- TABLAS DE SEGMENTOS ---------------------\n");
    void print_tabla(char* pid, void* una_tabla){
        tabla_segmentos* tabla = (tabla_segmentos*) una_tabla;
		printf("Tabla pid: %s\n", pid);
        if(tabla->segmento_pcb != NULL){
            printf("\t Segmento PCB:\n");
            printf("\t\t base: %d\n",tabla->segmento_pcb->base);
            printf("\t\t tam: %d\n",tabla->segmento_pcb->tam);
        }
        if(tabla->segmento_tareas != NULL){
            printf("\t Segmento Tarea:\n");
            printf("\t\t base: %d\n",tabla->segmento_tareas->base);
            printf("\t\t tam: %d\n",tabla->segmento_tareas->tam);
        }
        int size = list_size(tabla->segmentos_tcb);
        for(int i = 0; i < size; i++){
            segmento* seg_tcb = list_get(tabla->segmentos_tcb,i);
            printf("\t Segmento TCB %d:\n",i+1);
            printf("\t\t base: %d\n",seg_tcb->base);
            printf("\t\t tam: %d\n",seg_tcb->tam);
        }
    }

    dictionary_iterator(tablas,print_tabla);

	printf("------------------>\n");
}
*/

void dump_segmentacion(){
    t_list* dump_segmentos = list_create();

    void cargar_tabla_al_dump(char* spid, void* una_tabla){
        int pid = atoi(spid);
        tabla_segmentos* tabla = (tabla_segmentos*) una_tabla;

        if(tabla->segmento_pcb != NULL){
            segmento_dump_wrapper* seg = malloc(sizeof(segmento_dump_wrapper));
            seg->segmento = tabla->segmento_pcb;
            seg->pid = pid;
            seg->num = 1;
            list_add(dump_segmentos,seg);
        }
        if(tabla->segmento_tareas != NULL){
            segmento_dump_wrapper* seg = malloc(sizeof(segmento_dump_wrapper));
            seg->segmento = tabla->segmento_tareas;
            seg->pid = pid;
            seg->num = 2;
            list_add(dump_segmentos,seg);
        }
        int size = list_size(tabla->segmentos_tcb);
        for(int i = 0; i < size; i++){
            segmento_dump_wrapper* seg = malloc(sizeof(segmento_dump_wrapper));
            segmento* seg_tcb = list_get(tabla->segmentos_tcb,i);
            seg->segmento = seg_tcb;
            seg->pid = pid;
            seg->num = i + 3;
            list_add(dump_segmentos,seg);
        }

    }
    bloquear_lista_tablas();
    bloquear_lista_segmentos();

    dictionary_iterator(tablas,cargar_tabla_al_dump);

    bool ordenador(void* un_segmento, void* otro_segmento){
        segmento_dump_wrapper* seg1 = (segmento_dump_wrapper*) un_segmento;
        segmento_dump_wrapper* seg2 = (segmento_dump_wrapper*) otro_segmento;   
        return seg1->segmento->base < seg2->segmento->base;
    }
    list_sort(dump_segmentos,ordenador);

    char* path = string_from_format("./dump/Dump_%d.dmp", (int) time(NULL));
    FILE* file = fopen(path,"w");
    free(path);

    void impresor_dump(void* un_segmento){
        segmento_dump_wrapper* seg = (segmento_dump_wrapper*) un_segmento;
        char* dump_row = string_from_format("Proceso: %d\tSegmento: %d\tInicio: 0x%.4x\tTam: %db\n", seg->pid, seg->num, seg->segmento->base, seg->segmento->tam);
        //char* dump_row = string_from_format("Proceso: %d\tSegmento: %d\tInicio: %d\tTam: %db\n", seg->pid, seg->num, seg->segmento->base, seg->segmento->tam);
        txt_write_in_file(file, dump_row);
        free(dump_row);
    }

    char* temporal = temporal_get_string_time("%d/%m/%y %H:%M:%S");
	char* string_a_printear = string_from_format("Dump: %s\n", temporal);

    txt_write_in_file(file, string_a_printear);

    free(temporal);
	free(string_a_printear);
    
    list_iterate(dump_segmentos, impresor_dump);
    txt_close_file(file);

    void destructor(void* un_segmento){
        segmento_dump_wrapper* seg = (segmento_dump_wrapper*) un_segmento;
        free(seg);
    }
    list_destroy_and_destroy_elements(dump_segmentos,destructor);

    desbloquear_lista_segmentos();
    desbloquear_lista_tablas();
}
