#include "utils.h"


size_t tamanio_tarea(t_tarea* tarea){
    size_t tam = sizeof(uint32_t) * 5;
    tam += tarea->largo_nombre * sizeof(char);
    return tam;
}
