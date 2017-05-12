/*
 * BoundedBuffer.c
 *
 * Implementation file for a bounded buffer implemented using pthread
 * facilities to allow it to be driven by concurrent threads.
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

#include "BoundedBuffer.h"
#include <pthread.h>
#include <stdlib.h>

typedef int	Boolean;
#define False	(0)
#define True	(!False)

struct bounded_buffer {
    pthread_mutex_t lock;      /* protects buffer from multiple writers */
    pthread_cond_t non_empty;  /* for consumer to await more stuff */
    pthread_cond_t non_full;   /* for producer to await space for stuff */

    int maxlength;      /* size of buffer */
    int readP;          /* slot before one to read from (if data exists) */
    int writeP;         /* slot before one to write to (if space exists) */
    Boolean emptyF;     /* flag true => buffer is empty */

    void   **buffer;    /* storage for the actual stuff held */
};
/* NB: if readP==writeP, buffer is empty or full, use emptyF to show which */

BoundedBuffer *createBB(int size) {
    BoundedBuffer *newBB = (BoundedBuffer *)malloc(sizeof(BoundedBuffer));
    if (newBB == NULL)
        return NULL;
    newBB->buffer = (void **) malloc(size*sizeof(void *));
    if (newBB->buffer == NULL) {
        free(newBB);
        return NULL;
    }

    newBB->maxlength = size;
    newBB->writeP = newBB->readP = size-1;
    newBB->emptyF = True;

    pthread_mutex_init(&newBB->lock, NULL);
    pthread_cond_init(&newBB->non_full, NULL);
    pthread_cond_init(&newBB->non_empty, NULL);

    return newBB;
}

void destroyBB(BoundedBuffer *bb) {
    free(bb->buffer);
    free(bb);
}

void blockingWriteBB(BoundedBuffer *bb, void *value) {

    pthread_mutex_lock(&bb->lock);

    /* await space to store the value in */
    while ((bb->writeP==bb->readP) && (!bb->emptyF))
        pthread_cond_wait(&bb->non_full, &bb->lock);

    /* advance the write index, cycling if necessary, and store value */
    bb->buffer[bb->writeP = (bb->writeP + 1) % bb->maxlength] = value;

    /* the buffer definitely isn't empty at this moment */
    bb->emptyF = False;
    pthread_cond_signal(&bb->non_empty);

    pthread_mutex_unlock(&bb->lock);
}

void *blockingReadBB(BoundedBuffer *bb) {
    void *value;

    pthread_mutex_lock(&bb->lock);

    /* await a value to hand to our caller */
    while (bb->emptyF)
        pthread_cond_wait(&bb->non_empty, &bb->lock);

    /* advance the read index, cycling if necessary, and retrieve value */
    value = bb->buffer[bb->readP = (bb->readP + 1) % bb->maxlength];

    /* the buffer definitely isn't full at this moment */
    bb->emptyF = (bb->readP == bb->writeP);
    pthread_cond_signal(&bb->non_full);

    pthread_mutex_unlock(&bb->lock);

    return value;
}
