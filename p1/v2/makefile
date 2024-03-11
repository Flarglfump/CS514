OBJS = ish.o env.o lex.o parse.o main.o
CC = gcc
OUTNAME = ish

all: ${OBJS}
	gcc -g ${OBJS} -o ${OUTNAME}
	rm -f ${OBJS}
ish.o:
	gcc -c ish.c -o ish.o
env.o:
	gcc -c env.c -o env.o
lex.o:
	gcc -c lex.c -o lex.o
parse.o:
	gcc -c parse.c -o parse.o
main.o:
	gcc -c main.c -o main.o
clean:
	rm -f ${OBJS} ${OUTNAME}