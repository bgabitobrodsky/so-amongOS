TARGET = mi_ram_hq
LIBS = -lcommons -lpthread -lcomms -lcunit -lnivel-gui -lncurses
CC = gcc
CFLAGS = -g -Wall

.PHONY: default all clean

default: $(TARGET)

all: default

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h, ../Modulos/*.h, )

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
