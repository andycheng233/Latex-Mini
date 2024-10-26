CC = gcc
CFLAGS = -g3 -Wall -pedantic

proj1: latexMini.o
	$(CC) $(CFLAGS) -o $@ $^
	
latexMini.o: latexMini.c latexMini.h