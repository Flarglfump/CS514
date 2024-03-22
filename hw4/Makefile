SRCDIR = src
CC = gcc

all:
	make break; make unbreak;
break:
	$(CC) -I. -I$(SRCDIR) -g $(SRCDIR)/break.c -o ./break
unbreak:
	$(CC) -I. -I$(SRCDIR) -g $(SRCDIR)/unbreak.c -o ./unbreak
clean: 
	rm -f $(SRCDIR)/*.o ./unbreak ./break