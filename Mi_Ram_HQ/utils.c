#include "utils.h"


int tamanio_tarea(t_tarea* tarea){
    int tam = sizeof(uint32_t) * 5;
    tam += tarea->largo_nombre * sizeof(char);
    return tam;
}
