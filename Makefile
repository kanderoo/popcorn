CC=gcc
SRCS=src/popcorn.c src/config.c src/ui.c src/edit.c
HEADS=src/popcorn.h src/config.h src/ui.h src/edit.h
CFLAGS=-Wall -lncurses
OBJDIR=objs

popcorn: $(SRCS) $(HEADS)
	$(CC) -ggdb -c src/ui.c -o objs/ui.o $(CFLAGS)
	$(CC) -ggdb -c src/config.c -o objs/config.o $(CFLAGS)
	$(CC) -ggdb -c src/edit.c -o objs/edit.o $(CFLAGS)
	$(CC) -ggdb -c src/popcorn.c -o objs/popcorn.o $(CFLAGS)
	$(CC) -ggdb -o popcorn objs/*.o $(CFLAGS)

edit: src/edit.c src/edit.h
	$(CC) -ggdb src/edit.c

config: src/config.c
	$(CC) -o popcorn src/config.c $(CFLAGS)

debug:
	$(CC) -ggdb -o popcorn src/config.c $(CFLAGS)
