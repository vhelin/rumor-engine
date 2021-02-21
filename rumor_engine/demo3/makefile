
CC=gcc
LD=gcc

CFLAGS = -Wall -c -g -ansi -pedantic
LDFLAGS = -lncurses

CFILES = main.c level.c item.c ai.c map.c
HFILES = main.h level.h item.h ai.h map.h
OFILES = main.o level.o item.o ai.o map.o
ORUMOR = ../rumor/rumor.o ../rumor/person.o
EXECUT = demo3


all: $(OFILES) makefile
	$(LD) $(LDFLAGS) $(OFILES) $(ORUMOR) -o $(EXECUT) -lm

main.o: main.c defines.h
	$(CC) $(CFLAGS) main.c

level.o: level.c defines.h
	$(CC) $(CFLAGS) level.c

item.o: item.c defines.h
	$(CC) $(CFLAGS) item.c

ai.o: ai.c defines.h
	$(CC) $(CFLAGS) ai.c

map.o: map.c defines.h
	$(CC) $(CFLAGS) map.c


$(OFILES): $(HFILES)


clean:
	rm -f $(OFILES) core *~ $(EXECUT) debug

nice:
	rm *~
