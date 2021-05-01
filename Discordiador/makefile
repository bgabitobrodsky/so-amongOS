
TARGET = discordiador # Definimos la tarea que va a ejecutar make por defecto Y el nombre del binario final
LIBS = -lreadline -lcommons -lpthread # Incluimos las bibliotecas que necesitamos, las commons y readline
CC = gcc # Vamos a usar gcc como compilador, porque nos encanta!
CFLAGS = -g -Wall # Compilamos con los flags -g (con simbolos de debug) y -Wall (que nos muestre todos los warnings)

.PHONY: default all clean

default: $(TARGET) # la regla por defecto (tp0)
all: default # Si ejecutan all, que ejecute la default


OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS) # Las variables que le pasamos a la tarea, le decimos que use todos los .c y todos los .h
	$(CC) $(CFLAGS) -c $< -o $@ # Creamos los objects sin linkear

.PRECIOUS: $(TARGET) $(OBJECTS) # Nos permite mantener los archivos temporales terminados si matan a make con Cntl-C

$(TARGET): $(OBJECTS) # Le pasamos a la tarea los objects ya compilados
	$(CC) $(OBJECTS) $(CFLAGS) $(LIBS) -o $@ # Usamos el compilador para linkear los objects a las bibliotecas y tener nuestro binario

clean: # Finalmente creamos una tarea para eliminar todos los .o y el binario
	-rm -f *.o
	-rm -f $(TARGET)