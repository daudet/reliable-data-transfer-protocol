CC = gcc

CFLAGS = -o
OFLAGS = -Wall -g -c

OFILES = rws.o rwsc.o
PROJECT = rws rwsc

all: $(PROJECT)

rws: rws.o
	$(CC) $(CFLAGS) rws rws.o

rwsc: rwsc.o
	$(CC) $(CFLAGS) rwsc rwsc.o

rws.o: rws.c rdp.h
	$(CC) $(OFLAGS) rws.c

rwsc.o:	rwsc.c rdp.h
	$(CC) $(OFLAGS) rwsc.c

clean:
	rm -f $(OFILES) $(PROJECT)
