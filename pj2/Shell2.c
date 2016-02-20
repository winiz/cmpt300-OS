
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/syscall.h>
#include <sys/wait.h>


#define DELIMITER "  \n\a\t\r"
#define COMMAND_LENGTH 1024
#define NUM_TOKENS (COMMAND_LENGTH / 2 + 1)
#define HISTORY_DEPTH 10

/**
* Read a command from the keyboard into the buffer 'buff' and tokenize it
• * such that 'tokens[i]' points into 'buff' to the i'th token in the command.
• * buff: Buffer allocated by the calling code. Must be at least
• * COMMAND_LENGTH bytes long.
• * tokens[]: Array of character pointers which point into 'buff'. Must be at
• * least NUM_TOKENS long. Will strip out up to one final '&' token.
• * 'tokens' will be NULL terminated.
• * in_background: pointer to a boolean variable. Set to true if user entered
• * an & as their last token; otherwise set to false.
• */
int tokenize_command(char *buff, char *tokens[])
{
	char* token;   // a token is word
    int pos = 0;
    
    token = strtok(buff, DELIMITER);
    
    while (token != NULL)
    {
      tokens[pos] = token;
      token = strtok(NULL, DELIMITER);
      pos++;
    }
    return pos;
	
}
void read_command(char *buff, char *tokens[], _Bool *in_background)
{
    
	*in_background = false;

	// Read input
	
	int length = read(STDIN_FILENO, buff, COMMAND_LENGTH-1);
	if ( (length < 0) && (errno !=EINTR) )
	{
		perror("Unable to read command. Terminating.\n");
		exit(-1); /* terminate with error */
	}

	// Null terminate and strip \n.
	buff[length] = '\0';
	if (buff[strlen(buff) - 1] == '\n') 
	{
		buff[strlen(buff) - 1] = '\0';
	}


	// Tokenize (saving original command string)
	int token_count = tokenize_command(buff, tokens);
	
	if (token_count == 0) 
	{
		return;
	}

	// Extract if running in background:
	if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) 
	{
		*in_background = true;
		tokens[token_count - 1] = 0;
	}
}

int history_add_command(int spot, char history[HISTORY_DEPTH][COMMAND_LENGTH], char input_buffer[COMMAND_LENGTH]){
    int k = 0;
        for (k=0; k<30; k++){
        history[spot][k] = input_buffer[k];
            k++;
        }
        
        history[spot][k] = '\0'; 
    return 0;
}

//int history_retrieve_command(int spot, char** history,int* agrv,){};

void history_print(int spot, char history[HISTORY_DEPTH][COMMAND_LENGTH]){
  int i = 0;
  for (i=0;i<spot;i++){
    printf("spot is %d, command is %s\n", i,history[i]);
  }
}

/**
• * Main and Execute Commands
• */
int main(int argc, char* argv[])
{
    pid_t pid;
    int status;
    char input_buffer[COMMAND_LENGTH];
    int total_command = 0;
    char *tokens[NUM_TOKENS];
     // counting commands through out history of run time
    char history[HISTORY_DEPTH][COMMAND_LENGTH]; // global history 2D array
    int spot = 0;
    int i = 0;
    
    while (true) 
    {

// Get command
// Use write because we need to use read()/write() to work with
// signals, and they are incompatible with printf().

	char* dirname = getcwd(NULL, 0);
	strcat(dirname, " $ ");
    write(STDOUT_FILENO, dirname , strlen(dirname) + 1);

//    write(STDOUT_FILENO, "> ", strlen("> "));
    _Bool in_background = false;

// Clear last tokens list
    while (tokens[i] != NULL){
        tokens[i] = NULL;
        i++;
    }
    
// Filling in tokens list with new commands 
// from charaters buffer called input_buffer
    read_command(input_buffer, tokens, &in_background);
    total_command++;
    spot = 10 % total_command;
    history_add_command(spot, history,input_buffer);

    char *cmd = tokens[0];
    if (tokens[0] == NULL)
    {
    	continue;
    }
    if (strcmp(cmd,"exit") == 0)
    {
    	exit(-1);
    	return 0;
    }
    if (strcmp(cmd,"pwd") == 0)
    {
    	char* dirname2 = getcwd(NULL,0);
    	write(STDOUT_FILENO, dirname2, strlen(dirname2));
    	continue;
    }
    if (strcmp(cmd,"cd") == 0)
    {
    	if (chdir(tokens[1]) == -1)
    		perror("Can't change directory   Blah");
    	continue;
    }
    if (strcmp(cmd,"history") == 0)
    {
    	history_print(spot, history);
    }
    
    
    pid = fork();
    if (pid == -1)
    {
    	perror("Unable to create child process. Terminating.\n");
    	exit(-1);
        return 0;
    }
    else if (pid == 0)
    {
        if (execvp(tokens[0], tokens) == -1)
        	perror("Can't execute commands\n");;
        exit(-1);
        return 0;
    }
    else
    {
        if (in_background)
        	continue;
        else 
        	waitpid(pid, &status, WUNTRACED);
    }
    

/**
• * Steps For Basic Shell:
• * 1. Fork a child process
• * 2. Child process invokes execvp() using results in token array.
• * 3. If in_background is false, parent waits for
• * child to finish. Otherwise, parent loops back to
• * read_command() again immediately.
• */
    }
    
    return 0;
}