
CC=gcc
LD=gcc

CFLAGS = -Wall -c -g -ansi -pedantic
LFLAGS =
LLIBS =  -lm

CFILES = demo.c action_memory.c unit_test.c
HFILES = demo.h action_memory.h unit_test.h
OFILES = demo.o action_memory.o unit_test.o


all: unit_test demo

.c.o: defines.h
	$(CC) $(CFLAGS) $<

unit_test: action_memory.o unit_test.o makefile
	$(LD) $(LFLAGS) action_memory.o unit_test.o -o unit_test $(LLIBS)

demo: action_memory.o demo.o makefile
	$(LD) $(LFLAGS) action_memory.o demo.o -o demo $(LLIBS)


$(OFILES): $(HFILES)


clean:
	rm -f $(OFILES) core *~ unit_test demo debug
