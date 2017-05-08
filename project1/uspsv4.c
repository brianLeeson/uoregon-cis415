/*
 * uspsv3.c
 *
 *  Created on: Apr 28, 2017
 *      Author: brian
 *      ID: bel
 *      CIS 415 Project 1
 *
 *      This is my own work
 *      ADT Queue based off of: https://github.com/rkwan/adt/blob/master/queue/queue.c
 *
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

#define BUFSIZE 512
#define UNUSED __attribute__((unused))
struct q;
typedef struct q Queue;
typedef struct process Process;

volatile int USR1_received = 0;
volatile int processesAlive = 0;
Process *curProc;
int *pidList;

//make Global queue
Queue *pQueue;

struct process{
	char *cmd;
	char **args;
	int pid;
	int status;
	int numArgs;
};


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
		processStruct->pid = -1; //no pid yet;
		processStruct->status = 2; //2: waiting for usr1, 1, for cont, 0 it's dead jim.
	}

	return processStruct;
}
int dp = 0;
void destroyProcess(Process *p){
	dp++;
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

int j = 0;
void deleteQueue() {
	while(!isQueueEmpty()) {
		destroyProcess(dequeue());
		j++;
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
	char buff[BUFSIZE];
	Process *currProcess = NULL;

	//while lines remaining in workfile
	while((n = p1getline(fd, buff, sizeof(buff))) > 0){

		int numArgs = 0;
		char wordBuff[100];
		char tempBuff[BUFSIZE];
		int i = 0;

		//get num args in buff
		p1strcpy(tempBuff, buff);
		while ((i = p1getword(tempBuff, i, wordBuff)) > 0){
			numArgs++;
		}
		currProcess = createProcess(numArgs);
		if (currProcess == NULL){
			p1perror(2, "failed creating process");
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

void displayUsage(Process *p){
	//display information about process about to be scheduled
	int pid = p->pid;
	char filePath[BUFSIZE];
	printf("PID: %d --- ", pid);

	//put pid as string into buff
	char buf[BUFSIZE];
	p1itoa(pid, buf);



	// command being executed
	p1strcpy(filePath, "/proc/");
	p1strcat(filePath, buf);


	printf("path is: %s\n", filePath);

	//fd = open(fileName, 0);

	//execution time

	//memory used

	//and I/O

}

static void onusr1(UNUSED int sig){
	//this function handles usr1 signals that are sent to our process.
	switch(sig){
		case(SIGUSR1):
			USR1_received++;
			break;
		default:
			break;
	}
}

static void onalrm(UNUSED int sig) {
	//on alarm called periodically based on quantum. does the scheduling work

	//stop curProc
	kill(curProc->pid, SIGSTOP);
	enqueue(curProc);
	int found = 0;
	while(!isQueueEmpty() && !found){
		//get new proc
		curProc = dequeue();

		//if status 0, destroy
		if (curProc->status == 0){
			destroyProcess(curProc);
		}

		//else, success
		else{
			found = 1;
		}
	}

	displayUsage(curProc);

	if (curProc->status == 2){
		curProc->status = 1;
		kill(curProc->pid, SIGUSR1);
	}
	else if(curProc->status == 1){
		kill(curProc->pid, SIGCONT);
	}
	else{
		p1perror(2, "this should never happen");
	}
}

static void onchild(UNUSED int sig){
	//set status to 0
	pid_t pid;
	int status;

	while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
		if (WIFEXITED(status) || WIFSIGNALED(status)) {
			processesAlive--;

			if(curProc->pid == pid ){
				curProc->status = 0;
			}
			else{
				//iterate through pQueue and set process status of pid to 0.
				ProcessNode *cur = pQueue->head;
				while(cur != NULL){
					if (cur->process->pid == pid){
						cur->process->status = 0;
						break;
					}
					cur = cur->next;
				}
			}

		}
	}
}

void setSignalHandlers(){
	//set sigusr1 handlers
    if (signal(SIGUSR1, onusr1) == SIG_ERR) {
    	p1perror(2, "Can't establish SIGUSR1 handler\n");
    	exit(1);
    }
    if (signal(SIGALRM, onalrm) == SIG_ERR) {
    	p1perror(2, "Can't establish SIGALRM handler\n");
        exit(1);
    }
    if (signal(SIGCHLD, onchild) == SIG_ERR) {
    	p1perror(2, "Can't establish SIGCHLD handler\n");
        exit(1);
    }
}

int *forkPrograms(){
	/*
	 * calls all programs in the queue
	 */
	int numprograms = pQueue->initialSize;

	//malloc for pidList
	pidList = (int *) malloc(numprograms * sizeof(int));
	if (pidList == NULL){
		p1perror(2, "failed creating pid");
		exit(1);
	}

	int i = 0;
	struct timespec tm = {0, 20000000};
	ProcessNode *cur = pQueue->head;
	while(cur != NULL){
		pidList[i] = fork();
			if (pidList[i] == 0){
				//wait for sigusr1
				while (! USR1_received){
					(void)nanosleep(&tm, NULL);
				}

				char *prog = cur->process->cmd;
				char **args = cur->process->args;
				execvp(prog, args);

				//if illegal program or unallowed program
				p1perror(2, "execvp fail");
				exit(1);
			}
			else if (pidList[i] > 0){
				cur->process->pid = pidList[i];
			}
			else{
				p1perror(2, "fork unsuccessful");
				exit(1);
			}
		cur = cur->next;
		i++;
	}


	return pidList;
}

void setTimer(int quantum){
	struct itimerval it_val;
	it_val.it_value.tv_sec = quantum/1000;
	it_val.it_value.tv_usec = (quantum*1000) % 1000000;
	it_val.it_interval = it_val.it_value;
	if (setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
		p1perror(2, "error calling setitimer()");
		exit(1);
	}
}

int main(int argc, char *argv[]){
	queueInit();

	//get quantum
	int quantum = getQuantum(argc, argv);
	if (quantum< 0){
		p1perror(2, "No quantum found or specified.");
		exit(1);
	}

	//make process array from commandline or stdin
	getWorkload(argc, argv);
	int numProcesses = pQueue->initialSize;
	processesAlive = numProcesses;

	//set sig handlers
	setSignalHandlers();

	//create children, return list of children
	forkPrograms();

	//set timer base on quantum
	setTimer(quantum);

	//start first process
	curProc = dequeue();
	kill(curProc->pid, SIGUSR1);

	struct timespec tm = {0, 20000000};
	while (processesAlive){
		(void)nanosleep(&tm, NULL);
	}

	//dealloc pidList
	free(pidList);

	//delete queue
	deleteQueue();
	destroyProcess(curProc); //don't like that I do this, should be handled in deleteQueue
	//exit when done
	exit(0);

}
