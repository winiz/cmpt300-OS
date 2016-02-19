#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define COMMAND_LENGTH 1024
#define NUM_TOKENS (COMMAND_LENGTH / 2 + 1)
#define DELIMITER "  \n\a\t\r" 


/* -----------------------------------------------
  Function Prototypes
  ------------------------------------------------
*/ 

/*
* Read a command from the keyboard into the buffer 'buff' and tokenize it
* such that 'tokens[i]' points into 'buff' to the i'th token in the command.
* buff: Buffer allocated by the calling code. Must be at least
* COMMAND_LENGTH bytes long.
* tokens[]: Array of character pointers which point into 'buff'. Must be at
* least NUM_TOKENS long. Will strip out up to one final '&' token.
* 'tokens' will be NULL terminated.
* in_background: pointer to a boolean variable. Set to true if user entered
* an & as their last token; otherwise set to false.
*/

int tokenize_command(char *buff, char *tokens[]){
    //returns token count
    char* token;
    int pos = 0;
    
    token = strtok(buff, DELIMITER);
    while (token != NULL){
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
  	if (buff[strlen(buff) - 1] == '\n') {
  		buff[strlen(buff) - 1] = '\0';
  	}
  	
  	// Tokenize (saving original command string)
  	int token_count = tokenize_command(buff, tokens);
  	if (token_count == 0) {
  		return;
  	}

  	// Extract if running in background:
  	if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
  		*in_background = true;
  		tokens[token_count - 1] = 0;
  	}
}

int meat(char* tokens[], _Bool* in_background){
  pid_t pid, wpid;
  int status, errorChecker, i;
  
  if (tokens[0] == NULL) {return 1;} //prompt again to ask for command
  
  // forking starts 
  pid = fork();
  if (pid == 0) {
    // Child process
    errorChecker = execvp(tokens[0], tokens);
    if (errorChecker == -1) {
        perror("execvp() error");

   }
    exit(EXIT_FAILURE);
  } 
  
  else if (pid < 0) {
  // Error forking
  perror("fork() error");
  }
  
  else {
    // Parent process
    if (in_background == false ){
      do {
       wpid = waitpid(pid, &status, WUNTRACED);
      } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
  }
  return 1; // we should prompt for input again
}

/*
  Function Declarations for commands:
 */
int shell_exit(char* tokens[]){return 0;}

//int shell_pwd(){}

int shell_cd(char* tokens[]){
  if (tokens[1] == NULL) {    
    perror("not enough arguments provided");
  } 
  else {
    if (chdir(tokens[1]) != 0) {
    perror("chdir() error");
    }
  }
  return 1;
}

/**
* Main and Execute Commands
*/
int main(int argc, char* argv[]){
  
  char input_buffer[COMMAND_LENGTH];
  char *tokens[NUM_TOKENS];
  char cwdBuff[1024];
  int i = 0; //testing
  int pos = 0;
  

  while (true) {
  // Get command
  // Use write because we need to use read()/write() to work with
  // signals, and they are incompatible with printf().
  getcwd(cwdBuff, sizeof(cwdBuff));
  while (cwdBuff[pos] != '\0'){
    pos++;
  }
  char cwd[pos+1];
  strcpy (cwd, cwdBuff);
  cwd[pos] = '>';
  write(STDOUT_FILENO, cwd, strlen(cwd));
  _Bool in_background = false;
  read_command(input_buffer, tokens, &in_background);

  for (i = 0; i < 3; i++){
    printf ("        test: tokens[%d]=%s\n", i, tokens[i]);
  }//testing
  
  meat(tokens, &in_background);
  }
  return 0;
}
