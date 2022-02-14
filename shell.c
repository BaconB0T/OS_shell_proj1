/*
* Author:       Ethan Brown
* Course:       COMP 340, Operating Systems
* Date:         13 February 2022
* Description:  This file implements the 
*               Shell program
* Compile with: gcc -o shell shell.c
* Run with:     ./shell
*/
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/wait.h>

int shell_change_dir(char *dir_path) {
	//use chdir() system call to change the current directory
	return chdir(dir_path);
}

int shell_file_exists(char *file_path) {
	//use stat() sys call to check if a file exists
	char buff[1024];
	for(int i = 0; i < 1024; ++i) {
		printf("%s", buff);
	}
	printf("\n");
	struct stat *poopie;
	return stat(file_path, poopie);
}

int shell_find_file(char *file_name, char *file_path, char file_path_size) {
	//traverse PATH env var to find absolute path of a file/command
	char* path = strdup(getenv("PATH"));
	char* pathString = "";
	while(strlen(path) > 0 ) {
		pathString = strsep(&path, ";");
		strcat(pathString, file_name);
		if(shell_file_exists(pathString) == 0) {
			file_path = strdup(pathString);
			return 0;
		}
	}
	return -1;
}

int shell_execute(char *file_path, char **argv) {
	//execute the file with the command line arguments
	// use the fork() and exec() sys call
	printf("Before Fork");
	pid_t pid = fork();
	printf("After fork: %d", pid);
	if(pid == 0) {
		//child
		int status;
		if((status = execv(file_path, argv)) == -1);
			exit(status);
	} else {
		//parent
		wait(NULL);
	}
	return 0;
}

int main(int argc, char *argv[]) {
	///run the shell
	char buf[1024];
	char cwd[1024];

	int exit = 0;
	getcwd(cwd, 1023);
	while(!exit) {
		//display prompt and wait for user input
		printf("%s$ ", cwd);
		fscanf(stdin, "%s", buf);
		puts(buf);

		//2. filter out whitespace command 

    //3. if command line contains non-whitespace characters

		//if the specified command is “exit”, terminate the program taking care to release any allocated resources.
		if(strcmp(buf, "exit")) {
			exit = 1;
		}
    //if the specified command is “cd”

    //  change the current working directory to the specified directory path using shell_change_dir()
    // if the command is specified using an absolute path, exists in the user’s PATH or exists in the current folder
			// execute the command using shell_execute()
		// else
			// report an error message


	}
}
