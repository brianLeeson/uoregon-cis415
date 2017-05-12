CC=gcc
CFLAGS=-W -Wall

bbcat: bbcat.o BoundedBuffer.o
	$(CC) -o bbcat bbcat.o BoundedBuffer.o -lpthread

bbcat.o: bbcat.c BoundedBuffer.h
BoundedBuffer.o: BoundedBuffer.c

clean:
	rm -f *.o bbcat
