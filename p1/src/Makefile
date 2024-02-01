OBJS = ish.c lexer.c command.c
CC = gcc
OUTPUT = ish

all: ${OBJS}
	${CC} ${OBJS} -o ${OUTPUT} -Wall -g
clean:
	rm -f ish