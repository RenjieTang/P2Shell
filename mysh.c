#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

int counter = 0;
int* processes; 

int buildRedir(char** args, int length) {
	int less = -1;
	int more = -1;
	//int errFlag = 0;
	for(int i = 0; i < length; i++) {
		if(strcmp(args[i], "<") == 0) {
			if(less != -1) {
				return 1;
			}
			less = i;
		}
		else if(strcmp(args[i], ">") == 0) {
			if(more != -1) {
				return 1;
			}
			more = i;
		}
	}
	if(less == -1 && more == -1) {
		return 0;
	}
	else if(less != -1 && more == -1) {
		if(less != length-2) {
			return 1;
		}
		else {
			int in = open(args[length-1],O_RDONLY);
			if (in < 0) {
				//error
				close(in);
				return 1;
			}
			dup2(in, 0);
			args[length-2] = NULL;
			close(in);
		}
	}
	else if(less == -1 && more != -1) {
		if(more != length-2) {
			return 1;
		}
		else {
			int out = open(args[length-1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
			dup2(out, 1);
			args[length-2] = NULL;
			close(out);
		}
	}
	else {
		int max = -1;
		int min = -1;
		if(less > more) {
			max = less;
			min = more;
		}
		else {
			max = more;
			min = less;
		}
		if(max-min != 2 || args[max+2] != NULL) {
			return 1;
		}
		int in = open(args[less+1],O_RDONLY);
		dup2(in, 0);
		if (in < 0) {
			//error
			close(in);
			return 1;
		}
		int out = open(args[more+1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
		dup2(out, 1);		
		close(in);
		close(out);
		args[min] = NULL;
	}
	return 0;
}

void exitCommand(char** args) {
	for (int i = 0; i < 20; i++) {
	//	printf("process id is %d\n", processes[i] );
		if(processes[i] != -1) {
			kill(processes[i], 1);
		}
	}																
	exit(0);
}

int cdCommand(char** args) {
//	printf("cd called\n");
//	printf("args[1] is %s\n", args[1]);
	int ret = 0;
	if(args[2] != NULL) {
		char error_message[30] = "An error has occurred\n";
		write(STDERR_FILENO, error_message, strlen(error_message));
		return 1;
	}
	else if(args[1] == NULL) {
		ret = chdir(getenv("HOME"));
	}
	else {
		ret = chdir(args[1]);
	}

	if(ret == -1) {
		char error_message[30] = "An error has occurred\n";
		write(STDERR_FILENO, error_message, strlen(error_message));
	}

	return 0;
}

int pwdCommand(char** args) {
	//printf("pwd called\n");
	if(args[1] != NULL) {
		char error_message[30] = "An error has occurred\n";
		write(STDERR_FILENO, error_message, strlen(error_message));
		return 1;
	}
	size_t size = 4096;
	char buffer[size];
	memset(buffer, 0, 4096);
	if(getcwd(buffer, size) == NULL) {
		char error_message[30] = "An error has occurred\n";
		write(STDERR_FILENO, error_message, strlen(error_message));
		return 1;
	}
	strcat(buffer, "\n");
	int len = strlen(buffer);
	
	if(write(STDOUT_FILENO, buffer, len) == -1) {
		char error_message[30] = "An error has occurred\n";
		write(STDERR_FILENO, error_message, strlen(error_message));
	}

	return 0;
}

int madeCommands(char** args, int length) {
	// if(strcmp(args[0], "exit") == 0) {
	// 	exitCommand(args);
	// }
	// else{
		// int rc = fork();
		// if(rc < 0) {
		// 	char error_message[30] = "An error has occurred\n";
		// 	write(STDERR_FILENO, error_message, strlen(error_message));
		// 	//TODO;
		// 	exit(1);
		// }
		// else if (rc == 0) {
	if(length >= 3 && strcmp(args[length-2],">") == 0) {
		//printf("I am in \n");
		int out = open(args[length-1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
		dup2(out, 1);
		args[length-2] = NULL;
		close(out);
	}

	int ret = 0;
	if(strcmp(args[0], "pwd") == 0) {
		ret = pwdCommand(args);
	}
	else if(strcmp(args[0], "cd") == 0) {
		ret = cdCommand(args);
	}
	else {
		exitCommand(args);
	}
	return(ret);
		// }
		// else {
		// 	waitpid(rc, NULL, 0);
		// 	return 0;
		// }
	// }
	return 0;
}

int existedCommands(char** args, int length) {
	int background = 0;
	if(strcmp(args[length-1],"&") == 0) {
		background = 1;
		args[length-1] = NULL;
		length = length-1;
	}


	int rc = fork();
	if(rc < 0) {
		char error_message[30] = "An error has occurred\n";
		write(STDERR_FILENO, error_message, strlen(error_message));
		//TODO;
		exit(1);
	}
	else if (rc == 0) {
		// char filename[128];
		// strcpy(filename, "/bin/");
		// strcat(filename, args[0]);
		// printf("file name is %s\n", filename);
		// if(length >= 3 && strcmp(args[length-2],">") == 0) {
		// 	printf("I am in\n");
		// 	int out = open(args[length-1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
		// 	dup2(out,1);
		// 	args[length-2] = NULL;
		// 	close(out);
		// }
		// if(length >= 3 && strcmp(args[length-2], "<") == 0) {
		// 	int in = open(args[length-1],O_RDONLY);
		// 	dup2(in,0);
		// 	args[length-2] = NULL;
		// 	close(in);
		// }
		int vb = -1;
		for(int i = 0; i < length; i++) {

			if(strcmp(args[i],"|") == 0 ) {
				if(vb != -1) {
					char error_message[30] = "An error has occurred\n";
					write(STDERR_FILENO, error_message, strlen(error_message));
					break;
				}
				vb = i;
			}
		} 

		
	//	printf("vb is !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!%d\n", vb);

		if(vb == -1) {
			if(buildRedir(args, length) == 1) {
				char error_message[30] = "An error has occurred\n";
				write(STDERR_FILENO, error_message, strlen(error_message));
				exit(1);
			}
			else{

				int rt = execvp(args[0], args);
				//printf("This command is excuted \n");
				if(rt == -1) {
					char error_message[30] = "An error has occurred\n";
					write(STDERR_FILENO, error_message, strlen(error_message));
					exit(1);
				}	
			}
		}
		else {
			if(args[vb+1] == NULL) {
				char error_message[30] = "An error has occurred\n";
				write(STDERR_FILENO, error_message, strlen(error_message));
				exit(1);
			}
			int pipefd[2];
			pid_t cpid;
			char** args2 = &args[vb+1];

			args[vb] = NULL;
			
			pipe(pipefd);
			cpid = fork();

			
			if (cpid == -1) {
				char error_message[30] = "An error has occurred\n";
				write(STDERR_FILENO, error_message, strlen(error_message));
				exit(1);
			}
			if(cpid == 0) {
					
				dup2(pipefd[0], 0);
				close(pipefd[1]);
				
				int rt = execvp(args2[0], args2);
				if(rt == -1) {
					char error_message[30] = "An error has occurred\n";
					write(STDERR_FILENO, error_message, strlen(error_message));
					exit(1);
				}
				// for(int i = 0; i < length+1; i++) {
				// 	free(args[i]);
				// }
				// free(args);
				//exit(0);
			}
			else {
				// printf("args0 %s\n", args[0]);
				// printf("args1 %s\n",args[1]);
				// printf("args2 %s\n",args[2]);
				// printf("args'0%s\n",args2[0]);
				// printf("args'1%s\n",args2[1]);

				dup2(pipefd[1], 1);
				close(pipefd[0]);
				int rt = execvp(args[0], args);
				if(rt == -1) {
					char error_message[30] = "An error has occurred\n";
					write(STDERR_FILENO, error_message, strlen(error_message));
				}
				// for(int i = 0; i < 20; i++) {
				// 	if(processes[i] == -1) {
				// 		processes[i] = cpid;
				// 		break;
				// 	}
				// }
				// printf("This line is executed\n");
				// waitpid(cpid, NULL, 0);
			}
		}
		//exit(0);
	}
	else {
		for(int i = 0; i < 20; i++) {
			if(processes[i] == -1) {
				processes[i] = rc;
			//	printf("process id is %d\n", processes[i]);
				break;
			}
		}
		if(background == 0) {
			waitpid(rc, NULL, 0);
		}
		// else {
		// 	waitpid(rc, NULL, 0);
		// }
		return 0;
	}
	
}

int loop() {
	char* input = (char*) malloc(129);
	char* savedCom = (char*) malloc(129);

	while(1) {
		counter++;
		int builtIn = 1;
		memset(input,0,129);
		//sleep(1);
		struct timespec tim, tim2;
   		tim.tv_sec = 0;
   		tim.tv_nsec = 5000000L;
   		nanosleep(&tim, &tim2);

   		int status = -1;
   		for(int i = 0; i < 20; i++) {
   				//printf("process id %d\n",processes[i]);
   			waitpid(processes[i], &status, WNOHANG);

   			//printf("status number is %d\n", status);
   			if(WIFEXITED(status)) {
   				//printf("process killed %d\n", i);
   				processes[i] = -1;

   			
   			}
   		}

		printf("mysh (%d)> ", counter);
		fflush(stdout);
		if(fgets(input,129,stdin) == NULL) {
			char error_message[30] = "An error has occurred\n";
			write(STDERR_FILENO, error_message, strlen(error_message));
			return 1;
		}

		char* pos; 
		if((pos = strchr(input,'\n')) != NULL) {
			*pos = '\0';
		}
		else {
			char error_message[30] = "An error has occurred\n";
			write(STDERR_FILENO, error_message, strlen(error_message));
			scanf("%*[^\n]");
			scanf("%*c");
			continue;
		}
		for(int i = 0; i < 129; i++) {
			if(input[i] == '\t') {
				input[i] = ' ';
			}
		}

		//printf("intput is %s\n", input);
		char* tempInput = (char*) malloc(129);
		strcpy(tempInput,input);
		char* com = strtok_r(tempInput, " ", &savedCom);
		if(!com) {
			counter--;
			continue;
		}
		//printf("the comman is %s\n", com);
		if(strcmp(com, "pwd") == 0 || strcmp(com, "cd") == 0 || strcmp(com, "exit") == 0) {
			builtIn = 0;
		}
		int length = 0;
		while(com != NULL) {
			com = strtok_r(NULL, " ", &savedCom);
			length++;
		}
		//printf("length is %d\n", length);
	
		char** args = (char**) malloc((length+1)*sizeof(char*));
		for(int i = 0; i < length+1; i++) {
			args[i] = (char*) malloc(129);
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
		for(int i = 0; i < length+1; i++) {
			free(args[i]);
		}
		free(args);
		free(tempInput);
	}
	free(input);
	free(savedCom);


}

int main(int argc, char* argv[]) {
	processes = (int*) malloc(20*sizeof(int));
	memset(processes, -1, 20*sizeof(int));
	if(argc != 1) {
		char error_message[30] = "An error has occurred\n";
		write(STDERR_FILENO, error_message, strlen(error_message));
		exit(1);
	}
	loop();
	free(processes);
	return 0;
}
