#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#define COMMAND_BUF_SIZE 500
#define SHELL_NEWLINE_SYMBOL "\n>> "
#define MAX_NUM_PIPES 10
#define MAX_NUM_ARGUMENTS 10
#define MAX_ARG_LENGTH 50
#define RANDOM_STRING "QwErTyUiOpAsDfGh"
#define RANDOM_STRING_LENGTH 16
#define NAME_LENGTH 4
#define TEMP_FILE_DIR "/tmp/"
#define TEMP_FILE_EXTENSION ".txt"
typedef void (*exec_func)();
typedef struct command {
	exec_func func;
	int num_files;
	char* files[MAX_NUM_PIPES][MAX_NUM_ARGUMENTS];
}command;
int i,j,num_bytes_read;
int close_fds[MAX_NUM_PIPES], dup_fds[MAX_NUM_PIPES], multi_pipe_output[MAX_NUM_PIPES];
int pipes[MAX_NUM_PIPES][2];
char command_buf[COMMAND_BUF_SIZE];
command cmd;
char pipe_buf[PIPE_BUF];
char temp_file_name[50];
char name_buf[MAX_NUM_PIPES][NAME_LENGTH];
extern char **environ;
void get_file_name(int buf_index, int create_new)
{
	for(j=0;j<NAME_LENGTH;j++)
	{
		if(create_new)
			name_buf[buf_index][j] = RANDOM_STRING[rand()%RANDOM_STRING_LENGTH];
		temp_file_name[strlen(TEMP_FILE_DIR)+j] = name_buf[buf_index][j];
	}
	temp_file_name[strlen(TEMP_FILE_DIR)+NAME_LENGTH+strlen(TEMP_FILE_EXTENSION)] = '\0';
}
void print_command(int file_index)
{
	printf("\nCommand:");
	for(j=0;cmd.files[file_index][j]!=0;j++)
		printf("%s ",cmd.files[file_index][j]);
	printf("\n\n");
}
void handle_delimiter(int i, int* first_char, int next_file, int* next_argument)
{
	if(*first_char == 0)
	{
		*first_char = 1;
		command_buf[i] = '\0';
		(*next_argument)++;
	}
}
void wrap_argument(int next_file, int next_argument)
{
	cmd.files[next_file][next_argument] = NULL;
}
void new_file(int* next_file, int* next_argument, int* first_char)
{
	(*next_file)++;
	*next_argument = 0;
	*first_char = 1;
}
void fork_and_execute(int file_index, int num_close, int num_dup)
{
	if(fork()==0)
	{
		for(i=0;i<num_close && i < num_dup;i++)
		{
			close(close_fds[i]);
			dup(dup_fds[i]);
		}
		for(;i<num_close;i++)
			close(close_fds[i]);
		execvp(cmd.files[file_index][0], cmd.files[file_index]);
	}
}
void redirect_input()
{
	close_fds[0] = 0;
	dup_fds[0] = fileno(fopen(cmd.files[1][0], "r"));
	fork_and_execute(0,1,1);
	while(wait(NULL)>0);	
}
void append_output()
{
	close_fds[0] = 1;
	dup_fds[0] = fileno(fopen(cmd.files[1][0], "a"));
	fork_and_execute(0,1,1);
	while(wait(NULL)>0);
}
void overwrite_output()
{
	close_fds[0] = 1;
	dup_fds[0] = fileno(fopen(cmd.files[1][0], "w"));
	fork_and_execute(0,1,1);
	while(wait(NULL)>0);
}
void multi_pipe()
{
	int num_files = cmd.num_files - 1;
	pipe(pipes[0]);
	close_fds[0] = 1;
	dup_fds[0] = pipes[0][1];
	fork_and_execute(0,1,1);
	close(pipes[0][1]);
	close_fds[0] = 0;
	print_command(1);
	for(i=1;i<=num_files;i++)
	{
		pipe(pipes[i]);
		dup_fds[0] = pipes[i][0];
		if(i > 1)
		{
			if(i==2)
				close_fds[1] = 1;
			get_file_name(i-2,1);
			multi_pipe_output[i-2] = fileno(fopen(temp_file_name,"w"));
			dup_fds[1] = multi_pipe_output[i-2];
		}
		for(j=1;j<=i;j++)
			close_fds[(i==1?j:j+1)] = pipes[j][1];
		fork_and_execute(i,(i==1?2:i+2),(i==1?1:2));
		close(pipes[i][0]);	
		if(i>1)
			close(multi_pipe_output[i-2]);
	}
	while((num_bytes_read = read(pipes[0][0],pipe_buf,PIPE_BUF))!=0)
	{
		for(i=1;i<=num_files;i++)
			write(pipes[i][1],pipe_buf,num_bytes_read);
	}
	close(pipes[0][0]);
	for(i=1;i<=num_files;i++)
		close(pipes[i][1]);
	while(wait(NULL)>0);
	for(i=0;i<num_files-1;i++)
	{
		get_file_name(i,0);
		multi_pipe_output[0] = fileno(fopen(temp_file_name,"r"));
		print_command(i+2);
		while((num_bytes_read = read(multi_pipe_output[0],pipe_buf,PIPE_BUF))!=0)
			write(1,pipe_buf,num_bytes_read);
		close(multi_pipe_output[0]);
		remove(temp_file_name);
	}

}
void single_pipe()
{
	int read_pipe;
	int write_pipe = 0;
	int num_files = cmd.num_files;	
	close_fds[0] = 1;
	for(i=0;i<num_files;i++)
	{
		if(i == 0)
		{
			pipe(pipes[write_pipe]);
			dup_fds[0] = pipes[write_pipe][1];
		}
		else
		{
			close(pipes[read_pipe][1]);
			if(i == 1)
				close_fds[1] = 0;
			else close(pipes[write_pipe][0]);
			if(i<num_files-1)
			{
				pipe(pipes[write_pipe]);
				dup_fds[0] = pipes[write_pipe][1];
				dup_fds[1] = pipes[read_pipe][0];
			}
			else
			{
				close_fds[0] = 0;
				dup_fds[0] = pipes[read_pipe][0];
			}
		}
		fork_and_execute(i,(i==0 || i==num_files-1?1:2),(i==0 || i==num_files-1?1:2));
		read_pipe = write_pipe;
		write_pipe = (write_pipe+1)%2;
	}
	while(wait(NULL)>0);
}
/*void triple_pipe()
{
	multi_pipe_aux(3);
}
void double_pipe()
{
	multi_pipe_aux(2);
}*/
void simple_exec()
{
	fork_and_execute(0,0,0);
	while(wait(NULL)>0);
}
void process_command()
{
	cmd.func = NULL;
	int first_char = 1;
	int next_file = 0;
	int next_argument = 0;
	int command_length = strlen(command_buf);
	for(i=0;i<command_length;i++)
	{
		switch(command_buf[i])
		{
			case ' ':
				handle_delimiter(i, &first_char, next_file, &next_argument);
				break;
			case ',':
				handle_delimiter(i, &first_char, next_file, &next_argument);
				wrap_argument(next_file, next_argument);
				new_file(&next_file, &next_argument, &first_char);
				break;
			case '\n':
				handle_delimiter(i, &first_char, next_file, &next_argument);
				wrap_argument(next_file, next_argument);
				break;
			case '<':
				cmd.func = redirect_input;
				handle_delimiter(i, &first_char, next_file, &next_argument);
				wrap_argument(next_file, next_argument);
				new_file(&next_file, &next_argument, &first_char);
				break;
			case '>':
				cmd.func = command_buf[i+1] == '>' ? append_output : overwrite_output;
				handle_delimiter(i, &first_char, next_file, &next_argument);
				i = i + (command_buf[i+1] == '>' ? 1 : 0);
				wrap_argument(next_file, next_argument);
				new_file(&next_file, &next_argument, &first_char);
				break;
			case '|':
				cmd.func = command_buf[i+1] == '|' ? multi_pipe : single_pipe;
				handle_delimiter(i, &first_char, next_file, &next_argument);
				for(;command_buf[i+1]=='|';i++);
				wrap_argument(next_file, next_argument);
				new_file(&next_file, &next_argument, &first_char);
				break;
			default:
				if(first_char == 1)
				{
					cmd.files[next_file][next_argument] = &command_buf[i];
					first_char = 0;
				}
		}
	}
	if(!cmd.func)
		cmd.func = simple_exec;
	cmd.num_files = next_file + 1;
}
void shell_loop()
{
	for(;;)
	{
		printf(SHELL_NEWLINE_SYMBOL);
		fgets(command_buf, COMMAND_BUF_SIZE, stdin);
		process_command();
		cmd.func();
	}
}
int main()
{
	strcpy(temp_file_name, TEMP_FILE_DIR);
	strcpy(&temp_file_name[strlen(TEMP_FILE_EXTENSION)+NAME_LENGTH+1],TEMP_FILE_EXTENSION);
	shell_loop();
	return EXIT_SUCCESS;
}