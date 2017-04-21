#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include<signal.h>
#include<unistd.h>


typedef struct _argparse
{
	int ThreadCount;
	int TimerCountLower;
	int TimerCountUpper;
	char *Message;
} Argparse; 

Argparse *malloc_Argparse(int thread_count, int timer_count_upper,int timer_count_lower, char*msg)
{
	Argparse *returnValue = NULL;
	returnValue = (Argparse*)malloc(sizeof(Argparse));
	returnValue ->ThreadCount = thread_count;
	returnValue ->TimerCountUpper = timer_count_upper;
	returnValue-> TimerCountLower = timer_count_lower;
	if(msg != NULL)
	{
		returnValue-> Message = (char*)malloc(sizeof(char)*(strlen(msg)+1));
		strcpy(returnValue->Message,msg);
	}
	else
	{
		returnValue-> Message = (char*)malloc(sizeof(char)*1);
		strcpy(returnValue->Message,"");
	}
	return returnValue;
}

void free_Argparse(Argparse **ptr)
{
	free((*ptr)->Message);
	free((*ptr));
	*ptr = 0;
}

Argparse *ParseArguments(int argc, char*argv[])
{
	char *message = NULL;
	int thread_count = 0;
	int timer_count_lower =0;
	int timer_count_upper =0;

	int index =0;
	int arg_len=0;
	int position = 0;
	char *arg = NULL;
	int mode =0;
	for(index =1; index < argc; index += 1)
	{
		arg = argv[index];
		arg_len = strlen(arg);
		position =0;
		
		// code state switching such as from -t -> makes a mode
		
		switch(mode)
		{
			case 0:
				if((strcmp("--message",arg)==0) || strcmp("-M",arg)==0)
				{
					mode = 1;
				}
				else if(strcmp("-",arg)==0)
				{
				}
				else
				{
					fprintf(stderr,"Invalid option for starting argument.\n");
					exit(-1);
				}
				break;
			case 1:
				message = arg;
				mode =0;
				break;
			default:
				fprintf(stderr,"Invalid mode state for parser: %d \n",mode);
				exit(mode);
		}									
	}
	
	return malloc_Argparse(thread_count,timer_count_upper,timer_count_lower,arg);	
}

void print_Argparse(Argparse *args)
{
	fprintf(stdout,"Argparse->ThreadCount = %d\n",args->ThreadCount);
	fprintf(stdout,"Argparse->TimerCountUpper= %d\n",args->TimerCountUpper);
	fprintf(stdout,"Argparse->TimerCountLower= %d\n",args->TimerCountLower);
	fprintf(stdout,"Argparse->Message = %s\n",args->Message);

}

typedef struct _thread_container
{
	pthread_t thread;

}ThreadContainer;

void *CreateMemoryMap(ThreadContainer *tc)
{
	// implement and create a map here. 
}

void DestroyMemoryMap(void *memory,ThreadContainer *tc)
{
	// implement and destroy a map here.
}

void print_ThreadID()
{
	fprintf(stdout,"Current threadID: %d\n",(int)pthread_self());
}


void sig_handler(int signo)
{
    if (signo == SIGUSR1)
        printf("received SIGUSR1\n");
    else if (signo == SIGKILL)
        printf("received SIGKILL\n");
    else if (signo == SIGSTOP)
        printf("received SIGSTOP\n");
}


void ThreadProc(void *ptr)
{
	// catch the ptr here and cast to the object passed in.
	ThreadContainer *tc = (ThreadContainer*)ptr;
	// here you could print the message if you had it...
	if (signal(SIGUSR1, sig_handler) == SIG_ERR)
        	printf("\ncan't catch SIGUSR1\n");
    	if (signal(SIGKILL, sig_handler) == SIG_ERR)
        	printf("\ncan't catch SIGKILL\n");
    	if (signal(SIGSTOP, sig_handler) == SIG_ERR)
        	printf("\ncan't catch SIGSTOP\n");	
	print_ThreadID();
}


ThreadContainer *create_threads(int thread_count, void *argument)
{
	ThreadContainer *ptr = malloc(sizeof(ThreadContainer)*thread_count);

	// start a lot of threads here...
	return ptr;
}

void stop_threads(int thread_count, ThreadContainer *ptr)
{
	// stop all the threads here.
	free(ptr);
}
int main(int argc, char *argv[])
{
	Argparse *options = NULL;

	options = ParseArguments(argc,argv);	
	
	print_Argparse(options);

	if (signal(SIGUSR1, sig_handler) == SIG_ERR)
        	printf("\ncan't catch SIGUSR1\n");
    	if (signal(SIGKILL, sig_handler) == SIG_ERR)
        	printf("\ncan't catch SIGKILL\n");
    	if (signal(SIGSTOP, sig_handler) == SIG_ERR)
        	printf("\ncan't catch SIGSTOP\n");

	ThreadContainer *ptr = create_threads(options->ThreadCount,(void*)options);
	stop_threads(options->ThreadCount,ptr);

	free_Argparse(&options);
	return 42;
}
