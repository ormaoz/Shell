/*
 * Copier.h
 *
 *  Created on: Apr 7, 2011
 *      Author: yotam
 */

#ifndef COPIER_H_
#define COPIER_H_

#define CMD_EXIT "exit\n"

/*
 * Listener thread starting point.
 * Creates a named pipe (the name should be supplied by the main function) and waits for
 * a connection on it. Once a connection has been received, reads the data from it and
 * parses the file names out of the data buffer. Each file name is copied to a new string
 * and then enqueued to the files queue.
 * If the enqueue operation fails (returns 0), the copier is application to exit and therefore
 * the Listener thread should stop. Before stopping, it should remove the pipe file and
 * free the memory of the filename it failed to enqueue.
 */
void *run_listener(void *param);

/*
 * A file copy function. Copies file from src to dest using a buffer.
 */
void copy_file(char *src, char *dest);

/*
 * Copier thread starting point.
 * The copier reads file names from the files queue, one by one, and copies them to the
 * destination directory as given to the main function. Then it should free the memory of
 * the dequeued file name string (it was allocated by the Listener thread).
 * If the dequeue operation fails (returns NULL), it means that the application is trying
 * to exit and therefore the thread should simply terminate.
 */
void *run_copier(void *param);

/*
 * Main function.
 * Reads command line arguments in the format:
 * 		./Copier pipe_name destination_dir
 * Where pipe_name is the name of FIFO pipe that the Listener should create and
 * destination_dir is the path to the destination directory that the copier should
 * copy files into.
 * This function should create the files queue and prepare the parameters to the Listener and
 * Copier threads. Then, it should create these threads.
 * After threads are created, this function should control them as follows:
 * it should read input from user, and if the input line is EXIT_CMD (defined above), it should
 * set the files queue as "finished". This should make the threads terminate (possibly only
 * when the next connection is received).
 * At the end the function should join the threads and exit.
 */
int main(int argc, char *argv[]);

char *get_file_name(char *path);
#endif /* COPIER_H_ */
