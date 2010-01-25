CC=gcc

CFLAGS= -o
OFLAGS = -Wall -g -c

OFILES = sws.o
PROJECT = sws

all: $(OFILES)
		$(CC) $(CFLAGS) $(PROJECT) $(OFILES) -lpthread

sws.o: sws.c
		$(CC) $(OFLAGS) sws.c

clean:
		rm -f $(OFILES) $(PROJECT)
