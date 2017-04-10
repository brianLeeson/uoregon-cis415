OBJECTS = date.o tldlist.o tldmonitor.o
CFLAGS = -std=c99 -W -Wall -g
CC = gcc
PROG_NAME = project0
ARGS = 01/08/1999 31/07/2009 small.txt

project0: $(OBJECTS)
	$(CC) $(OBJECTS) -o project0

date.o: date.c date.h
tldlist.o: tldlist.c tldlist.h
tldmonitor.o: tldmonitor.c

run: ${PROG_NAME}
	./${PROG_NAME} ${ARGS}