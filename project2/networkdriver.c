/*
 * networkdriver.c
 *
 *  Created on: May 11, 2017
 *      Author: brian
 *		ID: bel
 *		Assignment: CIS 415 Project 2
 *		This is my own work except that... Sam and I talked about the flow of data through the network drive and how that would work.
 *		It was sam's idea to recycle the pd in get_from_network. We both talked out loud about segfaults on each other's code.
 */

#include "packetdescriptor.h"
#include "destination.h"
#include "pid.h"
#include "freepacketdescriptorstore.h"
#include "freepacketdescriptorstore__full.h"
#include "BoundedBuffer.h"
#include "stdlib.h"
#include "stdio.h"
#include "packetdescriptorcreator.h"
#include "pthread.h"
#include "networkdevice.h"

#define APP_BB_SIZE 2
#define UNUSED __attribute__((unused))

/* any global variables required for use by your threads and your driver routines */
int NUM_PID = 0;
int INITIALIZED = 0;
volatile int DONE = 0;

NetworkDevice *ND;
FreePacketDescriptorStore *FPDS;

BoundedBuffer *TO_APP_BUFF[MAX_PID+1];
BoundedBuffer *TO_NET_BUFF;

pthread_t to_network;
pthread_t from_network;

/* definition[s] of function[s] required for your thread[s] */
void *put_on_network(UNUSED void *args){
	while(!DONE){
		PacketDescriptor *pd = (PacketDescriptor *) blockingReadBB(TO_NET_BUFF);
		int attempts = 3;

		//attempt to send the pd at most attempts times
		while((attempts--) && (send_packet(ND, pd) != 1)){}

		blocking_put_pd(FPDS, pd);
	}
	pthread_exit(NULL);
}

void *get_from_network(UNUSED void *args){
	int recycling = 0;
	while(!DONE){
		PacketDescriptor *pd;

		//get from fpds and register packet
		if(!recycling){
			blocking_get_pd(FPDS, &pd);
		}
		init_packet_descriptor(pd);
		register_receiving_packetdescriptor(ND, pd);

		//listen to network
		await_incoming_packet(ND);

		//put in TO_APP_BUFF. if full, drop packet. If we fail, recycle pd
		recycling = !nonblockingWriteBB(TO_APP_BUFF[packet_descriptor_get_pid(pd)], (void *) pd);
	}
	pthread_exit(NULL);
}

void blocking_send_packet(PacketDescriptor *pd){
	if(!INITIALIZED){
		printf("network driver not initialized.");
		return;
	}
	blockingWriteBB(TO_NET_BUFF, (void *) pd);
}
int nonblocking_send_packet(PacketDescriptor *pd){
	if(!INITIALIZED){
		printf("network driver not initialized.");
		return 0;
	}
	return nonblockingWriteBB(TO_NET_BUFF, (void *) pd);
}
/* These calls hand in a PacketDescriptor for dispatching */
/* The nonblocking call must return promptly, indicating whether or */
/* not the indicated packet has been accepted by your code          */
/* (it might not be if your internal buffer is full) 1=OK, 0=not OK */
/* The blocking call will usually return promptly, but there may be */
/* a delay while it waits for space in your buffers.                */
/* Neither call should delay until the packet is actually sent      */

void blocking_get_packet(PacketDescriptor **pd, PID pid){
	if(!INITIALIZED){
		printf("network driver not initialized.");
		return;
	}
	*pd = (PacketDescriptor *) blockingReadBB(TO_APP_BUFF[(unsigned int) pid]);
}
int  nonblocking_get_packet(PacketDescriptor **pd, PID pid){
	if(!INITIALIZED){
		printf("network driver not initialized.");
		return 0;
	}
	return nonblockingReadBB(TO_APP_BUFF[(unsigned int) pid], (void **) pd);
}
/* These represent requests for packets by the application threads */
/* The nonblocking call must return promptly, with the result 1 if */
/* a packet was found (and the first argument set accordingly) or  */
/* 0 if no packet was waiting.                                     */
/* The blocking call only returns when a packet has been received  */
/* for the indicated process, and the first arg points at it.      */
/* Both calls indicate their process number and should only be     */
/* given appropriate packets. You may use a small bounded buffer   */
/* to hold packets that haven't yet been collected by a process,   */
/* but are also allowed to discard extra packets if at least one   */
/* is waiting uncollected for the same PID. i.e. applications must */
/* collect their packets reasonably promptly, or risk packet loss. */

void init_network_driver(NetworkDevice *nd, void *mem_start, unsigned long mem_length, FreePacketDescriptorStore **fpds_ptr){
/* Called before any other methods, to allow you to initialize */
/* data structures and start any internal threads.             */
/* Arguments:                                                  */
/*   nd: the NetworkDevice that you must drive,                */
/*   mem_start, mem_length: some memory for PacketDescriptors  */
/*   fpds_ptr: You hand back a FreePacketDescriptorStore into  */
/*             which you have put the divided up memory        */
/* Hint: just divide the memory up into pieces of the right size */
/*       passing in pointers to each of them                     */


	ND = nd;
/* create Free Packet Descriptor Store */
	if ((*fpds_ptr = create_fpds()) == NULL){
		printf("Failed to create free packet descriptor store.");
		goto clean;
	}
	FPDS = *fpds_ptr;

/* load FPDS with packet descriptors constructed from mem_start/mem_length */
	if((NUM_PID = create_free_packet_descriptors(*fpds_ptr, mem_start, mem_length)) == 0){
		printf("Failed to create free packet descriptors. NUM_PID: %d\n", NUM_PID);
		goto clean;
	}

/* create any buffers required by your thread[s] */

	//create buffer for apps to put into
	if((TO_NET_BUFF = createBB(MAX_PID)) == NULL){
		printf("Failed to TO_NET_BUFF.");
		goto clean;
	}

	//create MAX_PID + 1 buffers, so that each app has a buffer to get from
	int i;
	for(i=0; i <= MAX_PID; i++){
		if((TO_APP_BUFF[i] = createBB(APP_BB_SIZE)) == NULL){
			//log error and return
			printf("Failed to create TO_APP_BUFF for app %d\n", i);
			goto clean;
		}
	}

/* create any threads you require for your implementation */
	//putting on network
	if(pthread_create(&to_network, NULL, put_on_network, NULL) != 0){
		printf("Failed to create to_network pthread");
		goto clean;
	}

	//getting from network
	if(pthread_create(&from_network, NULL, get_from_network, NULL) != 0){
		printf("Failed to create from_network pthread");
		goto clean;
	}

/* return the FPDS to the code that called you */

	INITIALIZED = 1;
	return;

	clean:
		printf("cleaning up");

		//clean threads
		DONE = 1;

		//clean TO_NET_BUFF
		if (TO_NET_BUFF != NULL){
			destroyBB(TO_NET_BUFF);
		}

		//clean TO_APP_BUFF
		if (TO_APP_BUFF != NULL){
			int i;
			for(i=0; i <= MAX_PID; i++){
				if(TO_APP_BUFF[i] != NULL){
					destroyBB(TO_APP_BUFF[i]);
				}
			}
		}

		// clean FPDS
		if(FPDS != NULL){
			destroy_fpds(FPDS);
			FPDS = NULL;
		}
}



