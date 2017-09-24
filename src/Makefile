# IFJ/IAL project 2017

# author: Erik Kelemen


TARGET = IFJ17

CC=gcc

CFLAGS=-O2 -std=c11 -Wall -pedantic -lm

OBJS = main.o error.o



all: $(TARGET)

$(TARGET) : scanner.c scanner.h string.c string.h main.c
	$(CC) $(CFLAGS) scanner.c string.c main.c -o $@

clean:
	rm -f *.o $(TARGET)