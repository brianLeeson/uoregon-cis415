#include "p1fxns.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

#define BUFFSIZE 256

typedef struct command{
	struct command *next;
	char *cmd;
	char **args;
}Command;

//Linked List
typedef struct commandList{
	Command *start;
}CommandList;


Command *createCommand(int numArgs){
	Command *commandStruct = (Command *)malloc(sizeof(Command));

	if (commandStruct != NULL){
		commandStruct->args = (char **) malloc((numArgs+1) * sizeof(char *));

		if (commandStruct->args == NULL){
			free(commandStruct);
			return commandStruct = NULL;
		}
		commandStruct->next = NULL;
	}

	return commandStruct;
}

void destroyCommand(Command *command){
	//free cmd
	puts("free cmd");
	if (command->cmd != NULL){
		free(command->cmd);

	}

	//free args
	int i = 0;
	char *current;
	while((current = command->args[i++]) != NULL){
		puts("free current char*");
		free(current);
	}
	//free struct
	puts("free command");
	free(command);
}

CommandList *createCommandList(){
	CommandList *commandListStruct = (CommandList *)malloc(sizeof(CommandList));

	if (commandListStruct != NULL){
		commandListStruct->start = NULL;
	}

	return commandListStruct;
}

void destroyCommandList(CommandList *commandList){
	puts("destroying CL");
	// free commands
	Command *current;
	if ((current = commandList->start) != NULL){
		Command *next;

//		puts("destroying dummy first");
//		//free dummy struct
//		current = first->next;
//		free(first);

		puts("destroying C");
		//while more command structs to free
		while ((next = current->next) != NULL){
			//free current
			destroyCommand(current);
			current = next;
		}
		destroyCommand(current);
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
	puts("In getquantum");

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
	prevCommand = createCommand(0);
	if (prevCommand == NULL){
		exit(1); //TODO make proper
	}
	commandList->start = prevCommand;

	puts("getting from buff");
	//while lines remaining in workfile
	while((n = p1getline(fd, buff, sizeof(buff))) > 0){
		puts(buff);

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

		puts("getting cmd");
		//get command
		p1getword(buff, 0, word);
		stripNewLine(word);
		currCommand->cmd = p1strdup(word);
		printf("should work: %s\n", currCommand->cmd);

		puts("getting args");
		//get args
		int index = 0;
		while ((j = p1getword(buff, j, word)) > 0){
			stripNewLine(word);
			currCommand->args[index++] = p1strdup(word);
		}
		currCommand->args[index+1] = NULL;

		prevCommand->next = currCommand;
		prevCommand = currCommand;
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
	puts("In  getworkload");
	char* fileName = NULL;
	int fd;


	//check if file in argv
	if(argc > 1){
		puts("checking cmd line for workfile");

		if (p1strneq(argv[1], "-", 1)){
			fileName = argv[2];
		}
		else if (p1strneq(argv[2], "-", 1)){
			fileName = argv[2];
		}
	}


	CommandList *commandList = createCommandList();
	if (commandList == NULL){
		exit(1); //TODO make proper
	}

	// if filename in argv
	if (fileName != NULL){
		puts("workfile in cmd line");
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


int main(int argc, char *argv[]){

	//get quantum
	int quantum;
	if ((quantum = getQuantum(argc, argv)) < 0){
		puts("No quantum found or specified."); // TODO: remove?
		exit(1);
	}

	//command array from commandline or stdin
	CommandList *argList = getWorkload(argc, argv);

	//print commands an and args
	puts("\n**printing cmds and args**");
	Command *command = argList->start;
	if(command != NULL){
		do{
			printf("cmd: %s\n", command->cmd);
			int i = 0;
			char *arg;
			while((arg = command->args[i++]) != NULL){
				printf("\targ%d: %s\n ", i-1, arg);
			}
		}while((command = command->next) != NULL);
	}


	//run each program

	//free
	destroyCommandList(argList);
}
