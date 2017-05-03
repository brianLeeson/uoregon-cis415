#include "p1fxns.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

int main(int argc, char *argv[]){
	//get quantum
	printf("%d\n", getQuantum(argc, argv));
	if (getQuantum(argc, argv) < 0){
		puts("No quantum found or specified.");
		exit(1);
	}

}
