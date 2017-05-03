#include "p1fxns.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

//USPS_QUANTUM_MSEC

int getQuantum(int argc, char *argv[]){
	/* function takes argc and argv
	 * gets quantum from environment or command line with command line priority
	 * returns quantum if exists or specified, -1 otherwise.
	 */
	char *p = NULL;
	int quantum = -1;
	char *argName = "--quantum=";
	int argLen = p1strlen(argName);

	if ((p = getenv("USPS_QUANTUM_MSEC")) != NULL){
		quantum = p1atoi(p);
	}
	if (argc > 1) {
		//if quantum is 1st arg
		if(p1strneq(argName, argv[1], argLen)){
			quantum = p1atoi(&(argv[1][argLen]));
		}
		//if quantum is 2nd arg
		else if(p1strneq(argName, argv[2], argLen)){
			quantum = p1atoi(&(argv[2][argLen]));
		}
	}

	return quantum;
}

char* getWorkload(int argc, char *argv[]){
	char* fileName = NULL;
	int fd;


	//check if file in argv
	if(argc > 1){
		if (p1strneq(argv[1], "-", 1)){
			fileName = argv[2];
		}
		else if (p1strneq(argv[2], "-", 1)){
			fileName = argv[2];
		}
	}
	fd = open(fileName, 0);

	// if filename not in argv read from stdin
	if (fd < 0){
		fd = 0;
	}
	int n;
	char buff[256];
	while((n = p1getline(fd, buff, sizeof(buff))) < 0){
		puts(buff);
	}


	return fileName;
}

int main(int argc, char *argv[]){
	//get quantum
	printf("%d\n", getQuantum(argc, argv));
	if (getQuantum(argc, argv) < 0){
		puts("No quantum found or specified."); // TODO: remove?
		exit(1);
	}

	//get workload file form cmd line
	char *filename = getWorkload(argc, argv);
	if (filename == NULL){
		//get workload file from stdin

	}
	puts(filename);

	//run each program

}
