
CC=gcc
LD=gcc

CFLAGS = -Wall -c -g -ansi -pedantic
LDFLAGS =

CFILES = main.c
HFILES = main.h
OFILES = main.o
ORUMOR = ../rumor/rumor.o ../rumor/person.o
EXECUT = demo1


all: $(OFILES) makefile
	$(LD) $(LDFLAGS) $(OFILES) $(ORUMOR) -o $(EXECUT) -lm

main.o: main.c defines.h
	$(CC) $(CFLAGS) main.c


$(OFILES): $(HFILES)


clean:
	rm -f $(OFILES) core *~ $(EXECUT) debug
