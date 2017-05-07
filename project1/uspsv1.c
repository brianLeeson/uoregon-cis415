/*
 * uspsv1.c
 *
 *  Created on: Apr 28, 2017
 *      Author: brian
 *      ID: bel
 *      CIS 415 Project 1
 *
 *      This is my own work except Holden Marsh, Sam Oberg, and I
 *      talked out loud about C syntax, data structures, and some function calls.
 */

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>
#include "p1fxns.h"

#define BUFFSIZE 256
#define UNUSED __attribute__((unused))
struct q;
typedef struct q Queue;
typedef struct process Process;

Process *curProc;
int *pidList;

//make Global queue
Queue *pQueue;

struct process{
	struct process *next;
	char *cmd;
	char **args;
	int pid;
	int status;
	int numArgs;
};

//Linked List
typedef struct processList{
	Process *start;
	int numProcesss; //this does not include dummy
}ProcessList;

Process *createProcess(int numArgs){
	Process *processStruct = (Process *)malloc(sizeof(Process));

	if (processStruct != NULL){
		processStruct->args = (char **) malloc((numArgs+1) * sizeof(char *));

		if (processStruct->args == NULL){
			free(processStruct);
			return processStruct = NULL;
		}
		processStruct->numArgs = numArgs;
		processStruct->cmd = NULL;
		processStruct->next = NULL;
	}

	return processStruct;
}

void destroyProcess(Process *p){
	//free cmd
	free(p->cmd);

	//free args
	int i = 0;
	char *arg;
	while(i <= p->numArgs){
		arg = p->args[i++];
		free(arg);
	}
	free(p->args);

	//free struct
	free(p);
}

/* ----- QUEUE ----- */

typedef struct pNode {
        Process *process;
        struct pNode *next;
} ProcessNode;

struct q {
	int initialSize;
	ProcessNode *head;
	ProcessNode *tail;
};

void queueInit() {
	//initializes global pQueue
	pQueue = malloc(sizeof(Queue));
	if (pQueue == NULL) {
		p1perror(2, "Error: queueInit(): failed to malloc");
		exit(1);
	}
	pQueue->initialSize = 0;
	pQueue->head = NULL;
	pQueue->tail = NULL;
}

void enqueue(Process *p) {
	//enqueue to global pQueue
	ProcessNode *newNode = malloc(sizeof(ProcessNode));
	if (newNode == NULL) {
		p1perror(2, "Error: enqueue(): failed to malloc");
		exit(1);
	}
	newNode->process = p;
	newNode->next = NULL;
	if (pQueue->head == NULL) {
		pQueue->head = newNode;
		pQueue->tail = newNode;
	}else {
		pQueue->tail->next = newNode;
		pQueue->tail = newNode;
	}
}

Process *dequeue() {
	if (pQueue->head == NULL) {
		p1perror(2, "Error: dequeue(Queue): dequeuing of an empty");
		exit(1);
	}
	Process *ret = pQueue->head->process;
	ProcessNode *tmp = pQueue->head;
	pQueue->head = pQueue->head->next;
	tmp->next = NULL;
	free(tmp);
	return ret;
}

unsigned int isQueueEmpty(){
	if (pQueue->head == NULL){
		return 1;
	}
	else{
		return 0;
	}
}

void deleteQueue() {
	while(!isQueueEmpty()) {
		destroyProcess(dequeue());
	}
	free(pQueue);
	pQueue = NULL;
}

/* ----- Functions ----- */

void stripNewLine(char word[]){
	int i = 0;
	while (word[i] != '\0'){
		if(word[i] == '\n'){
			word[i] = '\0';
			break;
		}
		i++;
	}
}

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

void setupQueue(int fd){
	int n;
	char buff[BUFFSIZE];
	Process *currProcess = NULL;

	//while lines remaining in workfile
	while((n = p1getline(fd, buff, sizeof(buff))) > 0){

		int numArgs = 0;
		char wordBuff[100];
		char tempBuff[BUFFSIZE];
		int i = 0;

		//get num args in buff
		p1strcpy(tempBuff, buff);
		while ((i = p1getword(tempBuff, i, wordBuff)) > 0){
			numArgs++;
		}
		currProcess = createProcess(numArgs);
		if (currProcess == NULL){
			exit(1);
		}

		char word[100]; //assume no arg is more than 99 chars long
		int j = 0;

		//get command
		p1getword(buff, 0, word);
		stripNewLine(word);
		currProcess->cmd = p1strdup(word);

		//get args
		int index = 0;
		while ((j = p1getword(buff, j, word)) > 0){
			stripNewLine(word);
			currProcess->args[index++] = p1strdup(word);
		}
		currProcess->args[index] = NULL;

		pQueue->initialSize++;
		enqueue(currProcess);
	}
}

void getWorkload(int argc, char *argv[]){
	/*
	 * function takes argc and argv
	 * returns a linked list of command structs
	 */
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

	// if filename in argv
	if (fileName != NULL){
		fd = open(fileName, 0);
		setupQueue(fd);
	}

	//else read from stdin
	else{
		fd = 0;
		setupQueue(fd);
	}
}

void forkPrograms(){
	/*
	 * calls all programs in the queue
	 */
	int *pidList;
	int numprograms = pQueue->initialSize;

	//malloc for pidList
	pidList = (int *) malloc(numprograms * sizeof(int));
	if (pidList == NULL){
		exit(1);
	}

	int i = 0;
	ProcessNode *cur = pQueue->head;
	while(cur != NULL){
		pidList[i] = fork();
			if (pidList[i] == 0){
				char *prog = cur->process->cmd;
				char **args = cur->process->args;
				execvp(prog, args);

				//if illegal program or unallowed program
				p1perror(2, "execvp fail");
				exit(1);
			}
		cur = cur->next;
		i++;
	}

	for(i=0; i < numprograms; i++){
		waitpid(pidList[i],0 ,0 );
	}

	//dealloc pidList
	free(pidList);
}

int main(int argc, char *argv[]){
	queueInit();

	//get quantum
	int quantum;
	if ((quantum = getQuantum(argc, argv)) < 0){
		p1perror(2, "No quantum found or specified.");
		exit(1);
	}

	//process array from commandline or stdin
	getWorkload(argc, argv);

	//run each program 	and wait until they are all done
	forkPrograms();

	//free
	deleteQueue();

	//exit when done
	exit(0);
}
