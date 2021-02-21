
CC=gcc
LD=gcc

CFLAGS = -Wall -c -g -ansi -pedantic
LDFLAGS =

CFILES = rumor.c person.c
HFILES = rumor.h person.h
OFILES = rumor.o person.o


all: $(OFILES) makefile

rumor.o: rumor.c defines.h
	$(CC) $(CFLAGS) rumor.c

person.o: person.c defines.h
	$(CC) $(CFLAGS) person.c


$(OFILES): $(HFILES)


clean:
	rm -f $(OFILES) core *~
