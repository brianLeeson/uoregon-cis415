OBJECTS = p1fxns.o uspsv1.o
CFLAGS = -std=c99 -W -Wall -g
CC = gcc
PROG_NAME = usps
ARGS = --quantum=1234 workload_file.txt

test: uspsv1
	./$(PROG_NAME) $(ARGS)

uspsv1: $(OBJECTS)
	$(CC) $(OBJECTS) -o $(PROG_NAME)

uspsv2: $(OBJECTS)
	$(CC) $(OBJECTS) -o $(PROG_NAME)
	
uspsv3: $(OBJECTS)
	$(CC) $(OBJECTS) -o $(PROG_NAME)
	
uspsv4: $(OBJECTS)
	$(CC) $(OBJECTS) -o $(PROG_NAME)

p1fxns.o: p1fxns.c p1fxns.h

uspsv1.o: uspsv1.c

clean:
	rm $(PROG_NAME)