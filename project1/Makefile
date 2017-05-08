OBJECTS1 = p1fxns.o uspsv1.o
OBJECTS2 = p1fxns.o uspsv2.o
OBJECTS3 = p1fxns.o uspsv3.o
OBJECTS4 = p1fxns.o uspsv4.o

CFLAGS = -W -Wall -g
CC = gcc
PROG_NAME = uspsv
ARGS = workload_file.txt

all: uspsv1 uspsv2 uspsv3 uspsv4 cpu io
	
#tests only one version at a time.
test: all
	./$(PROG_NAME)4 $(ARGS)

#tests only one version at a time.
waterproof: all
	(valgrind --track-origins=yes --leak-check=full  --show-leak-kinds=all -v ./$(PROG_NAME)4 $(ARGS)) || true

uspsv1: $(OBJECTS1) 
	$(CC) $(OBJECTS1) -o $(PROG_NAME)1

uspsv2: $(OBJECTS2)
	$(CC) $(OBJECTS2) -o $(PROG_NAME)2
	
uspsv3: $(OBJECTS3)
	$(CC) $(OBJECTS3) -o $(PROG_NAME)3
	
uspsv4: $(OBJECTS4)
	$(CC) $(OBJECTS4) -o $(PROG_NAME)4

cpu: cpubound.o
	$(CC) cpubound.o -o cpubound
	
io: iobound.o
	$(CC) iobound.o -o iobound

p1fxns.o: p1fxns.c p1fxns.h

uspsv1.o: uspsv1.c

uspsv2.o: uspsv2.c

uspsv3.o: uspsv3.c

uspsv4.o: uspsv4.c

cpubound.o: cpubound.c

iobound.o: iobound.c 

clean:
	rm uspsv1
	rm uspsv2
	rm uspsv3
	rm uspsv4
	rm *.o
	rm cpubound 
	rm iobound 