OBJECTS = p1fxns.o uspsv3.o
CFLAGS = -W -Wall -g
CC = gcc
PROG_NAME = usps
ARGS = --quantum=800 workload_file.txt

test: current
	./$(PROG_NAME) $(ARGS)

sense: current
	gdb ./$(PROG_NAME) 

waterproof: current
	(valgrind --track-origins=yes -v ./$(PROG_NAME) $(ARGS)) || true

current: uspsv3 cpu io

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

uspsv2.o: uspsv2.c

uspsv3.o: uspsv3.c

cpubound.o: cpubound.c

iobound.o: iobound.c 

clean:
	rm $(PROG_NAME)
	rm *.o
	rm cpubound 
	rm iobound 