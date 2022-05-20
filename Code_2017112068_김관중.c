#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_LINE 80

int enter_command(char input[]){
	
	while(1){
		int input_leng = read(STDIN_FILENO, input, MAX_LINE);
		
		if(input_leng == -1){
			printf("please rewrite the command");
		}
		else{
			return input_leng;
		}
	
	}

}

void parsing(char input[], char *args[], int input_leng, int *is_background){
	
	char* token = malloc(sizeof(char)*MAX_LINE);
	int idx = 0;
	
	for (int i = 0; i < input_leng; i++){
		if(input[i] == ' ' || input[i] == '\t' || input[i] == '\n'){
			char nul = '\0';
			strcat(token, &nul);
			
			args[idx++] = token;
			token = malloc(sizeof(char)*MAX_LINE);
		}
		else{
			char now = input[i];
			if(now == '&') *is_background = 1;
			
			strcat(token, &now);
		}
	
	}
	
	args[idx] = NULL;
}

void show_history(char *history[], int history_cnt){
	printf("\nCommand History\n");
	
	int idx = history_cnt;
	char yesno = 'Y';
	int flag = 0;
	int i;
	
	while(yesno == 'Y'){
	
		for(i = history_cnt; i >= history_cnt - 9; i--){
			if (i <= 0) {
				flag = 1;
				break;
			}
			printf("%d : %s", i, history[i-1]);
		}
		
		if (flag == 1 || history_cnt == 10) break;
		
		history_cnt = i;
		printf("\n");
		printf("Do you want to know more commands?, Enter [Y/N] : ");
		
		scanf(" %c", &yesno);
		while(yesno != 'Y' && yesno != 'N') {
			printf("Please Enter [Y/N] : ");
			scanf(" %c", &yesno);
		}
	}

}


int main(void){
	char *args[MAX_LINE/2 +1];
	int should_run = 1;
	
	char* input;
	int input_leng = 0;
	int is_background = 0;
	
	char* history[1000];
	int history_cnt = 0;
	
	while (should_run){
		printf("\nosh>");
		fflush(stdout);
		
		is_background = 0;
		input = malloc(sizeof(char)*MAX_LINE);
		
		// 1. input the command
		input_leng = enter_command(input);
		
		// 2. parsing the input
		parsing(input, args, input_leng, &is_background);
		
		// 3. execute the command
		// 3-1. exit
		if(strcmp(args[0],"exit") == 0) exit(0);
		// 3-2. history
		else if(strcmp(args[0], "history") == 0) {
			if(history_cnt == 0) printf("There is no command history\n");
			else show_history(history, history_cnt);
		}
		// 3-3. others
		else{
			
			pid_t pid = fork();
			
			if(pid < 0) printf("Error : Failed to Fork ");
			else if(pid == 0){ // children process
			
				if(args[0][0] == '!' && args[0][1] == '!') { // !! --> execute most recent command
				
					if (history_cnt == 0) {
						printf("Command history does not exist\n");
					}
					else{
						int cnt = history_cnt - 1;
					
						char *new = malloc(sizeof(char)*MAX_LINE);
						strcpy(new, history[cnt]);
					
						parsing(new, args, strlen(new), &is_background);

						if(execvp(args[0], args) == -1){
							printf("Command Error\n");
						}
					}
				}
				else if(args[0][0] == '!' && isdigit(args[0][1]) != 0){	// !number --> execute history number command
					char *num = &args[0][1];
					int cnt = atoi(num);
						
					if(cnt > history_cnt || cnt < 0){
						printf("The corresponding history number does not exist\n");
					}
					else{
						char *new = malloc(sizeof(char)*MAX_LINE);
						strcpy(new, history[cnt-1]);
						parsing(new, args, strlen(new), &is_background);
						
						if(execvp(args[0], args) == -1){
							printf("Command Error\n");
						}
					}
				}	
				else{ // other command
					if(execvp(args[0], args) == -1){
						printf("Command Error!\n");
					}
				}
			}
			else { // parent process
				if (is_background == 0){ // no background
					wait(NULL);
				}
				
				// push only pure command, not history command
				if (args[0][0] != '!') history[history_cnt++] = input;
			}
		}
	}

	return 0;
}
