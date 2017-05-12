#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

#define THREAD_COUNT 4

char *messages[] = {"Hello","World","Test","String",NULL};
int thread_count = THREAD_COUNT;
pthread_t pthreads[THREAD_COUNT]; 

int memory_size = 0;
volatile char *memory = "StartingMessage";

void set_message(char *message){
	memory = message;
}

char *get_message(){
	char *returnValue = memory;
	return returnValue;
}

void *pthread_proc(void *ptr){
	char *message = (char*)ptr;
	
	// check message... ?
	// print the message, and thread id.

	int exit_count = 1000;
	while(exit_count-- > 0)
	{
		// set the message.
		// get the message.
		// print the messages, and thread id. if they are different.
		
	}
	return NULL;
}

void create_threads(){
	// create a thread passing 1 message each.
	// for each:  print the thread id and the message passed. 
	pthread_t t1, t2, t3, t4;
	int i;
	//make array of pthreads
	for(i = 0; i < THREAD_COUNT; i++){
		pthreads[i] =

	}

	for(i = 0; i<THREAD_COUNT; i++){
		if (pthread_create(&t1, NULL, pthread_proc, (void *)messages[i])) {
			fprintf(stderr, "Error creating thread 1\n");
			exit(1);
		}

	}

}

void wait_for_threads_to_exit(){
	// spin in a loop, wait for each thread to exit.

}

int main(int argc, char *argv[]){
	create_threads();
	
	wait_for_threads_to_exit();
	return 0;
}
