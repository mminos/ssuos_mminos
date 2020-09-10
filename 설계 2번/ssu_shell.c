#include "ssu_shell.h"


int main (int argc, char* argv[]) 
{
	char line[MAX_INPUT_SIZE];            
	char **tokens;              
	int i;

	FILE* fp;
	if (argc == 2) {
		fp = fopen(argv[1],"r");

		if(fp < 0) {
			printf("File doesn't exists.");
			return -1;
		}
	}

	while(1) {
		memset(line, 0, sizeof(line));

		if(argc == 2) { // batch mode
			if(fgets(line, sizeof(line), fp) == NULL) { // file reading finished
				break;	
			}

			line[strlen(line) - 1] = '\0';
		} 
		else { // interactive mode
			printf("$ ");
			scanf("%[^\n]", line);
			getchar();
		}

		printf("Command entered: %s (remove this debug output later)\n", line);

		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);

		for (i = 0; tokens[i] != NULL; i++) {
			printf("found token %s (remove this debug output later)\n", tokens[i]);
		}






		// Freeing the allocated memory	
		for(i = 0; tokens[i] != NULL; i++){
			free(tokens[i]);
		}
		free(tokens);

	}
	return 0;
}

char **tokenize(char *line)
{
        char **tokens = (char **)malloc(MAX_TOKEN_NUM * sizeof(char *));
        char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
        int i, tokenIndex = 0, tokenNo = 0;

        for (i =0; i < strlen(line); i++) { 

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
