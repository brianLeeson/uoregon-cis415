OBJECTS = p1fxns.o uspsv1.o
CFLAGS = -std=c99 -W -Wall -g
CC = gcc
PROG_NAME = usps
ARGS = --quantum=1234 workload_file.txt

test: uspsv1 cpu io
	./$(PROG_NAME) $(ARGS)

sense: uspsv1 cpu io
	gdb ./$(PROG_NAME) 

waterproof: uspsv1 cpu io
	(valgrind --leak-check=full -v ./$(PROG_NAME)) || true

uspsv1: $(OBJECTS) 
	$(CC) $(OBJECTS) -o $(PROG_NAME)

uspsv2: $(OBJECTS)
	$(CC) $(OBJECTS) -o $(PROG_NAME)
	
uspsv3: $(OBJECTS)
	$(CC) $(OBJECTS) -o $(PROG_NAME)
	
uspsv4: $(OBJECTS)
	$(CC) $(OBJECTS) -o $(PROG_NAME)

cpu: cpubound.o
	$(CC) cpubound.o -o cpubound
	
io: iobound.o
	$(CC) iobound.o -o iobound

p1fxns.o: p1fxns.c p1fxns.h

uspsv1.o: uspsv1.c

cpubound.o: cpubound.c

iobound.o: iobound.c 

clean:
	rm $(PROG_NAME)