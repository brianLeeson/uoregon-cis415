/*
 * networkdriver.c
 *
 *  Created on: May 11, 2017
 *      Author: brian
 *		ID: bel
 *		Assignment: CIS 415 Project 2
 *		This is my own work except that: Sam and I talked about the flow of data through the network driver and how that would work.
 *		It was Sam's idea to recycle the pd in get_from_network. I gave him the idea to have while(!DONE) in the thread functions.
 *		We both talked out loud about segfaults on each other's code.
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
#define TRY_TO_SEND 3
#define UNUSED __attribute__((unused))

/* any global variables required for use by your threads and your driver routines */
int NUM_PD = 0;
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
		//get next pd to put on network
		PacketDescriptor *pd = (PacketDescriptor *) blockingReadBB(TO_NET_BUFF);

		//attempt to send the pd at most attempts times
		int attempts = TRY_TO_SEND;
		while((attempts--) && (send_packet(ND, pd) != 1)){}

		if(!attempts){
			printf("Failed to put packet on network after %d attempts\n", TRY_TO_SEND);
		}

		//return pd to fpds
		blocking_put_pd(FPDS, pd);
	}
	pthread_exit(NULL);
}

void *get_from_network(UNUSED void *args){
	int recycling = 0;
	PacketDescriptor *pd;

	while(!DONE){
		//get from fpds and register packet
		if(!recycling){
			blocking_get_pd(FPDS, &pd);
		}
		init_packet_descriptor(pd);
		register_receiving_packetdescriptor(ND, pd);

		//listen to network
		await_incoming_packet(ND);

		//put in TO_APP_BUFF. if full or fail, drop packet and set up to recycle the packet
		recycling = !nonblockingWriteBB(TO_APP_BUFF[packet_descriptor_get_pid(pd)], (void *) pd);
	}
	//return recycled packet to the fpds
	if(recycling){blocking_put_pd(FPDS, pd);}
	pthread_exit(NULL);
}

/*
 * These calls hand in a PacketDescriptor for dispatching
 * The nonblocking call must return promptly, indicating whether or
 * not the indicated packet has been accepted by your code
 * (it might not be if the internal buffer is full) 1=OK, 0=not OK
 */
void blocking_send_packet(PacketDescriptor *pd){
	if(!INITIALIZED){
		printf("network driver not initialized.\n");
		return;
	}
	blockingWriteBB(TO_NET_BUFF, (void *) pd);
}
int nonblocking_send_packet(PacketDescriptor *pd){
	if(!INITIALIZED){
		printf("network driver not initialized.\n");
		return 0;
	}
	return nonblockingWriteBB(TO_NET_BUFF, (void *) pd);
}

/*
 * These represent requests for packets by the application threads
 * The nonblocking call returns 1 if a packet was found (and the first
 * argument set accordingly) or 0 if no packet was waiting.
 * The blocking call only returns when a packet has been received
 * for the indicated process, and the first arg points at it.
 */

void blocking_get_packet(PacketDescriptor **pd, PID pid){
	if(!INITIALIZED){
		printf("network driver not initialized.\n");
		return;
	}
	*pd = (PacketDescriptor *) blockingReadBB(TO_APP_BUFF[(unsigned int) pid]);
}
int  nonblocking_get_packet(PacketDescriptor **pd, PID pid){
	if(!INITIALIZED){
		printf("network driver not initialized.\n");
		return 0;
	}
	return nonblockingReadBB(TO_APP_BUFF[(unsigned int) pid], (void **) pd);
}

/* initializes data structures and starts internal threads.
 * Arguments:
 *   nd: the NetworkDevice that you must drive,
 *   mem_start, mem_length: some memory for PacketDescriptors
 *   fpds_ptr: You hand back a FreePacketDescriptorStore into
 *             which you have put the divided up memory
 */

void init_network_driver(NetworkDevice *nd, void *mem_start, unsigned long mem_length, FreePacketDescriptorStore **fpds_ptr){
	ND = nd;

	/* create Free Packet Descriptor Store */
	if ((*fpds_ptr = create_fpds()) == NULL){
		printf("Failed to create free packet descriptor store.\n");
		goto clean;
	}
	FPDS = *fpds_ptr;

	/* load FPDS with packet descriptors constructed from mem_start/mem_length */
	if((NUM_PD = create_free_packet_descriptors(*fpds_ptr, mem_start, mem_length)) == 0){
		printf("Failed to create free packet descriptors. NUM_PID: %d\n", NUM_PD);
		goto clean;
	}

	/* create any buffers required by your thread[s] */
	// size of the buffer should be strictly less than the number of pds
	if(NUM_PD < 3){
		printf("Failed to create enough free packet descriptors. NUM_PID: %d\n", NUM_PD);
		goto clean;
	}
	int to_net_size = NUM_PD / 2;

	//create buffer for apps to put into
	if((TO_NET_BUFF = createBB(to_net_size)) == NULL){
		printf("Failed to TO_NET_BUFF.\n");
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
		printf("Failed to create to_network pthread\n");
		goto clean;
	}

	//getting from network
	if(pthread_create(&from_network, NULL, get_from_network, NULL) != 0){
		printf("Failed to create from_network pthread\n");
		goto clean;
	}

	// send and get functions wont work until everything is initialized and INITIALIZED is set.
	INITIALIZED = 1;
	return;

	clean:
		//something has gone wrong and we will clean up before returning.
		printf("init_network_driver failed. Cleaning up\n");

		//clean threads. wait for threads to die before cleaning buffers
		DONE = 1;
		pthread_join(to_network, NULL);
		pthread_join(from_network, NULL);

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
		return;
}
