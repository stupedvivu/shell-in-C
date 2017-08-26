/*This is a shell in c
	
	run this code in LINUX
	sudo apt-get install libreadline-dev ( if you already have, then ignore this )  
	references - https://brennan.io/2015/01/16/write-a-shell-in-c/
		     https://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm
		     https://youtu.be/ZGmg8wEdQOM
		     https://youtu.be/FcUlMsVX7aE 
		     
	*
	*
	@author vivek */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/wait.h>
#define PIPE_BUFFER_SIZE 20
#define PIPE_BUFFER_DIMENSION 2
#define BUFFER_SIZE 1024


int builtin_length = 2, pipe_flag = 0;
char *builtin_str[] = {"cd", "exit"};//builtins functions string...
char pipe_buffer[PIPE_BUFFER_DIMENSION][PIPE_BUFFER_SIZE];//char matrix to store pipe arguments....



//function to get the present directory...
void show_current_directory()
{
	char cwd[BUFFER_SIZE];
	
	getcwd(cwd, sizeof(cwd));

	printf("working dir@%s: ", cwd);

}


//routine to read....
char *read_line()
{
	char *command_buffer;
	int i;
	
	//readline command to read from the input..
	command_buffer = readline("\n$");
	
	//if there is a command input
	//store it as a history
	if( strlen(command_buffer) > 0 ){
		add_history(command_buffer);
	}

	//check if there is a pipe command or not...
	while( command_buffer[i] != '\0' ){
		if( command_buffer[i] == '|' ){
			pipe_flag = 1;
			break;
		}
		i++;
	}
	return command_buffer;
}


//fucntion to tokenize the input buffer....
char **parse_input_buffer(char *buffer)
{
	char *token;
	char *delim = " "; //parse with respect to the delim...
	int position = 0, buffer_size = BUFFER_SIZE;
	char **parsed_buffer = malloc(sizeof(char) * buffer_size);

	//error in malloc allocation...
	if( !parsed_buffer )
	{
		fprintf(stderr, "shell: parsed buffer allocation\n");
		exit(0);
	}


	//strtok function to parse the 
	//string buffer w.r.t. the delim...
	token = strtok(buffer, delim);
	while( token != NULL )
	{
		parsed_buffer[position] = token;
		position++;
		token = strtok(NULL, delim);
	}
	
	return parsed_buffer;
}


//execvp call handler...
int shell_execvp_execute(char **args)
{
	pid_t process_id;
	process_id = fork();

	//execute the arguments in child...
	if( process_id == 0 )
	{
		execvp(args[0], args);
	}
	if( process_id > 0 )
	{
		wait(NULL);
		return 1;
	}
	if( process_id < 0 )
	{
		perror("shell: error in forking\n");
	}
	return 0;
}


//to execute the builtins commands...
int shell_execute_builtin(char **args, int i)
{
	
	if( i == 0 )
	{
		//if the dir is not specified...
		if (args[1] == NULL) 
		{
		    fprintf(stderr, "shell: expected argument to \"cd\"\n");
		}
		else
		{
		    if (chdir(args[1]) != 0) 
		    {
		      perror("shell: chdir");
    		    }
  		}
  	return 1;
	}
	
	if( i == 1 )
	{
		printf("\nshell: exiting shell\n");
		return 0;
	}

}


//function to get if the builtins command exists...
int shell_execute(char **args)
{
	int i;

	if( args[0] == NULL )
	{
		fprintf(stderr, "shell: an empty command was entered\n");
		return 1;
	}
	
	for( i = 0; i < builtin_length; i++ )
	{
		//if the current argument is a builtin...
		if( strcmp(args[0], builtin_str[i]) == 0 )
		{
			
			return (shell_execute_builtin(args, i));
		}
	}

	return shell_execvp_execute(args); 	
}


//function to execute the pipe arguments....
void execute_pipe(char **args1, char **args2)
{
	//array to get the input and output end of a pipe...                      
	//0 for read, 1 for write
	int pipefd[2];
	pid_t p1, p2;

	pipe(pipefd);//initialize the pipe..
	p1 = fork();
	if( p1 == 0 )
	{
		close(pipefd[0]);//process1 doenst need to read from pipe
		dup2(pipefd[1], STDOUT_FILENO);//write the file of process 1...
		close(pipefd[1]);//close the write end..
		execvp(args1[0], args1);
	}
	else
	{
		p2 = fork();
		if (p2 == 0)
                {
			close(pipefd[1]);//process2 does not need to write..
			dup2(pipefd[0], STDIN_FILENO);//read the from process1...
			close(pipefd[0]);
			execvp(args2[0], args2);
                }
		else
		{
			wait(NULL);
		}
	}
}


//function to read pipeline arguments...
void extract_pipe(char *buffer)
{
	//read the string upto | and after the |...
	
	int i = 0, pipe_count = 0, j = 0;

	while( buffer[i] != '\0' )
	{
		if( buffer[i] == '|' )
		{
			j = 0;
			i++;
			pipe_count++;
		}
		pipe_buffer[pipe_count][j++] = buffer[i];
		i++;
	}
}



int main(int argc, char **argv)
{
	char *buffer;
	char **args, **args1;
	int status;

	printf("\n--------------------------------------------------------------------------\n");
	printf("                      WELCOME TO THE VIVU SHELL                             \n");
	printf("               supports following command\n");
	printf("               ******< ls >\n");
	printf("               ******< cd >\n");
	printf("               ******< gcc >\n");
	printf("               ******< man >\n");
	printf("               ******< ls -l >\n");
	printf("               ******< | command >");
	printf("                \n\n\n"); 

	/*bsic procedure..
		read from the stdin..
		parse the arguments...
		execute the arguments...
	*/

	do{
		show_current_directory();
		buffer = read_line();
		
		
		if( pipe_flag == 1 )
		{
			extract_pipe(buffer);
			//tokenize the buffer...
			args = parse_input_buffer((char *)pipe_buffer[0]);
			args1 = parse_input_buffer((char *)pipe_buffer[1]);
	
			execute_pipe(args, args1);
			status = 1;
			pipe_flag = 0;//pipe has executed..
		}
		else
		{
			args = parse_input_buffer(buffer);
			status = shell_execute(args);
		}
	
	}while(status);
	
	return 0;
}
