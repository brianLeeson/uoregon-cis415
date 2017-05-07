/*
 * uspsv3.c
 *
 *  Created on: Apr 28, 2017
 *      Author: brian
 *      ID: bel
 *      CIS 415 Project 1
 *
 *      This is my own work except Sam Oberg and I
 *      talked out loud about C syntax, data structures, and some function calls. Help with onchld handler as well
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

#define BUFFSIZE 256
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
	struct process *next;
	char *cmd;
	char **args;
	int pid;
	int status;
};

//Linked List
typedef struct processList{
	Process *start;
	int numCommands; //this does not include dummy
}ProcessList;


ProcessList *processList;

/* ----- QUEUE ----- */

typedef struct pNode {
        Process *process;
        struct pNode *next;
} ProcessNode;

struct q {
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
	int i = 0;
	while(!isQueueEmpty()) {
		dequeue();
		i++;
	}
	free(pQueue);
	pQueue = NULL;
}

/* ----- Functions ----- */

Process *createProcess(int numArgs){
	Process *processStruct = (Process *)malloc(sizeof(Process));

	if (processStruct != NULL){
		processStruct->args = (char **) malloc((numArgs+1) * sizeof(char *));

		if (processStruct->args == NULL){
			free(processStruct);
			return processStruct = NULL;
		}
		processStruct->cmd = NULL;
		processStruct->next = NULL;
		processStruct->pid = -1; //no pid yet;
		processStruct->status = 2; //2: waiting for usr1, 1, for cont, 0 it's dead jim.
	}

	return processStruct;
}

void destroyProcess(Process *p){
	//free cmd
	free(p->cmd);

	//free args
	int i = 0;
	char *arg;
	while((arg = p->args[i++]) != NULL){
		free(arg);
	}
	//free struct
	free(p);
}

ProcessList *createProcessList(){
	ProcessList *processListStruct = (ProcessList *)malloc(sizeof(ProcessList));

	if (processListStruct != NULL){
		processListStruct->start = NULL;
		processListStruct->numCommands = 0;
	}
	return processListStruct;
}

void destroyProcessList(ProcessList *processList){
	// free commands
	Process *current;
	if ((current = processList->start) != NULL){
		Process *next;

		//free dummy command struct
		next = current->next;
		free(current);
		current = next;

		//while more command structs to free
		while (current != NULL){
			//free current
			next = current->next;
			destroyProcess(current);
			current = next;
		}
	}

	// free CommandList
	free(processList);
}

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

void setCommandList(int fd, ProcessList *commandList){
	int n;
	char buff[BUFFSIZE];
	Process *prevCommand = NULL;
	Process *currCommand = NULL;

	//make dummy first Command
	prevCommand = createProcess(0);
	if (prevCommand == NULL){
		exit(1);
	}
	commandList->start = prevCommand;

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
		currCommand = createProcess(numArgs);

		//add currCommand to queue
		enqueue(currCommand);

		if (currCommand == NULL){
			exit(1); //TODO make proper
		}

		char word[100]; //assume no arg is more than 99 chars long
		int j = 0;

		//get command
		p1getword(buff, 0, word);
		stripNewLine(word);
		currCommand->cmd = p1strdup(word);

		//get args
		int index = 0;
		while ((j = p1getword(buff, j, word)) > 0){
			stripNewLine(word);
			currCommand->args[index++] = p1strdup(word);
		}
		currCommand->args[index] = NULL;

		prevCommand->next = currCommand;
		prevCommand = currCommand;
		commandList->numCommands++;
	}
	//prevCommand is last struct of linked list
	if (commandList->start != NULL){
		prevCommand->next = NULL;  //redundant?
	}

}

ProcessList* getWorkload(int argc, char *argv[]){
	/*
	 * function takes arc and argv
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

	ProcessList *commandList = createProcessList();
	if (commandList == NULL){
		exit(1);
	}

	// if filename in argv
	if (fileName != NULL){
		fd = open(fileName, 0);
		setCommandList(fd, commandList);
	}

	//else read from stdin
	else{
		fd = 0;
		setCommandList(fd, commandList);
	}

	return commandList;
}

static void onusr1(UNUSED int sig){
	//this function handles all signals that are send to our process.
	//except sigstop and sigcont
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

	//find next ready process
	do{
		curProc = dequeue();

	}while(!isQueueEmpty() && (curProc->status == 0));

	if (curProc->status == 2){
		kill(curProc->pid, SIGUSR1);
		curProc->status = 1;
	}
	else if(curProc->status == 1){
		kill(curProc->pid, SIGCONT);
	}
}

static void onchild(UNUSED int sig){
	//set status to 0
	pid_t pid;
	int status;

	while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
		if (WIFEXITED(status) || WIFSIGNALED(status)) {
			processesAlive--;

			//iterate through processList and set process status of pid to 0.
			Process *cur = processList->start->next;
			while(cur != NULL){
				if (cur->pid == pid){
					cur->status = 0;
					break;
				}
				cur = cur->next;
			}
		}
	}
}

void setSignalHandlers(){
	//set sigusr1 handlers
    if (signal(SIGUSR1, onusr1) == SIG_ERR) {
    	p1perror(stderr, "Can't establish SIGUSR1 handler\n");

    }
    if (signal(SIGALRM, onalrm) == SIG_ERR) {
    	p1perror(stderr, "Can't establish SIGALRM handler\n");
        exit(1);
    }
    if (signal(SIGCHLD, onchild) == SIG_ERR) {
    	p1perror(stderr, "Can't establish SIGCHLD handler\n");
        exit(1);
    }
}

int *forkPrograms(ProcessList *processList){
	/*
	 * function takes a processList that represents all programs to execute.
	 * calls all programs in the argslist
	 */
	int *pidList;
	int numPrograms = processList->numCommands;

	//get command, skip dummy
	Process *command = processList->start->next;

	//malloc for pidList
	pidList = (int *) malloc(numPrograms * sizeof(int));
	if (pidList == NULL){
		exit(1); //TODO: make proper
	}

	//start children
	int i;
	struct timespec tm = {0, 20000000};
	for(i=0; i < numPrograms; i++){
		pidList[i] = fork();
		if (pidList[i] == 0){
			//wait for sigusr1
			while (! USR1_received){
				(void)nanosleep(&tm, NULL);
			}

			char *prog = command->cmd;
			char **args = command->args;
			execvp(prog, args);

			//these lines execute if illegal program or unallowed program
			p1perror(2, "execvp fail");
			exit(1);
		}
		else if (pidList[i] > 0){
			command->pid = pidList[i];
		}
		else{
			//fork unsuccessful
			exit(1);
		}
		command = command->next;
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

	deleteQueue(); exit(1);

	//make process array from commandline or stdin
	processList = getWorkload(argc, argv);
	int numProcesses = processList->numCommands;

	//set sig handlers
	setSignalHandlers();

	//create children, return list of children
	pidList = forkPrograms(processList);

	//set timer base on quantum
	setTimer(quantum);

	processesAlive = numProcesses;


	//start first process
	curProc = dequeue();
	kill(curProc->pid, SIGUSR1);

	struct timespec tm = {0, 20000000};
	while (processesAlive){
		(void)nanosleep(&tm, NULL);
	}

	//free
	destroyProcessList(processList);

	//dealloc pidList
	free(pidList);

	//delete queue
	deleteQueue();

	//exit when done
	exit(0);

}
