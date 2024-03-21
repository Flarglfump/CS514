SRCDIR = src
OBJS = ish.o parse.o
CC = gcc
OUTFILE = ish

all: ${OBJS}
	${CC} -g ${OBJS} -o ${OUTFILE}
	rm -f *.o
ish.o:
	${CC} -c -g ${SRCDIR}/ish.c
parse.o:
	${CC} -c -g ${SRCDIR}/parse.c
clean:
	rm -f ${OUTFILE} *.o