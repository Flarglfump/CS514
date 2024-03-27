CC=gcc
CFLAGS=-g -Wall
SRC=src
OBJ=obj
SRCS=$(wildcard $(SRC)/*.c)
OBJS=$(patsubstr $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))

BINDIR=bin
BIND=$(BINDIR)/main
SUBMITNAME=p2.tgz

RM=rm

all: $(BIN)


release: CFLAGS=-Wall -O2 -DNDEBUG
release: clean
release: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -f $(BINDIR)/* $(OBJ)/* $(SUBMITNAME)
submit:
	$(RM) -f $(SUBMITNAME)
	tar -czf $(SUBMITNAME) * $(BINDIR) $(OBJ) $(SRC) Makefile