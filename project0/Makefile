OBJECTS = date.o tldlist.o tldmonitor.o
CFLAGS = -std=c99 -W -Wall -g
CC = gcc
PROG_NAME = tldmonitor
SMALL = 01/01/2000 01/09/2013 small.txt
LARGE = 01/01/2000 01/09/2013 large.txt

tldmonitor: $(OBJECTS)
	$(CC) $(OBJECTS) -o tldmonitor

date.o: date.c date.h
tldlist.o: tldlist.c tldlist.h
tldmonitor.o: tldmonitor.c

small: ${PROG_NAME}
	./${PROG_NAME} ${SMALL}
	
large: ${PROG_NAME}
	./${PROG_NAME} ${LARGE}