# No matar a rey de fuego

### Registro de cambios
17/06/2021
+ Agregadas varias funciones de serializacion.
+ EXPULSAR_TRIPULANTE esta terminado en el modulo discordiador.
+ INICIAR_PATOTA es completamente funcional.
+ Agregadas funciones para leer archivos.
+ Agregadas funcionalidades del I-Mongo-Store.
+ Agregadas funciones de serialización y estructuras de archivo de tareas.
+ Agregada recepción de tareas en RAM.

14/06/2021
+ Nos mudamos a monitores!
+ DIseñados monitores para colas y listas.
+ Modificada eliminar_tcb_de_lista para hacerla polimorfica.

13/06/2021
+ Agregada funcion para eliminar punteros dobles en DIscordiador, y arregladas algunas perdidas de bytes, cortesia de Valgrind.

11/06/2021
+ Se pudo enviar y recibir TCBs de RAM a DISCORDIADOR bidireccionalmente.
+ Arregladas funciones de serializar y desserializar TCB.

11/06/2021
+ Se pudo enviar y recibir TCBs de RAM a DISCORDIADOR bidireccionalmente.
+ Arregladas funciones de serializar y desserializar TCB.
+ Creadas funciones para crear TCB y PCB en ram (a comentar).

10/06/2021
+ FInalizados los siguientes TODO:  
    - GENERALIZAR LOS ARGS DE TODOS LOS  ESCUCHAR.  
    - JUNTAR LA INICIALIZACION DE LISTAS Y COLAS EN UNA FUNC.  
+ Pseudocodigo de subfunciones de LISTAR_TRIPULANTES.

09/06/2021
+ Cambiadas varias listas a colas.
+ Añadido escuchar discordiador y enlistar_tripulante().
+ Eliminado pthread_join de leer consola.
+ Creada variable "sistema_activo" y agregado el comando "APAGAR_SISTEMA"; el cual apaga el sistema sin depender de un hilo.
+ Arreglado puerto del mongo.
+ Añadida recepción de TCBs en MI_RAM.
+ Prototipo de listar_tripulantes().

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
