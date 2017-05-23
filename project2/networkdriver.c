/*
 * networkdriver.c
 *
 *  Created on: May 11, 2017
 *      Author: brian
 *		ID: bel
 *		Assignment: CIS 415 Project 2
 *		This is my own work except that... Sam and I talked about the flow of data through the network drive and how that would work.
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

#include "networkdevice.h"

#define TO_NET_SIZE 100
#define TO_APP_SIZE 100

/* any global variables required for use by your threads and your driver routines */
int NUM_PID = 0;
BoundedBuffer *TO_APP_BUFF[MAX_PID+1];

/* definition[s] of function[s] required for your thread[s] */

void blocking_send_packet(PacketDescriptor *pd){

}
int  nonblocking_send_packet(PacketDescriptor *pd){

}
/* These calls hand in a PacketDescriptor for dispatching */
/* The nonblocking call must return promptly, indicating whether or */
/* not the indicated packet has been accepted by your code          */
/* (it might not be if your internal buffer is full) 1=OK, 0=not OK */
/* The blocking call will usually return promptly, but there may be */
/* a delay while it waits for space in your buffers.                */
/* Neither call should delay until the packet is actually sent      */

void blocking_get_packet(PacketDescriptor **pd, PID pid){
	*pd = (PacketDescriptor *) blockingReadBB(TO_APP_BUFF[(unsigned int) pid]);
}
int  nonblocking_get_packet(PacketDescriptor **pd, PID pid){
	return nonblockingReadBB(TO_APP_BUFF[(unsigned int) pid], (void *) *pd);
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

/* create Free Packet Descriptor Store */
	*fpds_ptr = create_fpds();

/* load FPDS with packet descriptors constructed from mem_start/mem_length */
	if((NUM_PID = create_free_packet_descriptors(*fpds_ptr, mem_start, mem_length)) == 0){
		printf("Failed to create free packet descriptors. NUM_PID: %d\n", NUM_PID);
		return;
	}

/* create any buffers required by your thread[s] */
	//create MAX_PID + 1 buffers, so that each app has a buffer to get from
	int i;
	for(i=0; i <= MAX_PID; i++){
		if((TO_APP_BUFF[i] = createBB(NUM_PID)) == NULL){
			//log error and return
			printf("Failed to create TO_APP_BUFF for app %d\n", i);
			return;
		}
	}



/* create any threads you require for your implementation */
	//listening to app

	//listening to driver

	//sending to app

	//sending to driver



/* return the FPDS to the code that called you */

}



