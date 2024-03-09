OBJS = lexer.c parser.c env.c ish.c
CC = gcc
OUTPUT = ish

all: ${OBJS}
	${CC} ${OBJS} -o ${OUTPUT}
run: all
	./${OUTPUT}
debug: ${OBJS}
	${CC} ${OBJS} -o ${OUTPUT} -g
clean:
	rm -f ish