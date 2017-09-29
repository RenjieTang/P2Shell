#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int counter = 1;

int loop() {
	char* input = (char*) malloc(128);
	char* savedCom = (char*) malloc(128);

	while(1) {
		int builtIn = 0;
		memset(input,0,128);
		printf("mysh (%d)> ", counter);
		if(fgets(input,128,stdin) == NULL) {
			fprintf(stderr,"error\n");
			return 1;
		}
		printf("intput is %s\n", input);
		char* tempInput = (char*) malloc(128);
		strcpy(tempInput,input);
		char* com = strtok_r(tempInput, " ", &savedCom);
		if(strcmp(com, "pwd") == 0 || strcmp(com, "cd") == 0 || strcmp(com, "exit") == 0) {
			builtIn = 1;
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
		// int curr = 1;
		// while(com != NULL) {
		// 	com = strtok_r(NULL, " ", &savedCom);
		// 	strcpy(args[curr], com);
		// 	curr++;
		// }
		for(int i = 1; i < length; i++) {
			com = strtok_r(NULL, " ", &savedCom);
			strcpy(args[i], com);
		}
		args[length] = NULL;
		for(int i = 0; i < length+1; i++) {
			printf("the command is %s\n", args[i]);
		}
		// for (int i = 0; i < 20; i++) {
		printf("savedCom is %s\n", savedCom);

		// }


	
		// while(com != NULL) {
		// 	com = strtok_r(NULL, " ", savedCom);
		// 	printf("com is %s\n", com);
		// 	printf("savedCom is %s\n", savedCom[0]);
		// 	curr++;
		// }
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