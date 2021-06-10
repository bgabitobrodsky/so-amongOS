# No matar a rey de fuego

### Registro de cambios

10/06/2021
+ Cambiadas varias listas a colas.
+ Añadido escuchar discordiador y enlistar_tripulante().
+ Eliminado pthread_join de leer consola.
+ Creada variable "sistema_activo" y agregado el comando "APAGAR_SISTEMA"; el cual apaga el sistema sin depender de un hilo.
+ Arreglado puerto del mongo.
+ Añadida recepción de TCBs en MI_RAM.

06/06/2021
+ Nos mudamos a Listas.
+ Migradas todas las funcionalidades de la rama proceso-discordiador a main.
+ Organización de dependencias.

29/05/2021
+ Arregladas pequeñas cosas en get_pid y las funciones relacionadas a crear estructuras de patotas y tripulantes.
+ Cambiados archivos de config, ahora todos tienen la IP localhost para poder usar los define y no hardcodear
+ Unificadas todas las IP en LOCALHOST.
+ Eliminados practicamente todos los warnings de los modulos.
+ uint_32t* cambiados a uint_32t, con sus respectivos casteos. 
+ Descomentadas funciones y agregadas firmas para que compile.
