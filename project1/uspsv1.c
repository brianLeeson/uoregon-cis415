#include "p1fxns.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

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
		//malloc for at most 3 arguments of at most 30 chars each
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
	//TODO: A lot
}

CommandList *createCommandList(){
	CommandList *commandListStruct = (CommandList *)malloc(sizeof(CommandList));

	if (commandListStruct != NULL){
		commandListStruct->start = NULL;
	}

	return commandListStruct;
}

void destroyCommandList(CommandList *commandList){
	// free commands
	Command *current;
	if ((current = commandList->start) != NULL){
		Command *next = NULL;
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

CommandList* getWorkload(int argc, char *argv[]){
	/*
	 * function takes arc and argv
	 *  TODO: return an array of commands
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

	int n;
	char buff[256];
	CommandList *commandList = createCommandList();
	if (commandList == NULL){
		exit(1); //TODO make proper
	}

	// if filename in argv
	if (fileName != NULL){
		fd = open(fileName, 0);
		//parse buff

		//get numArgs
		int numArgs = 0;
		char wordBuff[100];
		while ((p1getword(buff, 0, wordBuff)) > -1){
			numArgs++;
		}

		Command *currCommand = createCommand(numArgs);
		if (currCommand == NULL){
			exit(1); //TODO make proper
		}
		commandList->start = currCommand;

		Command *prevCommand = NULL;
		Command *nextCommand = NULL;
		while((n = p1getline(fd, buff, sizeof(buff))) < 0){
			puts(buff);


			char word[100]; //assume no arg is more than 99 chars long
			int i;

			//get command
			i = p1getword(buff, 0, word);
			//TODO strip \n from word
			currCommand->cmd = p1strdup(word);

			//get args
			int index = 0;
			while ((i = p1getword(buff, i, word)) > 0){
				//TODO strip \n from word
				currCommand->args[index++] = p1strdup(word);
			}
			currCommand->args[index+1] = NULL;

			prevCommand = currCommand;
			nextCommand = createCommand(index);
			currCommand->next = nextCommand;

			if (nextCommand == NULL){
				exit(1); //TODO make proper
			}

			currCommand = nextCommand;
		}
		//preCommand is end of linked list
		prevCommand->next = NULL;

		destroyCommand(currCommand);


	}

	//else read from stdin
	else{
		fd = 0;
		while((n = p1getline(fd, buff, sizeof(buff))) > 0){
			puts(buff);
		}
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

	//run each program

}
