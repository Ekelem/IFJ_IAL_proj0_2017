# IFJ/IAL project 2017

# author/s: Erik Kelemen, Attila Lakatos


TARGET = IFJ17

CC=gcc

CFLAGS=-O2 -std=c11 -Wall -pedantic -lm

MODULES=error string scanner symtable tokens parser garbage_collector stack common expressions

OBJS = $(addprefix obj/, $(addsuffix .o,$(MODULES)))



all: $(TARGET)
.PHONY: clean, dokumentace

$(TARGET) : $(OBJS) src_v2/main.c
	$(CC) $(CFLAGS) $(OBJS) src_v2/main.c -o $@

obj/%.o : src_v2/%.c src_v2/%.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f ./obj/*.o $(TARGET)

dokumentace: doc/dokumentace.tex
	pdflatex doc/dokumentace.tex
