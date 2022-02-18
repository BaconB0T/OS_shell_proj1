/*
 * Author:       Ethan Brown
 * Course:       COMP 340, Operating Systems
 * Date:         13 February 2022
 * Description:  This file implements the
 *               Shell program
 * Compile with: gcc -o shell shell.c
 * Run with:     ./shell
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
	// prints 0 0 0...?
	printf("let's go %i %i %i\n", dir_path[0], dir_path[1], dir_path[2]);
	// int status = chdir("..");
	if (status != 0)
	{
		printf("Error: %s\n", strerror(status));
	}
	return status;
}

int shell_file_exists(char *file_path)
{
	// use stat() sys call to check if a file exists
	printf("Does file exist?\n");
	printf("filepath: %s\n", file_path);
	struct stat *statStruct;
	int status = stat(file_path, statStruct);
	printf("status: %d\n", status);
	return status;
}

int shell_find_file(char *file_name, char *file_path, char file_path_size)
{
	// traverse PATH env var to find absolute path of a file/command
	printf("find file\n");
	char *envPath = (char *)calloc((ENV_PATH_L + 1), sizeof(char));
	// this should return a new copy of the path environment variable and
	// put it in envPath
	strncpy(envPath, getenv("PATH"), ENV_PATH_L+1);

	// strcat(envPath, "\0");

	printf("%s\n", envPath);
	char *pathString = (char *)calloc(PATH_MAX_L+1, sizeof(char));
	while (envPath != NULL)
	{
		// env vars are colon seperated
		// maximum length the path can be is PATH_MAX_L minus the length of the file_name.
		pathString = strndup(strsep(&envPath, ":"), PATH_MAX_L - strlen(file_name));
		// something like "/home/user/bin" + "/" + "filename" = "/home/user/bin/filename"
		// If should be enough space to concat "/" to pathString
		strcat(pathString, "/");
		// If there's enough space (may not be), concat file_name to pathString
		if(strlen(pathString)+strlen(file_name) < PATH_MAX_L) {
			strcat(pathString, file_name);
			if(shell_file_exists(pathString) == 0)
			{
				file_path = strndup(pathString, PATH_MAX_L+1);
				return 0;
			}
		} else {
			// else there isn't enough space, so the file
			// can't exist in that directory
			continue;
		}
	}
	// no successful thing found, return -1
	return -1;
}

int shell_execute(char *file_path, char **argv)
{
	// execute the file with the command line arguments
	//  use the fork() and exec() sys call
	printf("file_path execute: %s\n", file_path);
	printf("Before Fork\n");
	pid_t pid = fork();
	printf("After fork: %d\n", pid);
	if (pid == 0)
	{
		// child
		int status;
		char* tmp_argv[] = {"shell", NULL};
		if(execv("/home/comp340/Documents/Ethan/project1/", tmp_argv) == -1) {
		//if ((status = execv(file_path, argv)) == -1)
			printf("Failed\n");
			exit(-1);
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
	// command from buffer
	// char *cmd = (char*)calloc(, sizeof(char));
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
		// print cmd
		args[0] = strtok(buf, " ");
		printf("args[0]: %s\n", args[0]);
		
		// print cmd by char
		for(int i = 0; i< strlen(args[0]); ++i) {
			printf("%c ", args[0][i]);
		}
		printf("\n");
		
		// print cmd chars as ints
		for(int i = 0; i< strlen(args[0]); ++i) {
			printf("%i ", args[0][i]);
		}
		printf("\n");
		
		int x = 1;
		while ((args[x] = strtok(NULL, " ")) != NULL && x < MAX_NUM_CMD)
		{
			++x;
		}
		printf("args: ");
		for (int i = 0; i < x; ++i)
		{
			printf("args[%d]=%s,", i,args[i]);
		}
		printf("\n");
		// 3. if command line contains non-whitespace characters
		//  buf contains whole input line (up to 1023+1 chars). buf+lenSpace(char) equals
		//  the index of the first non-whitespace character. Copy 1023-lenSpace bytes back
		//  into buffer
		// THIS ISN'T NECESSARY
		// strncpy(buf, (buf + (sizeof(char) * lenSpace)), PATH_MAX_L - lenSpace);
		// if the specified command is “exit”, terminate the program taking care to release
		// any allocated resources.
		// printf("%d\n", shell_file_exists(args[0]));
		if (strcmp(args[0], "exit") == 0)
		{
			exit = 1;
		} // change the current working directory to the specified directory path using shell_change_dir()
		else if (strcmp(args[0], "cd") == 0)
		{
			printf("change directories to %s\n", args[1]);
			shell_change_dir(args[1]);
		}
		else if(strcmp(args[0], "ls") == 0)
		{
			printf("if only...\n");
		}
		else
		{
			char *file_path = (char *)calloc((PATH_MAX_L + FILENAME_MAX + 1), sizeof(char));
			char *local_file = (char *)calloc(1024, sizeof(char));

			// how deal with too long path?
			strncat(cwd, local_file, PATH_MAX_L);
			strncat(args[0], local_file, 100);
			// make sure it's null terminated: strncat doesn't guarantee this.
			local_file[strlen(local_file) - 1] = '\0';
			// if the command is specified using an absolute path or local file?
			// test with notepad: "C:\ProgramData\Microsoft\Windows\Start Menu\Programs\Accessories\Notepad.lnk"
			if (shell_file_exists(args[0]) == 0)
			{
				printf("args[0] shell_file_exists\n");
				shell_execute(args[0], args);
			} // command exists in the PATH
			else if (shell_find_file(args[0], file_path, BUF_SIZE/*?*/) == 0)
			{
				printf("shell_find_file\n");
				shell_execute(file_path, args);
			} // or exists in current folder
			else if (shell_file_exists(local_file) == 0)
			{
				printf("local_file shell_file_exists\n");
				shell_execute(local_file, args);
			}
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
