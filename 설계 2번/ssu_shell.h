//headers

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>


//define

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_TOKEN_NUM 64
#define MAX_COMMAND_NUM 64

//functions

char **tokenize(char *line);
char **merge_to_command(char **tokens);
void execute_commands(char **commands, int commandNo);
void execute_command(char *command);
