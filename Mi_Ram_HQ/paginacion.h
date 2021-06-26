#ifndef PAGINACION_H_
#define PAGINACION_H_

#include "mi_ram_hq.h"


extern t_list* indices;


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
    t_list* paginas;
} tabla_paginas;


tabla_paginas* crear_tabla_paginas(uint32_t pid);
int completar_pagina(pagina* pagina, int tamano, tabla_paginas* tabla);
void agregar_paginas_segun_tamano(tabla_paginas* tabla, int tamano);
void agregar_pagina( tabla_paginas* tabla, int tamano);
pagina* crear_pagina(marco* marco, int ocupa);
int cantidad_marcos_completos(int tam);
int ocupa_marco_incompleto(int tam);
tabla_paginas* crear_tabla_paginas(uint32_t pid);
marco* crear_marco(int base, bool libre);
void liberar_marco(int num_marco);
marco* buscar_marco_libre();
marco* asignar_marco();



#endif /* PAGINACION_H_ */



