CC=gcc
CFLAGS=-Wall -lncurses

popcorn: src/display.c
	$(CC) -o popcorn src/display.c $(CFLAGS)

config: src/config.c
	$(CC) -o popcorn src/config.c $(CFLAGS)

debug:
	$(CC) -ggdb -o popcorn src/config.c $(CFLAGS)
