CC=gcc
CFLAGS=-Wall -lncurses

popcorn: src/popcorn.c src/config.c src/display.c
	$(CC) -ggdb -c src/display.c -o objs/display.o $(CFLAGS)
	$(CC) -ggdb -c src/config.c -o objs/config.o $(CFLAGS)
	$(CC) -ggdb -c src/popcorn.c -o objs/popcorn.o $(CFLAGS)
	$(CC) -ggdb -o popcorn objs/* $(CFLAGS)

config: src/config.c
	$(CC) -o popcorn src/config.c $(CFLAGS)

debug:
	$(CC) -ggdb -o popcorn src/config.c $(CFLAGS)
