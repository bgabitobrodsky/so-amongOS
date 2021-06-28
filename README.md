# No matar a rey de fuego

### Registro de cambios MONGO
24/06/2021
README: A partir de ahora lo implemento, prometo ir anotando los cambios para no olvidarme, como me pudo haber pasado hoy.
Hilos: join por detach
Inicializar archvos: Se inicializan los datos en 0 (tamaño, cant_bloques, etc)
Actualizaciones MD5: No las hacía, ahora si
obtener_bitacora: completa

### Registro de cambios
27/06/2021
+ Desbuggeadas operaciones de entrada/salida.

26/06/2021
+ Modelado el protocolo para la llegada de un sabotaje.

25/06/2021
+ Ahora los tripulantes actualizan su estado a RAM solo si lo cambiaron.
+ Ahora cada tripulante tiene su propio socket.

24/06/2021
+ Implementado DUMP de memoria.
+ Corregidos los puertos en la config.

23/06/2021
+ Planificacion mediante FIFO y ROUND ROBIN.
+ (ahora es discordiador quien actualiza a RAM el estado de quienes pasan a EXEC).

21/06/2021
+ Los tripulantes informan a ram sus movimientos y cambios de estado.
+ Grandes avances en discordiador.
+ Se reciben tareas desde RAM.
+ Los tripulantes son capaces de realizar sus tareas.

20/06/2021
+ Iniciado planteamiento de tripulantes.
+ Planteado movimiento en el tablero.

19/06/2021
+ LISTAR_TRIPULANTES completamente funcional.
+ Iniciado trabajo en conjunto con RAM.

18/06/2021
+ Revividas las estructuras t_tripulante y t_patota.
+ Solucionado el problema de no poder eliminar al tripulante tambien del modulo discordiador.
+ LISTAR_TRIPULANTES aceptablemente funcional.
+ Arreglada enlistar_algun_tripulante y eliminada enlistar_este_tripulante.
+ Pedido de tareas resuelto desde la parte de discordiador.

17/06/2021
+ Agregadas funcionalidades de segmentación y compactación a Mi_Ram_HQ
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

13/06/2021
+ Agregada funcion para eliminar punteros dobles en DIscordiador, y arregladas algunas perdidas de bytes, cortesia de Valgrind.

11/06/2021
+ Se pudo enviar y recibir TCBs de RAM a DISCORDIADOR bidireccionalmente (NOTA: TID no se actualiza en RAM).
+ Arregladas funciones de serializar y desserializar TCB.

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
