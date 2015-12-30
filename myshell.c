/*
 ============================================================================
 Name        : myshell.c
 Author      : 038064556 029983111
 Description : a unix shell written in c language
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>


#define MAX_COMMAND_LENGTH 256
#define MAX_COMMANDS 16
#define MAX_DIR_LENGTH 256
#define EXIT_COMMAND "exit"
#define EXIT_MESSAGE "You exit the shell, good bye!\n"
#define CHANGE_DIR_ERROR "Error: Could not change the directory to %s\n"
#define RUN_ERROR "Error: Could not start the program %s\n"


int exitFlag = 0;

/*
 *  Parses a command
 */
int parse_command(char* commands, char* allCommandsSeperate[MAX_COMMAND_LENGTH][MAX_COMMANDS]){
	char* singleCommand;
	char* singleWord;
	int counterOfCommands = 0;
	int counterOfWords = 0;
	char *allCommands[MAX_COMMAND_LENGTH * MAX_COMMANDS];

	// Separate all commands by the character '&'
	singleCommand = strtok(commands, "&");

	// Keep separating all commands to different cells in the allCommands array
	while (singleCommand != NULL){

		//inserts each command to the allCommands array
		allCommands[counterOfCommands] = singleCommand;
		counterOfCommands++;
		singleCommand = strtok(NULL, "&");
	}

	int i;
	for (i = 0; i < counterOfCommands; i++) {
		singleWord= strtok(allCommands[i], " ");
		while (singleWord != NULL){
			allCommandsSeperate[i][counterOfWords] = singleWord;
			counterOfWords++;
			singleWord= strtok(NULL, " ");
		}
		char* comm = allCommandsSeperate[i][counterOfWords - 1];
		// Get rid of the \n
		if (comm[strlen(comm) - 1] == '\n') {
			comm[strlen(comm) - 1] = '\0';
		}
		allCommandsSeperate[i][counterOfWords] = NULL;
		counterOfWords = 0;
	}

	return counterOfCommands;
}


/* executes the given command in the desired mode
 * parameters:
 * 			command - the command to execute
 * 			in_background - if the desired mode is background 1 else 0
 */
int execute_command(char* command[MAX_COMMAND_LENGTH], int in_background, char* dir){

	//char* allCommands[MAX_COMMANDS];
	pid_t pid;
	int status;
	char* comm = command[0];

	if (command[0] != NULL){

	    //parse_command(command, allCommandsSeperate);

	   //checks if the user typed exit and then closes the prompt
		if (!strcmp(command[0],"exit")){
			printf(EXIT_MESSAGE);
			exitFlag = 1;
			exit(0);
		}

		// Checks if the user typed cd
		if (!strcmp(command[0], "cd")){
			char* directory = command[1];
			if(directory[strlen(directory) - 1] == '\n') {
				directory[strlen(directory) - 1] = '\0';
			}
			chdir(directory);
			return 0;
		}
		pid = fork();
		if (pid == 0) {

			//It is a  child process (does not have a pid)
			if (execvp(command[0], command) != 0){
				printf(RUN_ERROR, command[0]);
				exit(0);
			} else {
				execvp(command[0], command);
				exit(1);
			}
		} else if (!in_background) {

			//It is a parent process
			waitpid(pid, &status, 0);
			return WEXITSTATUS(status);
		} else {
			return 0;
		}
	}
	return 1;
}


int main(int argc, char *argv[]){
	//deals with eclipse bug
	setvbuf (stdout, NULL, _IONBF, 0);

	// gets the current directory.
	char *dir;
	int i;
	int in_background;
	int number_of_commands = 0;
	char last_char;
	char command[MAX_COMMAND_LENGTH];
	char *allCommandsSeperate[MAX_COMMAND_LENGTH][MAX_COMMANDS];

	while (exitFlag == 0) {
		dir = getcwd(0, 0);
		printf("%s%s", dir, ">");
		fgets(command, MAX_COMMAND_LENGTH, stdin);


		//checks if the last char is & and return 1 else return 0
	    last_char = command[(strlen(command) - 2)];
	    if (last_char == '&'){
				in_background = 1;
	    } else {
				in_background = 0;
		}
	    number_of_commands = parse_command(command, allCommandsSeperate);
	    for (i = 0; i < number_of_commands; i++) {
	    	execute_command(allCommandsSeperate[i], in_background, dir);
		}
	}

	// prints the exit message and closes the shell
	printf("%s\n", EXIT_MESSAGE);
	return 0;

}
