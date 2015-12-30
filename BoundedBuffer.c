/*
 ============================================================================
 Name        : BoundedBuffer.c
 Author      : 038064556 029983111
 ============================================================================
 */

#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include "BoundedBuffer.h"


/*
 * Initializes the buffer with the specified capacity.
 * This function should allocate the buffer, initialize its properties
 * and also initialize its mutex and condition variables.
 * It should set its finished flag to 0.
 */
void bounded_buffer_init(BoundedBuffer *buff, int capacity) {

    // The buffer data (char pointers)
    buff->buffer = (char**)malloc(sizeof(char*) * capacity);

    // Current size of the buffer
    buff->size = 0;

    // Capacity of the buffer
    buff->capacity = capacity;
    // Index of the head of the buffer
    buff->head = 0;
    // Index of the tail of the buffer
    buff->tail = 0;

    // Semaphores
    buff->avail = sem_open("avail", O_CREAT, 0666, 1);
    if (buff->avail == SEM_FAILED) {
        printf("sem_open(avail) error");
        exit(1);
    }
    buff->empty = sem_open("empty", O_CREAT, 0666, 0);
    if (buff->empty == SEM_FAILED) {
    	printf("sem_open(empty) error");
        exit(1);
    }
    buff->cs = sem_open("cs", O_CREAT, 0666, 1);
    if (buff->cs == SEM_FAILED) {
    	printf("sem_open(cs) error");
        exit(1);
    }

    buff->finished = 0;
}

/*
 * Enqueue a string (char pointer) to the buffer.
 * This function should add an element to the buffer. If the buffer is full,
 * it should wait until it is not full, or until it has finished.
 * If the buffer has finished (either after waiting or even before), it should
 * simply return 0.
 * If the enqueue operation was successful, it should return 1. In this case it
 * should also signal that the buffer is not empty.
 * This function should be synchronized on the buffer's mutex!
 */
int bounded_buffer_enqueue(BoundedBuffer *buff, char *data) {

    // First, wait for the buffer to become available
    while ((buff->size == buff->capacity) && !buff->finished) {
        sem_wait(buff->avail);
    }

    // In case the buffer isn't finished
    if (!buff->finished) {
        // Before entering an item to the queue, protect the buffer from other threads who might try to use it
        sem_wait(buff->cs);

        // Put value
        buff->buffer[buff->tail] = data;
        buff->size++;
        buff->tail = ((buff->tail + 1) % buff->capacity);

        // Now exit the critical section by releasing the mutex
        sem_post(buff->cs);

        // Now signal that the buffer has an item available
        sem_post(buff->empty);
        return 1;
    } else {
        return 0;
    }
}

/*
 * Dequeues a string (char pointer) from the buffer.
 * This function should remove the head element of the buffer and return it.
 * If the buffer is empty, it should wait until it is not empty, or until it has finished.
 * If the buffer has finished (either after waiting or even before), it should
 * simply return NULL.
 * If the dequeue operation was successful, it should signal that the buffer is not full.
 * This function should be synchronized on the buffer's mutex!
 */
char *bounded_buffer_dequeue(BoundedBuffer *buff) {
    char *item;
    while ((buff->size == 0) && !buff->finished) {

        // If the buffer is empty, it should wait until it is not empty, or until it has finished.
        sem_wait(buff->empty);
    }
    if (!buff->finished) {

        // Then, protect the access to the buffer using a critical section around it
        sem_wait(buff->cs);

        item = buff->buffer[buff->head];
        buff->head = (buff->head + 1) % buff->capacity;
        buff->size--;

        // Now exit the critical section by releasing the mutex
        sem_post(buff->cs);

        // Signal that the buffer is not full.
        sem_post(buff->avail);

        return item;
    } else {
        return NULL;
    }
}

/*
 * Sets the buffer as finished.
 * This function sets the finished flag to 1 and then wakes up all threads that are
 * waiting on the condition variables of this buffer.
 * This function should be synchronized on the buffer's mutex!
 */
void bounded_buffer_finish(BoundedBuffer *buff){
	buff->finished = 1;
	sem_post(buff->empty);
	sem_post(buff->avail);
}
/*
 * Frees the buffer memory and destroys mutex and condition variables.
 */
void bounded_buffer_destroy(BoundedBuffer *buff){
	free(buff->buffer);
	sem_close(buff->avail);
	sem_close(buff->empty);
	sem_close(buff->cs);
}
