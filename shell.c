#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int counter = 1;

void exitCommand(char** args) {
	exit(0);
}

int cdCommand(char** args) {
	int ret = 0;
	if(args[2] != NULL) {
		fprintf(stderr, "error\n");
		return 1;
	}
	else if(args[1] == NULL) {
		ret = chdir(getenv("HOME"));
	}
	else {
		ret = chdir(args[1]);
	}

	if(ret == -1) {
		fprintf(stderr, "error\n");
	}

	return 0;
}

int pwdCommand(char** args) {
	if(args[1] != NULL) {
		fprintf(stderr, "error\n");
		return 1;
	}
	size_t size = 4096;
	char buffer[size];
	if(getcwd(buffer, size) == NULL) {
		fprintf(stderr, "error\n");
		return 1;
	}
	
	printf("%s\n", buffer);

	return 0;
}

int madeCommands(char** args, int length) {
	if(strcmp(args[0], "exit") == 0) {
		exitCommand(args);
	}
	else{
		int rc = fork();
		if(rc < 0) {
			fprintf(stderr, "error\n");
			//TODO;
			exit(1);
		}
		else if (rc == 0) {
			int ret = 0;
			if(strcmp(args[0], "pwd") == 0) {
				ret = pwdCommand(args);
			}
			else {
				ret = cdCommand(args);
			}
			return ret;
		}
		else {
			waitpid(rc, NULL, 0);
			return 0;
		}
	}
	return 0;
}

int existedCommands(char** args, int length) {
	int rc = fork();
	if(rc < 0) {
		fprintf(stderr, "error\n");
		//TODO;
		exit(1);
	}
	else if (rc == 0) {
		char filename[128];
		strcpy(filename, "/bin/");
		strcat(filename, args[0]);
		printf("file name is %s\n", filename);
		if(strcmp(args[length-2],">") == 0) {
			printf("I am in\n");
			int out = open(args[length-1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
			dup2(out,1);
			args[length-2] = NULL;
			close(out);
		}
		int rt = execvp(filename,args);
		fprintf(stderr, "error %d\n", rt); //look up perror
		return 0;
	}
	else {
		waitpid(rc, NULL, 0);
		return 0;
	}
	
}

int loop() {
	char* input = (char*) malloc(128);
	char* savedCom = (char*) malloc(128);

	while(1) {
		int builtIn = 1;
		memset(input,0,128);
		printf("mysh (%d)> ", counter);
		if(fgets(input,128,stdin) == NULL) {
			fprintf(stderr,"error\n");
			return 1;
		}

		char* pos = strchr(input,'\n');
		*pos = '\0';

		printf("intput is %s\n", input);
		char* tempInput = (char*) malloc(128);
		strcpy(tempInput,input);
		char* com = strtok_r(tempInput, " ", &savedCom);
		if(strcmp(com, "pwd") == 0 || strcmp(com, "cd") == 0 || strcmp(com, "exit") == 0) {
			builtIn = 0;
		}
		int length = 0;
		while(com != NULL) {
			com = strtok_r(NULL, " ", &savedCom);
			length++;
		}
		printf("length is %d\n", length);
	
		char** args = (char**) malloc((length+1)*sizeof(char*));
		for(int i = 0; i < length+1; i++) {
			args[i] = (char*) malloc(128);
		}

		com = strtok_r(input, " ", &savedCom);
		strcpy(args[0], com);
		for(int i = 1; i < length; i++) {
			com = strtok_r(NULL, " ", &savedCom);
			strcpy(args[i], com);
		}
		args[length] = NULL;


		if(builtIn == 1) {
			existedCommands(args, length);
		}
		else {
			madeCommands(args, length);
		}
		// for(int i = 0; i < length+1; i++) {
		// 	printf("the command is %s\n", args[i]);
		// }


		//TODO: Handle only enter
	 	counter++;
	}



}

int main(int argc, char* argv[]) {
	if(argc != 1) {
		fprintf(stderr,"error\n");
		exit(1);
	}
	loop();
}