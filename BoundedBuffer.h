#ifndef BOUNDEDBUFFER_H_
#define BOUNDEDBUFFER_H_

#include <pthread.h>
#include <semaphore.h>

/*
 * Interface file for a thread-safe synchronized bounded buffer
 */

/*
 * A struct of a bounded buffer
 */
typedef struct {
	char **buffer;				// The buffer data (char pointers)
	int size;					// Current size of the buffer
	int capacity;				// Capacity of the buffer
	int head;					// Index of the head of the buffer
	int tail;					// Index of the tail of the buffer
	sem_t *avail, *empty, *cs;			// Semaphores
	int finished;				// Finished flag
} BoundedBuffer;

/*
 * Initializes the buffer with the specified capacity.
 * This function should allocate the buffer, initialize its properties
 * and also initialize its mutex and condition variables.
 * It should set its finished flag to 0.
 */
void bounded_buffer_init(BoundedBuffer *buff, int capacity);

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
int bounded_buffer_enqueue(BoundedBuffer *buff, char *data);

/*
 * Dequeues a string (char pointer) from the buffer.
 * This function should remove the head element of the buffer and return it.
 * If the buffer is empty, it should wait until it is not empty, or until it has finished.
 * If the buffer has finished (either after waiting or even before), it should
 * simply return NULL.
 * If the dequeue operation was successful, it should signal that the buffer is not full.
 * This function should be synchronized on the buffer's mutex!
 */
char *bounded_buffer_dequeue(BoundedBuffer *buff);

/*
 * Sets the buffer as finished.
 * This function sets the finished flag to 1 and then wakes up all threads that are
 * waiting on the condition variables of this buffer.
 * This function should be synchronized on the buffer's mutex!
 */
void bounded_buffer_finish(BoundedBuffer *buff);

/*
 * Frees the buffer memory and destroys mutex and condition variables.
 */
void bounded_buffer_destroy(BoundedBuffer *buff);

#endif /* BOUNDEDBUFFER_H_ */
