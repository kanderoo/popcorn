CC=gcc
CFLAGS=-Wall -lncurses

popcorn: src/display.c
	$(CC) -o popcorn src/display.c $(CFLAGS)
