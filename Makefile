CC=gcc
SRCS=src/popcorn.c src/config.c src/ui.c src/edit.c src/omdb.c
HEADS=src/popcorn.h src/config.h src/ui.h src/edit.h
CFLAGS=-Wall
LIBS=-lncursesw -lcurl -lcjson
OBJDIR=objs

popcorn: $(SRCS) $(HEADS) Makefile
	$(CC) -ggdb -c src/ui.c -o objs/ui.o $(CFLAGS) $(LIBS)
	$(CC) -ggdb -c src/omdb.c -o objs/omdb.o $(CFLAGS) $(LIBS)
	$(CC) -ggdb -c src/config.c -o objs/config.o $(CFLAGS) $(LIBS)
	$(CC) -ggdb -c src/edit.c -o objs/edit.o $(CFLAGS) $(LIBS)
	$(CC) -ggdb -c src/popcorn.c -o objs/popcorn.o $(CFLAGS) $(LIBS)
	$(CC) -ggdb -o popcorn objs/*.o $(CFLAGS) $(LIBS)

edit: src/edit.c src/edit.h
	$(CC) -ggdb src/edit.c

config: src/config.c
	$(CC) -o popcorn src/config.c $(CFLAGS)

debug:
	$(CC) -ggdb -o popcorn src/config.c $(CFLAGS)
