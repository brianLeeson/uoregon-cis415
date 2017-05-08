OBJECTS1 = p1fxns.o uspsv1.o
OBJECTS2 = p1fxns.o uspsv2.o
OBJECTS3 = p1fxns.o uspsv3.o
OBJECTS4 = p1fxns.o uspsv4.o

CFLAGS = -W -Wall -g
CC = gcc
PROG_NAME = uspsv
ARGS = --quantum=800 workload_file.txt

all: uspsv1 uspsv2 uspsv3 uspsv4 cpu io
	
test: all
	./$(PROG_NAME)3 $(ARGS)

sense: all
	gdb ./$(PROG_NAME)3

waterproof: all
	(valgrind --track-origins=yes --leak-check=full -v ./$(PROG_NAME)3 $(ARGS)) || true

uspsv1: $(OBJECTS1) 
	$(CC) $(OBJECTS1) -o uspsv1

uspsv2: $(OBJECTS2)
	$(CC) $(OBJECTS2) -o uspsv2
	
uspsv3: $(OBJECTS3)
	$(CC) $(OBJECTS3) -o uspsv3
	
uspsv4: $(OBJECTS4)
	$(CC) $(OBJECTS4) -o uspsv4

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