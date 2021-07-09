#ifndef PAGINACION_H_
#define PAGINACION_H_

#include "mi_ram_hq.h"


typedef struct marco {
    int base;
    bool libre;
    int pid;
    int num_pagina;
} marco;
t_list* marcos;


typedef struct pagina {
    bool en_memoria;
    uint64_t ultimo_uso; // para LRU
    int disk_index;
    bool modificada;
    int tamano_ocupado;
    marco* puntero_marco;
    //bool usado // para clock
    //bool bloqueada?
} pagina;
t_list* paginas;


typedef struct tabla_paginas {
    pthread_mutex_t mutex;
    int dl_pcb;
    int dl_tareas;
    t_dictionary* dl_tcbs;
    t_list* paginas;
} tabla_paginas;

int intento_asignar_marco;


void* rescatar_de_paginas(tabla_paginas* tabla, int dl, int tam, int pid);
int rescatar_de_marco(marco* marco, void* data, int offset, int tam);
tabla_paginas* crear_tabla_paginas(uint32_t pid);
int completar_pagina(pagina* pagina, void* data, int tam);
int agregar_paginas_segun_tamano(tabla_paginas* tabla, void* data, int tam, int pid);
int agregar_pagina(tabla_paginas* tabla, void* data, int tam, int pid);
pagina* crear_pagina(marco* marco);
tabla_paginas* crear_tabla_paginas(uint32_t pid);
marco* crear_marco(int base, bool libre);
void liberar_marco(int num_marco);
marco* buscar_marco_libre();
marco* asignar_marco();
pagina* pagina_incompleta(tabla_paginas* tabla);
int matar_paginas_tcb(tabla_paginas* tabla, int tid);
int sobreescribir_paginas(tabla_paginas* tabla, void* data, int dl, int tam, int pid);
void liberar_lista_tcbs_paginacion(t_list* lista);
int escribir_en_marco(marco* marco, void* data, int offset, int tam);
void matar_tabla_paginas(int pid);
pagina* get_lru();
int swap_in(pagina* pagina);
void swap_out(pagina* pagina);
int algoritmo_de_reemplazo();
void page_fault(pagina* pag, int pid, int num);
uint64_t unix_epoch();

#endif /* PAGINACION_H_ */



