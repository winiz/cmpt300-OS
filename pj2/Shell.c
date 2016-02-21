#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <linux/limits.h>

#define DELIMITER "  \n\a\t\r"
#define COMMAND_LENGTH 1024
#define NUM_TOKENS (COMMAND_LENGTH / 2 + 1)
#define HISTORY_DEPTH 10


char history[HISTORY_DEPTH][COMMAND_LENGTH];
int commandCount;


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



void add_command(char buff[], int ordernumber)
{
    int i = (ordernumber -1) % 10;
    if (ordernumber > 10)
    {
        memset(&history[i][0], '\0', strlen(history[i]) + 1);
    }
    for(int j = 0; j < strlen(buff); j++)
    {
        history[i][j] = buff[j];
    }
    return;
}
char* retrieve_command(int ordernumber)
{   
    
    int x = (ordernumber -1 ) % 10;

    return history[x];
    
}
void print_commands()
{

    int ordernum;
    char doSomething[COMMAND_LENGTH];
    int number;
    if (commandCount < 10)
    {
        number = commandCount - 1;
    }
    else
    {
        number = 9;
    }

    while (number >= 0)
    {
        ordernum = commandCount - number;
        
        sprintf(doSomething, "%d", ordernum);
        strcat(doSomething, "\t");
        strcat(doSomething, retrieve_command(ordernum));
        strcat(doSomething, "\n");
        write(STDOUT_FILENO, doSomething , strlen(doSomething) + 1);
        number--;
    }
    return;
    
}


int tokenize_command(char *buff, char *tokens[])
{
	char* token;
	char* temptoken;

    int pos = 0;
    char temp[COMMAND_LENGTH];
    
    
    int i = 0;
    
    while (i < NUM_TOKENS)
    {
        tokens[i] = 0;
        i++;
    }
    
    memset(&temp[0], '\0', sizeof(temp));

    strcpy(temp, buff);
    token = strtok(temp, DELIMITER);
    
    
    while (token != NULL)
    {
        
        temptoken = strdup(token);
        tokens[pos] = strcat(temptoken,"");
        token = strtok(NULL, DELIMITER);
        pos++;
//        free(temptoken);
    }
    return pos;
	
}
void read_command(char *buff, char *tokens[], _Bool *in_background)
{
    
	*in_background = false;

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

/**
• * Main and Execute Commands
• */
void do_history(char* buff, char *tokens[], int ordernumber,  _Bool *in_background)
{
    memset(&buff[0],'\0', strlen(buff) + 1);
    strcpy(buff,retrieve_command(ordernumber));
    
    write(STDOUT_FILENO, buff, strlen(buff) + 1);
    write(STDOUT_FILENO, "\n", strlen("\n") + 1);
    int len = tokenize_command(buff, tokens);
    if (len > 0 && strcmp(tokens[len - 1], "&") == 0) 
	{
		*in_background = true;
		tokens[len - 1] = 0;
	}
}

void handle_SIGINT()
{

    write(STDOUT_FILENO, "\n" , strlen("\n") + 1);
    print_commands();
    return;
}
int main(int argc, char* argv[])
{
    pid_t pid;
    int status;
    char input_buffer[COMMAND_LENGTH];
    char *tokens[NUM_TOKENS];
    char *tempdir;
    char dirname[PATH_MAX]; 
    commandCount = 0;
    char *cmd = malloc(sizeof(char*));
    
    
    struct sigaction handler;
    handler.sa_handler = handle_SIGINT;
    handler.sa_flags = 0;
    sigemptyset(&handler.sa_mask);
    sigaction(SIGINT, &handler, NULL);
    
    
    while (true) 
    {


        _Bool in_background = false;
        tempdir = getcwd(NULL, 0);
        strcpy(dirname, tempdir);
    	strcat(dirname, " > ");
    	free(tempdir);
        write(STDOUT_FILENO, dirname , strlen(dirname) + 1);
    

        memset(&input_buffer[0], '\0', sizeof(input_buffer) + 1);
        
        read_command(input_buffer, tokens, &in_background);
        
        
        if (tokens[0] == NULL)
        {
        	continue;
        }
        

        
        cmd = (char*)realloc(cmd, sizeof(tokens[0]) + 1);
        strcpy(cmd, tokens[0]);
        
        
        if (strcmp(cmd,"exit") == 0)
        {
        	break;
        }
        
        if (cmd[0] == '!')
        {
            if (cmd[1] == '!')
            {

                if (commandCount == 0)
                {
                    write(STDOUT_FILENO, "No previous command.\n", strlen("No previous command.\n") + 1);
                    continue;
                }
                else
                {
                    
                    do_history(input_buffer, tokens, commandCount, &in_background);
                    strcpy(cmd, tokens[0]);
                }
            }
            else
            {
                char* substr = malloc(sizeof (substr));
                strncpy(substr, cmd + 1, strlen(cmd) - 1);
                int a = atoi(substr);
                if (a == 0)
                {
                    write(STDOUT_FILENO, "Invalid command number, must be a positive integer\n", strlen("Invalid command number, must be a positive integer\n") + 1);
                    continue;
                }
                if (a < commandCount - 9)
                {
                    write(STDOUT_FILENO, "Can only invoke the last 10 commands\n", strlen("Can only invoke the last 10 commands\n") + 1);
                    continue;
                }
                if (a > commandCount)
                {
                    write(STDOUT_FILENO, "Command number too high.\n", strlen("Command number too high.\n") + 1);
                    continue;
                }
                free(substr);
                
                do_history(input_buffer, tokens, a, &in_background);
                strcpy(cmd, tokens[0]);

            }
        }
        
        
        
        commandCount++;
        add_command(input_buffer, commandCount);
        
        if (strcmp(cmd,"pwd") == 0)
        {
            tempdir = getcwd(NULL,0);
        	strcpy(dirname,tempdir);
        	free(tempdir);
        	strcat(dirname, "\n");
        	write(STDOUT_FILENO, dirname, strlen(dirname) + 1);
        	continue;
        }
        if (strcmp(cmd,"cd") == 0)
        {
        	if (chdir(tokens[1]) == -1)
        		perror("Can't change directory");
        	continue;
        }
        
        if (strcmp(cmd,"history") == 0)
        {
        	print_commands();
        	continue;
        }
 
        pid = fork();
        char stringer[20];
        sprintf(stringer, "%d", getpid());
        if (pid == -1)
        {
        	perror("Unable to create child process. Terminating.\n");
        	exit(-1);
            return 0;
        }
        else if (pid == 0)
        {
            if (execvp(tokens[0], tokens) == -1)
            {
                write(STDOUT_FILENO, cmd, strlen(cmd) + 1);
            	write(STDOUT_FILENO, ": Unknown command.\n", strlen(": Unknown command.\n"));
            	exit(-1);
            }
         
        }
        else
        {
            if (!in_background)
            {
                do 
                {
                    waitpid(pid, &status, WUNTRACED);
                }while (!WIFEXITED(status) && !WIFSIGNALED(status));
            }

        }
        
        while (waitpid(-1, NULL, WNOHANG) > 0);
        for(int i = 0; i < NUM_TOKENS; i++)
        {
            free(tokens[i]);
        }
    }
    
    free(cmd);
    return 0;
}