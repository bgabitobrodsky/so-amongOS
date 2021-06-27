#ifndef PAGINACION_H_
#define PAGINACION_H_

#include "mi_ram_hq.h"


typedef struct marco {
    int base;
    bool libre;
    //uint64_t ultimo_uso; // para LRU
} marco;
t_list* marcos;


typedef struct pagina {
    marco* puntero_marco;
    int tamano_ocupado;
} pagina;


typedef struct tabla_paginas {
    int dl_pcb;
    int dl_tareas;
    t_dictionary* dl_tcbs;
    t_list* paginas;
} tabla_paginas;


void* rescatar_de_paginas(tabla_paginas* tabla, int dl, int tam);
int rescatar_de_marco(marco* marco, void* data, int offset, int tam);
tabla_paginas* crear_tabla_paginas(uint32_t pid);
int completar_pagina(pagina* pagina, void* data, int tam);
int agregar_paginas_segun_tamano(tabla_paginas* tabla, void* data, int tam);
int agregar_pagina(tabla_paginas* tabla, void* data, int tam);
pagina* crear_pagina(marco* marco);
int cantidad_marcos_completos(int tam);
int ocupa_marco_incompleto(int tam);
tabla_paginas* crear_tabla_paginas(uint32_t pid);
marco* crear_marco(int base, bool libre);
void liberar_marco(int num_marco);
marco* buscar_marco_libre();
marco* asignar_marco();
pagina* pagina_incompleta(tabla_paginas* tabla);
int matar_paginas_tcb(tabla_paginas* tabla, int tid);


#endif /* PAGINACION_H_ */



