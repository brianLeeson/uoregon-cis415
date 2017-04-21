OJECTS = arraylist.o bqueue.o hashmap.o iterator.o linkedlist.o
CFLAGS = -std=c99 -W -Wall -g
CC = gcc

lab2: $(OBJECTS)
	$(CC) $(OBJECTS) -o tldmonitor


all:

static_tests:

shared_tests:

libgencontainer.a: $(OBJECTS) 
	ar rcs $@ $^

libgencontainer.so: 

static_altest:

shared_altest:

static_bqtest:

shared_bqtest:

static_hmtest.o:

shared_hmtest.o:

static_lltest:

shared_lltest:


arraylist.o: arraylist.c arraylist.h
bqueue.o: bqueue.c bqueue.h
hashmap.o: hashmap.c hashmap.h
iterator.o: iterator.c iterator.h
linkedlist.o: linkedlist.c linkedlist.h


clean:
	$(RM) count *.o *~
