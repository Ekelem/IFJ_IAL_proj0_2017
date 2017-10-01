# IFJ/IAL project 2017

# author/s: Erik Kelemen, Attila Lakatos


TARGET = IFJ17

CC=gcc

CFLAGS=-O2 -std=c11 -Wall -pedantic -lm

MODULES=error string scanner symtable tokens

OBJS = $(addprefix obj/, $(addsuffix .o,$(MODULES)))



all: $(TARGET)
.PHONY: clean, dokumentace

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) src/main.c -o $@

obj/%.o : src/%.c src/%.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f ./obj/*.o $(TARGET)

dokumentace: doc/dokumentace.tex
	pdflatex doc/dokumentace.tex
