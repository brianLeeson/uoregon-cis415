#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "BoundedBuffer.h"

#define BUFFER_SIZE 10
#define UNUSED __attribute__ ((unused))

void *producer(void *args) {
    BoundedBuffer *bb = (BoundedBuffer *)args;
    char buf[2048];

    while (fgets(buf, sizeof buf, stdin) != NULL) {
        char *s = strdup(buf);
        blockingWriteBB(bb, s);
    }
    blockingWriteBB(bb, NULL);
    return NULL;
}

void *consumer(void *args) {
    BoundedBuffer *bb = (BoundedBuffer *)args;

    for (;;) {
        char *s = blockingReadBB(bb);
        if (s == NULL)
            break;
        fputs(s, stdout);
        free(s);
    }
    return NULL;
}

int main(UNUSED int argc, UNUSED char *argv[]) {
    BoundedBuffer *bb = createBB(BUFFER_SIZE);
    pthread_t prod, cons;

    if (pthread_create(&cons, NULL, consumer, (void *)bb)) {
        fprintf(stderr, "Unable to create consumer thread\n");
        goto cleanup;
    }
    if (pthread_create(&prod, NULL, producer, (void *)bb)) {
        fprintf(stderr, "Unable to create producer thread\n");
        pthread_cancel(cons);
        goto cleanup;
    }
    pthread_join(prod, NULL);	/* producer will stop when EOF on stdin */
    pthread_join(cons, NULL);	/* consumer will stop when it sees NULL */

cleanup:
    if (bb != NULL)
        destroyBB(bb);
    return 0;
}
