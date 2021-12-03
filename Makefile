CFILES=slashn.c
OFILES=$(CFILES:.c=.o)
CC=gcc -Wall

slashn:	$(OFILES)
	$(CC) -o slashn $(OFILES) -pthread

.c.o:
	$(CC) -c $<

clean::
	/bin/rm -f slashn $(OFILES)
