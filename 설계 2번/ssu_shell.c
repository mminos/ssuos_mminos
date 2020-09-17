#include "ssu_shell.h"


int main (int argc, char* argv[]) 
{
	pid_t pid;
	char line[MAX_INPUT_SIZE];            
	char **tokens;
	char **commands;
	int i, commandNo = 0;
	FILE* fp;

	if (argc == 2) {
		fp = fopen(argv[1],"r");

		if (fp == NULL) {
			
			printf("File doesn't exists.");
			return -1;
		}
	}

	while(1) {
		int pipe_num = 0;
		int status;
		
		commandNo = 0;
		memset(line, 0, sizeof(line));

		if(argc == 2) {
			if(fgets(line, sizeof(line), fp) == NULL) {
				break;	
			}

			line[strlen(line) - 1] = '\0';
		} else {
			printf("$ ");
			scanf("%[^\n]", line);
			getchar();
		}

//		printf("Command entered: %s (remove this debug output later)\n", line);

		line[strlen(line)] = '\n';
		tokens = tokenize(line);

		for (i = 0; tokens[i] != NULL; i++) {
			if (!strcmp(tokens[i], "pps")) {
				tokens[i] = realloc(tokens[i], 6);	
				sprintf(tokens[i], "%s", "./pps");
			}
			else if (!strcmp(tokens[i], "ttop")) {
				tokens[i] = realloc(tokens[i], 7);
				sprintf(tokens[i], "%s", "./ttop");
			}
//			printf("found token %s (remove this debug output later)\n", tokens[i]);
		}

		commands = merge_to_command(tokens);

		for (i = 0; commands[i] != NULL; i++, commandNo++) {
//			printf("commands : %s (remove this depug output later)\n", commands[i]);
		}

		
		if ((pid = fork()) < 0) {
			fprintf(stderr, "in function main: fork() error\n");
			return -1;
		}
		else if (pid == 0) {
			execute_commands(commands, commandNo);
		} 
		else {
			wait(&status);
			if (!WIFEXITED(status)) {
				fprintf(stderr, "child process doesn't exit normally.. check process\n");
				continue;
			}
		}

		
		for (i = 0; tokens[i] != NULL; i++){
			free(tokens[i]);
		}
		free(tokens);

		for (i = 0; commands[i] != NULL; i++) {
			free(commands[i]);
		}
		free(commands);
	}
	return 0;
}

char **tokenize(char *line)
{
        char **tokens = (char **)malloc(MAX_TOKEN_NUM * sizeof(char *));
        char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
        int i, tokenIndex = 0, tokenNo = 0;

        for (i = 0; i < strlen(line); i++) { 

                char readChar = line[i];

                if (readChar == ' ' || readChar == '\n' || readChar == '\t') {
                        token[tokenIndex] = '\0';
                        if (tokenIndex != 0) {
	                        tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	                        strcpy(tokens[tokenNo++], token);
	                        tokenIndex = 0; 
                        }
                } 
                else {
                        token[tokenIndex++] = readChar;
                }
        }
 
        free(token);
        tokens[tokenNo] = NULL ;
        return tokens;
}

char **merge_to_command(char **tokens) 
{
	char **commands = (char **)malloc(MAX_COMMAND_NUM * sizeof(char *));
	char command[1024];
	int i, commandNo = 0;

	memset(command, 0, 1024);

	for (i = 0; tokens[i] != NULL; i++) {
		if (!strcmp(tokens[i], "|")) {
			commands[commandNo] = (char *)malloc(1024 * sizeof(char));
			strcpy(commands[commandNo++], command);
			memset(command, 0, 1024);
		}
		else {
			strcat(command, tokens[i]);
			strcat(command, " ");
		}
	}
	
	commands[commandNo] = (char *)malloc(1024 * sizeof(char));
	strcpy(commands[commandNo++], command);

	commands[commandNo] = NULL;
	return commands;
}

void execute_commands(char **commands, int commandNo)
{
	int i;
	int pipe_fd[2];
	pid_t pid;


	for (i = 0; i < commandNo - 1; i++) {
		pipe(pipe_fd);
		
		if ((pid = fork()) < 0) {
			fprintf(stderr, "in function execute_commands: fork() error\n");
			exit(1);
		}
		else if (pid == 0) {
			close(pipe_fd[0]);
			dup2(pipe_fd[1], STDOUT_FILENO);
			execute_command(commands[i]);
		}
		else {
			close(pipe_fd[1]);
			dup2(pipe_fd[0], STDIN_FILENO);
		}
	}
	execute_command(commands[i]);
}

void execute_command(char *command)
{
	char **command_tok;

	command_tok = tokenize(command);
	execvp(command_tok[0], command_tok);

	fprintf(stderr, "SSUShell : incorrect command : %s\n", command);
	exit(1);
}
