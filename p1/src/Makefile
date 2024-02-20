OBJS = lexer.c parser.c env.c ish.c
CC = gcc
OUTPUT = ish

all: ${OBJS}
	${CC} ${OBJS} -o ${OUTPUT}
run: all
	./${OUTPUT}
clean:
	rm -f ish