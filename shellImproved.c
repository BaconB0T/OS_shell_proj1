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

const int 	BUF_SIZE = 1023;
const int 	MAX_NUM_CMD = 15;
const int 	ENV_PATH_L = 32760;
const int 	FILENAME_MAX_L = 255;
const int 	PATH_MAX_L = (4096 - 255);
const char	WHITESPACE[] = " \f\n\r\t\v";

int			num_cmds;

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
	// FINALLY! Thank you Dr. Zhange!
	// this shouldn't have been a pointer at all, and it should
	// have been initialized
	struct stat statStruct = {0};
	// now this will find the file if it exists and return 0
	// I'm not sure why it was returning 0 previously when I
	// was using an unitialized pointer, but that was
	// definitely the cause

	int status = stat(file_path, &statStruct);
	//printf("status=%d\n", status);
	return status;
}

int shell_find_file(char *file_name, char *file_path, char file_path_size)
{
	//printf("Checking PATH\n");
	const char *pathVar = getenv("PATH");
	char *mutablePath = strdup(pathVar);
	char *miniPath = strsep(&mutablePath, ":");
	while(miniPath != NULL) {
		miniPath = strdup(miniPath);
		strcat(miniPath, "/");
		strcat(miniPath, file_name);
		//printf("Checking path: %s\n", miniPath);
		if(shell_file_exists(miniPath) == 0) {
			//printf("file %s found! returning...\n", miniPath);
			strcat(file_path, miniPath);
			//return success
			return 0;
		}
		//printf("File %s not found.\n", miniPath);
		miniPath = strsep(&mutablePath, ":");
	}
	// return fail
	return -1;
}

int shell_execute(char *file_path, char *argv[])
{
	// printf("file_path=%s, *file_path=%p\n",file_path, file_path);
	// for(int i = 0; i < num_cmds; ++i) {
	// 	printf("argv[%d]=%s, %p\n", i, argv[i], argv[i]);
	// }
	// execute the file with the command line arguments
	//  use the fork() and exec() sys call
	//printf("file_path execute: file_path= %s\n", file_path);
	//printf("Before Fork\n");
	pid_t pid = fork();
	//printf("After fork: %d\n", pid);
	if (pid == 0)
	{
		// child
		int status;
		//char* tmp_argv[] = {"shell", NULL};
		if(execv(file_path, argv) == -1) {
		//if ((status = execv(file_path, argv)) == -1)
			printf("Failed\n");
			_exit(EXIT_FAILURE);
		}
	}
	else
	{
		// parent
		wait(NULL);
	}
	// for(int i = 0; i < num_cmds; ++i) {
	// 	printf("argv[%d]=%s, %p\n", i, argv[i], argv[i]);
	// }
	return 0;
}

int main(int argc, char *argv[])
{
	// run the shell
	int exit = 0;
	// input buffer
	char *buf = (char *)calloc(PATH_MAX_L + 1, sizeof(char));
	// current working directory
	char *cwd = (char *)calloc((PATH_MAX_L + 1), sizeof(char));
	// move this somewhere better. nulling one of the pointers causes a memory leak
	// array of arguments for command from buffer. array of MAX_NUM_CMD strings

	char **args = (char **)calloc(MAX_NUM_CMD, sizeof(char *));
	// for (int i = 0; i < MAX_NUM_CMD; ++i)
	// {
	// 	args[i] = (char *)calloc((PATH_MAX_L + 1), sizeof(char));
	// }

	while (!exit)
	{
		getcwd(cwd, PATH_MAX_L);
		// display prompt and wait for user input
		printf("%s$ ", cwd);
		// get input
		fgets(buf, PATH_MAX_L, stdin);
		int lenSpace = strspn(buf, WHITESPACE);
		//int lenChars = strcspn(buf, WHITESPACE);
		// printf("lenChars: %d\n", lenChars);

		// 2. filter out whitespace command
		if (lenSpace == strlen(buf))
		{
			continue;
		}
		// not whitespace, remove trailing newline
		buf[strlen(buf) - 1] = '\0';
		// strtok(buf, " "); will get first thingy.
		// strncpy(cmd, strtok(buf, " "), 100);
		// length = lenChars hopefully, unteseted

		args[0] = (char*)calloc(PATH_MAX_L, sizeof(char));
		strcpy(args[0], strtok(buf, " "));
		// print cmd
		// args[0] = strtok(buf, " ");
		// printf("args[0]: %s,%p\n", args[0],args[0]);

		num_cmds=1;
		char* tmp;
		while ((tmp = strtok(NULL, " ")) != NULL && num_cmds < MAX_NUM_CMD)
		{
			args[num_cmds] = (char*)calloc(PATH_MAX_L, sizeof(char));
			strcpy(args[num_cmds], tmp);
			// printf("args[%d]: %s,%p\n", num_cmds,args[num_cmds],args[num_cmds]);
			++num_cmds;
		}
		if(num_cmds == MAX_NUM_CMD) {
			// strcat(args[x-1], NULL);
			args[num_cmds-1] = NULL;
		} else {
			// strcat(args[x], NULL);
			args[num_cmds] = NULL;
		}
		//printf("args: ");
		//for (int i = 0; i < x; ++i)
		//	printf("args[%d]=%s,", i,args[i]);

		//printf("\n");
		// 3. if command line contains non-whitespace characters
		//  buf contains whole input line (up to 1023+1 chars). buf+lenSpace(char) equals
		//  the index of the first non-whitespace character. Copy 1023-lenSpace bytes back
		//  into buffer
		// THIS ISN'T NECESSARY
		// strncpy(buf, (buf + (sizeof(char) * lenSpace)), PATH_MAX_L - lenSpace);
		// if the specified command is ???exit???, terminate the program taking care to release
		// any allocated resources.
		// printf("%d\n", shell_file_exists(args[0]));

		if (strcmp(args[0], "exit") == 0)
		{
			exit = 1;
		} // change the current working directory to the specified directory path using shell_change_dir()
		else if (strcmp(args[0], "cd") == 0)
		{
			//if the directory is in quotes, that means there's a path with a space in it.
			if(strchr(args[1], '\"') != NULL) {
				//so long as the current token doesn't contain quotes
				int i = 2;
				while(strchr(args[i],'\"') == NULL && i < num_cmds) {
					//concatenate the token to the directory.
					strcat(args[1], " ");
					strcat(args[1], args[i]);
					++i;
					printf("current dir: %s\n", args[1]);
				}
				//args[i] does have a string, concat it to args[1]
				if(i < num_cmds) {
					strcat(args[1], " ");
					strcat(args[1],args[i]);
				}
			}
			//currently testing directories with spaces in their names
			printf("change directories to %s\n", args[1]);
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
			else if (shell_find_file(args[0], file_path, (PATH_MAX_L+FILENAME_MAX+1)) == 0)
			{
				strcpy(args[0], file_path);
				// args[0] = file_path;
				shell_execute(file_path, args);
			} // else report an error message
			else
			{
				printf("No command matching \"%s\"\n", buf);
			}
		}
	}

	for (int i = (num_cmds-1); i > 0; --i)
	{
		//doesn't print for some reason.
		// printf("free args[%d]=%s, %p\n",i,args[i], args[i]);
		if(args[i] != NULL)
			free(args[i]);
	}
	// printf("free(args)\n");
	free(args);
	// printf("free(cwd)\n");
	free(cwd);
	// printf("free(buf)\n");
	free(buf);
	// printf("Goodbye.\n");
}
