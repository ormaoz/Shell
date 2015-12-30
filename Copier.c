/*
 ============================================================================
 Name        : Copier.c
 Author      : 038064556 029983111
 ============================================================================
 */

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include "Copier.h"
#include "BoundedBuffer.h"

#define READ_BUFF_SIZE 4096
#define STDIN_READ_BUFF_SIZE 80
#define MAX_PATH_LENGTH 1024
#define COPY_BUFFER_SIZE 1024
#define FILE_QUEUE_SIZE 10

#define FILE_ACCESS_RW 0666
#define PATH_SEPARATOR "/"
#define PATH_SEPARATOR_CHAR '/'

typedef struct {
BoundedBuffer *buff;
char *dest;
int force;
} CopierData;

typedef struct {
BoundedBuffer *buff;
char *pipe;
} ListenerData;


/*
 * A file copy function. Copies file from src to dest using a buffer.
 */
void copy_file(char *src, char *dest){
    unsigned char buffer[COPY_BUFFER_SIZE];
    FILE *srcFile, *destFile;
    size_t readElements = 0;
    srcFile = fopen(src,"r");
    if (srcFile == NULL) {
        printf("Error opening file: %s\n", src);
        return;
    }

    destFile = fopen(dest,"w");
    if(destFile == NULL) {
        printf("Error opening file: %s\n", dest);
        return;
    }

    // Copies the files while using a buffer from srcFile to destFile
    while ((readElements = fread(buffer, sizeof(char), COPY_BUFFER_SIZE, srcFile)) != 0){
        fwrite(buffer, sizeof(char), readElements,destFile);
    }

    fclose(srcFile);
    fclose(destFile);
}


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
void *run_listener(void *param) {
    ListenerData *data;
    BoundedBuffer *buff;
    int fd, num, len, start, status;
    char filename[READ_BUFF_SIZE];
    char *fn_ptr;
    char *copy;

    // Get the pointer to the parameters struct
    data = (ListenerData*)param;

    //initiates the buffer that will contain the name of the files
    buff = data->buff;

    // Create the pipe
    mknod(data->pipe, S_IFIFO | FILE_ACCESS_RW, 0);

    while (1) {

    // Wait for a connection on the pipe
    fd = open(data->pipe, O_RDONLY);

    // Read data from pipe
    while ((num = read(fd, filename, READ_BUFF_SIZE)) > 0) {

    // Parse data into file names and enqueue them to buff
    fn_ptr = strtok(filename, "\n");

       // Keep separating all the names
       while (fn_ptr != NULL){
            len = strlen(fn_ptr);
            //inserts each name to the queue
            copy = malloc(sizeof(char)*len);
            strcpy(copy, fn_ptr);
            if (bounded_buffer_enqueue(buff, copy) == 0){
                free(copy);
                remove(data->pipe);
                return NULL;
            }
            fn_ptr = strtok(NULL, "\n");
           }
    }
    close(fd);
    }
}

/*
 * Copier thread starting point.
 * The copier reads file names from the files queue, one by one, and copies them to the
 * destination directory as given to the main function. Then it should free the memory of
 * the dequeued file name string (it was allocated by the Listener thread).
 * If the dequeue operation fails (returns NULL), it means that the application is trying
 * to exit and therefore the thread should simply terminate.
 */
void *run_copier(void *param){
    CopierData *data;
    char *dequeuedName, *nameWithoutPath;
    BoundedBuffer *buff;
    char destination[MAX_PATH_LENGTH];

    // Get the pointer to the parameters struct
    data = (CopierData*)param;
    buff = data->buff;

    // Dequeue the queue until it's empty and copy the files in it
    while ((dequeuedName = bounded_buffer_dequeue(buff)) != NULL){
        nameWithoutPath = get_file_name(dequeuedName);
        sprintf(destination,"%s%s", data->dest, nameWithoutPath);
        if(data->force == 0 && file_exists(destination)){
        	printf("The file: %s you are trying to copy already exists and you didn't use -f\n", destination);
        } else {
        	copy_file(dequeuedName, destination);
        }
        free(dequeuedName);
    }
    return NULL;
}

/*
 * Returns the name of a given file without the directory path.
 */
char *get_file_name(char *path) {
    int len;
    int i;

    len = strlen(path);

    for (i = len - 1; i >= 0; i--) {
        if (path[i] == PATH_SEPARATOR_CHAR) {
        return &(path[i + 1]);
        }
    }
    return path;
}

/*Helper method that checks if file already exists or not*/
int file_exists (char *filename)
{
  struct stat   buffer;
  return (stat (filename, &buffer) == 0);
}

/*
 * Main function.
 * Reads command line arguments in the format:
 * ./Copier pipe_name destination_dir
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

int main(int argc, char *argv[]){
    // Deals with eclipse bug
    setvbuf (stdout, NULL, _IONBF, 0);

    char *pipe_file_name, *destination_path;

    // Creates structs
    ListenerData lisData;
    CopierData copData;

    BoundedBuffer buff;
    pthread_t listener, copier;
    char inputCommand[STDIN_READ_BUFF_SIZE];

    // Checks validation of the typed arguments
    if (argc == 3){
        pipe_file_name = argv[1];
        destination_path = argv[2];
    } else if (argc == 4) {
        if (!strcmp(argv[3], "-f")){
        	copData.force = 1;
            pipe_file_name = argv[1];
            destination_path = argv[2];
        } else {
            printf("Usage: ./copier pipe_file_name destination_path [-f]\n");
            return 1;
        }
    } else {
        printf("Usage: ./copier pipe_file_name destination_path [-f]\n");
        return 1;
    }

    // Initiates the file queue
    bounded_buffer_init(&buff, FILE_QUEUE_SIZE);

    int exitFlag = 0;

    lisData.buff = &buff;
    copData.buff = &buff;
    lisData.pipe = pipe_file_name;
    copData.dest = destination_path;

    // Start threads
    pthread_create(&listener, NULL, &run_listener, (void*)(&lisData));
    pthread_create(&copier, NULL, &run_copier, (void*)(&copData));

    while (exitFlag == 0) {
        fgets(inputCommand, STDIN_READ_BUFF_SIZE, stdin);
        if (!strcmp(inputCommand,CMD_EXIT)){
            exitFlag = 1;
            bounded_buffer_finish(&buff);
        }
    }


    // Join threads
    pthread_join(listener, NULL);
    pthread_join(copier, NULL);

    // Free the allocated resources
    bounded_buffer_destroy(&buff);
    printf("You exit the copier program\n");
    return 0;
}
