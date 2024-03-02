OBJS = timingStuff.o helperFuncs.o
CC = gcc
OUTNAME = timingStuff


all: ${OBJS}
	${CC} -g ${OBJS} -o ${OUTNAME}
	rm -rf ${OBJS}
timingStuff.o:
	${CC} -c timingStuff.c -o timingStuff.o
helperFuncs.o:
	${CC} -c helperFuncs.c -o helperFuncs.o
clean:
	rm -rf ${OBJS} ${OUTNAME}