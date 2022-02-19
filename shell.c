/*
 * Author:      Ethan Brown
 * Course:      COMP 340, Operating Systems
 * Date:        13 February 2022
 * Description: This file implements the
 *              Shell program
 * Compile:	gcc -o shell shell.c
 * Run with:    ./shell
 * IMPORTANT:	For some reason, when shell_finds_file
		returns on my local machine and on the
		VM there is a segmentation fault. When
		Dr. Zhang ran the same code on his computer
		it worked, no problems. I cannot understand
		why this is the case, but I cannot test it.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

const int BUF_SIZE = 1023;
const int MAX_NUM_CMD = 15;
const int ENV_PATH_L = 32760;
const int FILENAME_MAX_L = 255;
const int PATH_MAX_L = (4096 - 255);
const char WHITESPACE[] = " \f\n\r\t\v";

int shell_change_dir(char *dir_path)
{
	// use chdir() system call to change the current directory
	int status = chdir(dir_path);
	if (status != 0)
	{
		printf("Error: %s\n", strerror(status));
	}
	return status;
}

int shell_file_exists(char *file_path)
{
	// use stat() sys call to check if a file exists
	struct stat *statStruct;
	int status = stat(file_path, statStruct);
	return status;
}

int shell_find_file(char *file_name, char *file_path, char file_path_size)
{
	const char *pathVar = getenv("PATH");
	char *mutablePath = strdup(pathVar);
	char *miniPath = strsep(&mutablePath, ":");
	while(miniPath != NULL) {
		miniPath = strdup(miniPath);
		strcat(miniPath, "/");
		strcat(miniPath, file_name);
		if(shell_file_exists(miniPath) == 0) {
			strcat(file_path, miniPath);
			sleep(10);
			return 0;
		}
		miniPath = strsep(&mutablePath, ":");
	}
	return -1;
}

int shell_execute(char *file_path, char **argv)
{
	// execute the file with the command line arguments
	//  use the fork() and exec() sys call
	pid_t pid = fork();
	if (pid == 0)
	{
		// child
		//execv shouldn't return at all if it succeeds, but it returns -1 on fail
		if(execv(file_path, argv) == -1) {
			printf("Failed\n");
			_exit(EXIT_FAILURE);
		}
	}
	else
	{
		// parent
		wait(NULL);
	}
	return 0;
}

int main(int argc, char *argv[])
{
	// run the shell
	// input buffer
	char *buf = (char *)calloc(PATH_MAX_L + 1, sizeof(char));
	// current working directory
	char *cwd = (char *)calloc((PATH_MAX_L + 1), sizeof(char));
	// array of arguments for command from buffer. array of MAX_NUM_CMD strings
	char **args = (char **)calloc(MAX_NUM_CMD, sizeof(char *));
	int exit = 0;

	for (int i = 0; i < MAX_NUM_CMD; ++i)
	{
		args[i] = (char *)calloc((PATH_MAX_L + 1), sizeof(char));
	}

	while (!exit)
	{
		getcwd(cwd, PATH_MAX_L);
		// display prompt and wait for user input
		printf("%s$ ", cwd);
		// get input
		fgets(buf, PATH_MAX_L, stdin);
		int lenSpace = strspn(buf, WHITESPACE);
		int lenChars = strcspn(buf, WHITESPACE);

		// 2. filter out whitespace command
		if (lenSpace == strlen(buf))
		{
			continue;
		}
		// not whitespace, remove trailing newline
		buf[strlen(buf) - 1] = '\0';
		args[0] = strtok(buf, " ");
		int x = 1;
		while ((args[x] = strtok(NULL, " ")) != NULL && x < MAX_NUM_CMD)
		{
			++x;
		}
		if (strcmp(args[0], "exit") == 0)
		{
			exit = 1;
		} // change the current working directory to the specified directory path using shell_change_dir()
		else if (strcmp(args[0], "cd") == 0)
		{
			shell_change_dir(args[1]);
		}
		else
		{
			//shell_find_file returns a path to here
			char *file_path = (char *)calloc((PATH_MAX_L + FILENAME_MAX + 1), sizeof(char));
			//if local or absolute
			if (shell_file_exists(args[0]) == 0)
			{
				shell_execute(args[0], args);
			} // command exists in the PATH.
			//else if (shell_find_file(args[0], file_path, (PATH_MAX_L+FILENAME_MAX+1)) == 0)
			//{
			//	shell_execute(file_path, args);
			//}
			else
			{ // else report an error message
				printf("No command matching \"%s\"\n", buf);
			}
		}
	}

	for (int i = 1; i < MAX_NUM_CMD; ++i)
	{
		free(args[i]);
	}
	free(args);
	free(cwd);
	free(buf);
	printf("Goodbye.\n");
}
