#ifndef __BOUNDED_BUFFER_HDR
#define __BOUNDED_BUFFER_HDR

/*
 * BoundedBuffer.h
 *
 * Header file for a bounded buffer implemented using pthread
 * facilities to allow it to be driven by concurrent threads.
 * In particular, each bounded buffer uses mutexes and condition variables
 * to guarantee thread-safety.
 *
 * Uses standard tricks to keep it very generic.
 * Uses heap allocated data structures.
 *
 * Author: Peter Dickman
 * Revised by Joe Sventek
 *
 * Created: 7-3-2000
 * Edited:  28-2-2001
 * Edited:  4 May 2015
 *
 * Version: 1.2
 *
 */

/* opaque data structure representing BoundedBuffer instance */
typedef struct bounded_buffer BoundedBuffer;

/* create a bounded buffer to hold `size' items; return NULL if error */
BoundedBuffer *createBB(int size);

/* destroy the bounded buffer, returning all heap-allocated memory */
void destroyBB(BoundedBuffer *bb);

/*
 * write methods on BB; blocking call causes calling thread to block
 * until there is room in the BB; nonblocking call returns 1 if item
 * successfully stored in the BB, 0 otherwise
 */
void blockingWriteBB(BoundedBuffer *bb, void *item);
int nonblockingWriteBB(BoundedBuffer *bb, void *item);

/*
 * read methods on BB; blocking call causes calling thread to block
 * until there is an item in the BB; nonblocking call returns 1 if item
 * was succesfully retrieved from the BB, 0 otherwise
 */
void *blockingReadBB(BoundedBuffer *bb);
int nonblockingReadBB(BoundedBuffer *bb, void **item);

#endif /* __BOUNDED_BUFFER_HDR */
