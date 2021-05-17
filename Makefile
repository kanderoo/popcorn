CC=gcc
OBJDIR=objs
SRCDIR=src
SRCS=$(wildcard $(SRCDIR)/*.c)
OBJS=$(patsubst src/%.c, $(OBJDIR)/%.o, $(SRCS))
HEADS=$(SRCDIR)/*.h
CFLAGS=-Wall
LIBS=-lncursesw -lcurl -lcjson

popcorn: $(OBJS)
	$(CC) $(CFLAGS) $(LIBS) -ggdb -o popcorn $(OBJS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(SRCDIR)/%.h
	$(CC) $(CFALGS) $(LIBS) -ggdb -c -o $@ $<
