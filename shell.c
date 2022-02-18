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

const int BUF_SIZE = 1023;
const int MAX_NUM_CMD = 15;
const int ENV_PATH_L = 32760;
const int FILENAME_MAX_L = 255;
const int PATH_MAX_L = (4096-FILENAME_MAX_L);
const char WHITESPACE[] = " \f\n\r\t\v";

int shell_change_dir(char *dir_path) {
  	//use chdir() system call to change the current directory
  	int status = chdir(dir_path);
	printf("change directories");
  	if(status != 0) {
  		printf("Error: %s\n", strerror(status));
  	}
  	return status;
}

int shell_file_exists(char *file_path) {
  //use stat() sys call to check if a file exists
	printf("Does file exist?");
  struct stat *statStruct;
  int status = stat(file_path, statStruct);
  return status;
}

int shell_find_file(char *file_name, char *file_path, char file_path_size) {
  //traverse PATH env var to find absolute path of a file/command
  	printf("find file");
	char* envPath = (char*)calloc((ENV_PATH_L+1), sizeof(char));
  	strncpy(envPath, getenv("PATH"), ENV_PATH_L);
  	strcat(envPath, "\0");

  	printf("%s\n", envPath);
  	char* pathString = (char*)calloc(1023+1, sizeof(char));
  	while(envPath != NULL) {
    		//env vars are colon seperated
    		pathString = strndup(strsep(&envPath, ":"), 1023-(1+strlen(file_name)));
    		strcat(pathString, "/");
    		strcat(pathString, file_name);
    		if(shell_file_exists(pathString) == 0) {
      			file_path = strndup(pathString, 1023);
      			return 0;
    		}
  	}
  	return -1;
}

int shell_execute(char *file_path, char **argv) {
  //execute the file with the command line arguments
  // use the fork() and exec() sys call
  printf("Before Fork\n");
  pid_t pid = fork();
  printf("After fork: %d\n", pid);
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
  // run the shell
  // input buffer
  char *buf = (char*)calloc(PATH_MAX_L+1, sizeof(char));
  // current working directory
  char *cwd = (char*)calloc((PATH_MAX_L+1), sizeof(char));
  // command from buffer
  // char *cmd = (char*)calloc(, sizeof(char));
  //array of arguments for commnad from buffer. array of MAX_NUM_CMD strings
  char **args = (char**)calloc(MAX_NUM_CMD, sizeof(char*));
  int exit = 0;

  for(int i = 0; i < MAX_NUM_CMD; ++i) {
    args[i] = (char*)calloc((PATH_MAX_L+1), sizeof(char));
  }

  while(!exit) {
    getcwd(cwd, PATH_MAX_L);
    //display prompt and wait for user input
    printf("%s$ ", cwd);
    //get input
    fgets(buf, PATH_MAX_L, stdin);
    int lenSpace = strspn(buf, WHITESPACE);
    int lenChars = strcspn(buf, WHITESPACE);
    printf("lenChars: %d\n", lenChars);

    //2. filter out whitespace command
    if(lenSpace == strlen(buf)) {
      continue;
    }
    //not whitespace, remove trailing newline
    buf[strlen(buf)-1] = '\0';
    // strtok(buf, " "); will get first thingy.
    // strncpy(cmd, strtok(buf, " "), 100);
    // length = lenChars hopefully, unteseted
    args[0] = strtok(buf, " ");
    int x = 0;
    while((args[x] = strtok(NULL, " ")) != NULL && x < MAX_NUM_CMD) {
      ++x;
    }
	printf("args: ");
    for(int i = 0; i < x; ++i) {
      printf("%s ", args[i]);
    }
    printf("\n");
    //3. if command line contains non-whitespace characters
    // buf contains whole input line (up to 1023+1 chars). buf+lenSpace(char) equals 
    // the index of the first non-whitespace character. Copy 1023-lenSpace bytes back
    // into buffer
//maybe move before strtok calls
    strncpy(buf, (buf+(sizeof(char)*lenSpace)), PATH_MAX_L-lenSpace);
    //if the specified command is “exit”, terminate the program taking care to release 
    //any allocated resources.
    //printf("%d\n", shell_file_exists(args[0]));
    if(strcmp(args[0], "exit") == 0) {
	printf("exit shell.");
	exit = 1;
    }// change the current working directory to the specified directory path using shell_change_dir()
    else if(strcmp(args[0], "cd") == 0) {
	printf("change directories");
	shell_change_dir(args[0]);
    }
    else {
      char* file_path = (char*)calloc((PATH_MAX_L+FILENAME_MAX+1), sizeof(char));
      char* local_file = (char*)calloc(1024, sizeof(char));

	 // how deal with too long path?
	strncat(cwd, local_file, PATH_MAX_L);
      	strncat(args[0], local_file, 100);
      	// make sure it's null terminated: strncat doesn't guarantee this.
      	local_file[strlen(local_file)-1] = '\0';
      	// if the command is specified using an absolute path or local file?
      	// test with notepad: "C:\ProgramData\Microsoft\Windows\Start Menu\Programs\Accessories\Notepad.lnk"
      	if (shell_file_exists(args[0]) == 0) {
        	printf("args[0] shell_file_exists\n");
       		shell_execute(args[0], args);
      	} //command exists in the PATH
      	else if (shell_find_file(args[0], file_path, BUF_SIZE/*?*/) == 0) {
        	printf("shell_find_file");
        	shell_execute(file_path, args);
      	} //or exists in current folder
      	else if(shell_file_exists(local_file) == 0) {
        	printf("local_file shell_file_exists\n");
        	shell_execute(local_file, args);
      	}
      	else {// else report an error message
        	printf("No command matching \"%s\"\n", buf);
      	}
    }
  }

  for(int i = 1; i < MAX_NUM_CMD; ++i) {
    free(args[i]);
  }
  free(args);
  free(cwd);
  free(buf);
}
