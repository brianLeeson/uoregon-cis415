/*
 * uspsv3.c
 *
 *  Created on: Apr 28, 2017
 *      Author: brian
 *      ID: bel
 *      CIS 415 Project 1
 *
 *      This is my own work except Sam Oberg and I
 *      talked out loud about C syntax, data structures, and some function calls.
 */

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
#define NALARMS 10
#define INTERVAL 500

volatile int USR1_received = 0;
int alarms_left = NALARMS;

typedef struct process{
	struct process *next;
	char *cmd;
	char **args;
	int pid;
	int status;
}Process;

//Linked List
typedef struct processList{
	Process *start;
	int numCommands; //this does not include dummy
}ProcessList;


Process *createCommand(int numArgs){
	Process *commandStruct = (Process *)malloc(sizeof(Process));

	if (commandStruct != NULL){
		commandStruct->args = (char **) malloc((numArgs+1) * sizeof(char *));

		if (commandStruct->args == NULL){
			free(commandStruct);
			return commandStruct = NULL;
		}
		commandStruct->cmd = NULL;
		commandStruct->next = NULL;
		commandStruct->pid = -1; //no pid yet;
		commandStruct->status = 0; //no status yet
	}

	return commandStruct;
}

void destroyCommand(Process *command){
	//free cmd
	free(command->cmd);

	//free args
	int i = 0;
	char *arg;
	while((arg = command->args[i++]) != NULL){
		free(arg);
	}
	//free struct
	free(command);
}

ProcessList *createCommandList(){
	ProcessList *processListStruct = (ProcessList *)malloc(sizeof(ProcessList));

	if (processListStruct != NULL){
		processListStruct->start = NULL;
		processListStruct->numCommands = 0;
	}
	return processListStruct;
}

void destroyCommandList(ProcessList *processList){
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
			destroyCommand(current);
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
	prevCommand = createCommand(0);
	if (prevCommand == NULL){
		exit(1); //TODO make proper
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
		currCommand = createCommand(numArgs);
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

	ProcessList *commandList = createCommandList();
	if (commandList == NULL){
		exit(1); //TODO make proper
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
		case(SIGALRM):
			puts("received SIGALRM");
	}
}

static void onalrm(UNUSED int sig) {
	printf("Timer went off.\n");
	alarms_left--;
}

void setSignalHandlers(){
	//set sigusr1 handlers
    if (signal(SIGUSR1, onusr1) == SIG_ERR) {
        fprintf(stderr, "Can't establish SIGUSR1 handler\n");

    }
    if (signal(SIGALRM, onalrm) == SIG_ERR) {
        fprintf(stderr, "Can't establish SIGALRM handler\n");
        exit(1);
    }
}

int *forkPrograms(ProcessList *processList){
	/*
	 * function takes a processList that represents all programs to execute.
	 * calls all programs in the argslist
	 */
	int *pidList;
	int numPrograms = argList->numCommands;

	//get command, skip dummy
	Process *command = argList->start->next;

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

int main(int argc, char *argv[]){
	//get quantum
	int quantum;
	if ((quantum = getQuantum(argc, argv)) < 0){
		puts("No quantum found or specified."); // TODO: remove?
		exit(1);
	}

	//command array from commandline or stdin
	ProcessList *processList = getWorkload(argc, argv);
	int numPrograms = processList->numCommands;

	//run each program 	and wait until they are all done
	int * pidList;

	setSignalHandlers();

	pidList = forkPrograms(processList);

	struct itimerval it_val;
	it_val.it_value.tv_sec = quantum/1000;
	it_val.it_value.tv_usec = (quantum*1000) % 1000000;
	it_val.it_interval = it_val.it_value;
	if (setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
		perror("error calling setitimer()");
		return 1;
	}
	while (alarms_left){
		pause();
	}

	//free
	destroyCommandList(processList);

	//dealloc pidList
	free(pidList);

	//exit when done
	exit(0);
}
