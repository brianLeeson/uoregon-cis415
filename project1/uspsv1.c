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

#include "p1fxns.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFSIZE 256

typedef struct command{
	struct command *next;
	char *cmd;
	char **args;
}Command;

//Linked List
typedef struct commandList{
	Command *start;
	int numCommands; //this does not include dummy
}CommandList;


Command *createProcess(int numArgs){
	Command *commandStruct = (Command *)malloc(sizeof(Command));

	if (commandStruct != NULL){
		commandStruct->args = (char **) malloc((numArgs+1) * sizeof(char *));

		if (commandStruct->args == NULL){
			free(commandStruct);
			return commandStruct = NULL;
		}
		commandStruct->cmd = NULL;
		commandStruct->next = NULL;
	}

	return commandStruct;
}

void destroyProcess(Command *command){
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

CommandList *createProcessList(){
	CommandList *commandListStruct = (CommandList *)malloc(sizeof(CommandList));

	if (commandListStruct != NULL){
		commandListStruct->start = NULL;
		commandListStruct->numCommands = 0;
	}
	return commandListStruct;
}

void destroyProcessList(CommandList *commandList){
	// free commands
	Command *current;
	if ((current = commandList->start) != NULL){
		Command *next;

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
	free(commandList);
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

void setCommandList(int fd, CommandList *commandList){
	int n;
	char buff[BUFFSIZE];
	Command *prevCommand = NULL;
	Command *currCommand = NULL;

	//make dummy first Command
	prevCommand = createProcess(0);
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
		currCommand = createProcess(numArgs);
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

CommandList* getWorkload(int argc, char *argv[]){
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

	CommandList *commandList = createProcessList();
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

void forkPrograms(CommandList *argList){
	/*
	 * function takes a commandlist that represents all programs to execute.
	 * calls all programs in the argslist
	 */
	int *pidList;
	int numprograms = argList->numCommands;

	//get command, skip dummy
	Command *command = argList->start->next;

	//malloc for pidList
	pidList = (int *) malloc(numprograms * sizeof(int));
	if (pidList == NULL){
		exit(1); //TODO: make proper
	}

	int i;
	for(i=0; i < numprograms; i++){
		pidList[i] = fork();
		if (pidList[i] == 0){
			char *prog = command->cmd;
			char **args = command->args;
			execvp(prog, args);

			//if illegal program or unallowed program
			p1perror(2, "execvp fail");
			exit(1);
		}
		command = command->next;
	}
	for(i=0; i < numprograms; i++){
		waitpid(pidList[i],0 ,0 );
	}

	//dealloc pidList
	free(pidList);
}

int main(int argc, char *argv[]){

	//get quantum
	int quantum;
	if ((quantum = getQuantum(argc, argv)) < 0){
		puts("No quantum found or specified."); // TODO: remove?
		exit(1);
	}

	//command array from commandline or stdin
	CommandList *argList = getWorkload(argc, argv);

	//run each program 	and wait until they are all done
	forkPrograms(argList);

	//free
	destroyProcessList(argList);

	//exit when done
	exit(0);
}
