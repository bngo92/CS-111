// UCLA CS 111 Lab 1 command execution

#include "alloc.h"
#include "command.h"
#include "command-internals.h"
#include "limit-parallel.h"

#include <error.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wait.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

int
command_status (command_t c)
{
  return c->status;
}

static void
command_print (command_t c)
{
  switch (c->type)
    {
    case AND_COMMAND:
      {
	command_print(c->u.command[0]);
	if (c->u.command[0]->status != 0) {
	  c->status = c->u.command[0]->status;
	  return;
	} 
	command_print(c->u.command[1]);
	c->status = c->u.command[1]->status;
	break;
      }
    case SEQUENCE_COMMAND:
      {
	command_print(c->u.command[0]);
	command_print(c->u.command[1]);
	c->status = c->u.command[1]->status;
	break;
      }
    case OR_COMMAND:
      {
        command_print(c->u.command[0]);
	if (c->u.command[0]->status == 0) {
	  c->status = c->u.command[0]->status;
	  return;
	}
	command_print(c->u.command[1]);
	c->status = c->u.command[1]->status;
	break;
      }
		case PIPE_COMMAND:
			{
				int fd[2];
				fd[0] = dup(STDIN_FILENO);
				fd[1] = dup(STDOUT_FILENO);
				int pipefd[2];
				pipe(pipefd);
				if (fork() == 0) { // write to pipe
					close(pipefd[0]);
					dup2(pipefd[1], STDOUT_FILENO);
					close(pipefd[1]);
					command_print(c->u.command[0]);
					_exit(0);
				} else { // read from pipe
					wait(NULL);
				}
				int status;
				close(pipefd[1]);
				dup2(pipefd[0], STDIN_FILENO);
				close(pipefd[0]);
				if (fork() == 0) {
					command_print(c->u.command[1]);
					_exit(c->u.command[1]->status);
				} else {
					wait(&status);
				}
				dup2(fd[0], STDIN_FILENO);
				dup2(fd[1], STDOUT_FILENO);
				c->status = WEXITSTATUS(status);
				break;
			}
		case SIMPLE_COMMAND:
			{
				int status = 0;
				if (strcmp(c->u.word[0], "exec") == 0) {
					int in, out;
					if (c->input) {
						in = open(c->input, O_RDONLY);
						dup2(in, STDIN_FILENO);
						close(in);
					}
					if (c->output) {
						out = creat(c->output, S_IRUSR|S_IWUSR);
						dup2(out, STDOUT_FILENO);
						close(out);
					}
					execvp(c->u.word[1], c->u.word+1);
					_exit(-1);
				}
				else {
					if (fork() == 0) {
						int in, out;
						if (c->input) {
							in = open(c->input, O_RDONLY);
							dup2(in, STDIN_FILENO);
							close(in);
						}
						if (c->output) {
							out = creat(c->output, S_IRUSR|S_IWUSR);
							dup2(out, STDOUT_FILENO);
							close(out);
						}
						execvp(c->u.word[0], c->u.word);
						_exit(-1);
					} else {
						wait(&status);
					}
					c->status = WEXITSTATUS(status);
				}
				break;
			}
		case SUBSHELL_COMMAND:
			{
				if (fork() == 0) {
					int fd[2];
					fd[0] = dup(STDIN_FILENO);
					fd[1] = dup(STDOUT_FILENO);
					int in, out;
					if (c->input) {
						in = open(c->input, O_RDONLY);
						dup2(in, STDIN_FILENO);
						close(in);
					}
					if (c->output) {
						out = creat(c->output, S_IRWXU);
						dup2(out, STDOUT_FILENO);
						close(out);
					}
					command_print(c->u.subshell_command);
					dup2(fd[0], STDIN_FILENO);
					dup2(fd[1], STDOUT_FILENO);
					_exit(c->u.subshell_command->status);
				} else {
					int status;
					wait(&status);
					c->status = WEXITSTATUS(status);
				}
				break;
			}
		}
}

void
execute_command (command_t c, bool time_travel)
{
  command_print(c);
}

/****************************************************************************
TIME TRAVEL VERSION
***************************************************************************/
enum p_status
{
    NOT_RAN,
    RUNNING,
    RAN
};

struct d_pair
{
    command_t c; 
    char **input;
    size_t input_size;
    size_t input_max;
    char **output;
    size_t output_size;
    size_t output_max;
    enum p_status ran; //bool
};


void fill(struct d_pair *pair, command_t c)
{
    if (c->input) {
        pair->input[pair->input_size] = c->input;
        pair->input_size++;
        if (pair->input_size * sizeof(char *) >= pair->input_max/ 2)
            pair->input = checked_grow_alloc(pair->input, &pair->input_max);
    }

    switch (c->type) {
    case AND_COMMAND:
    case OR_COMMAND:
    case PIPE_COMMAND:
    case SEQUENCE_COMMAND:
        fill(pair, c->u.command[0]);
        fill(pair, c->u.command[1]);
        break;
    case SUBSHELL_COMMAND:
        fill(pair, c->u.subshell_command);
        break;
    case SIMPLE_COMMAND:
    {
        char **temp = c->u.word;
        while (temp != NULL && *temp!=NULL) {
            pair->input[pair->input_size] = *temp;
            pair->input_size++;
            if (pair->input_size * sizeof(char *) >= pair->input_max/ 2)
                pair->input = checked_grow_alloc(pair->input, &pair->input_max);
            temp++;
        }

        if (c->output) {
            pair->output[pair->output_size] = c->output;
            pair->output_size++;
            if (pair->output_size * sizeof(char *) >= pair->output_max/ 2)
                pair->output = checked_grow_alloc(pair->output, &pair->output_max);
        }
    }
    default:
        pair->input[pair->input_size] = NULL;
        pair->output[pair->output_size] = NULL;
        break;
    }
}

int check_match(char **s1, char **s2)
{
    while (s1 != NULL && *s1 != NULL) {
        char **temp = s2;
        while (temp != NULL && *temp != NULL) {
            if (strcmp(*s1, *temp) == 0)
                return 1;
            temp++;
        }
        s1++;
    }
    return 0;
}

/* 
@precondition	function assumes time_travel has been called.
@postcondition	all commands in command_stream have been run using time travel to maximize speed

@return			status of "last command" 
*/
int execute_command_stream(command_stream_t command_stream) //DESIGN PROBLEM: , int max_processes)
{
   
    //DESIGN PROBLEM  TODO: Implement counter for max processes; 
    int size = 0;
    size_t max = 10 * sizeof(struct d_pair);
    struct d_pair* c_array = (struct d_pair*) checked_malloc( max * sizeof(struct d_pair) ); 
	
    //read all commands
    //get input/output
    command_t command = NULL; 
    while ( ( command = read_command_stream (command_stream) ) ) {
        struct d_pair *pair = &c_array[size];
	    pair->c = command; 
	    pair->ran = -1;
	    pair->input_size=0;
	    pair->output_size=0;
        pair->input_max = 10 * sizeof(char *);
        pair->output_max = 10 * sizeof(char *);
        pair->input = (char **) checked_malloc(pair->input_max);
        pair->output = (char **) checked_malloc(pair->output_max);
        pair->ran = NOT_RAN;

        fill(pair, pair->c);
		size++;
        if (size * sizeof(pair) >= max / 2)
            c_array = checked_grow_alloc(c_array, &max);
    }

    //if null stream, return 0 as status
    if(size==0) {
        return 0;
    }
    
    //setup pid array
    int* pid_set = (int*) checked_malloc(size*sizeof(int)); 
    memset(pid_set, 0, size*sizeof(int));
    
    int start = 0;
    while (start != size) {
        int i;
        for (i = start; i < size; i++) {
            if (c_array[i].ran != NOT_RAN)
                continue;

            // check for dependencies
            int j;
            int fail = 0;
            for (j = start; j < i; j++) {
                if (c_array[j].ran != RAN) {
                    if (check_match(c_array[i].input, c_array[j].input) ||
                            check_match(c_array[i].input, c_array[j].output)) {
                        fail = 1;
                        break;
                    }
                }
            }
            if (!fail) {
                pid_t pid = limitfork();
                if (pid == -1) {
			break;
		} else if(pid == 0) {
                    command_print(c_array[i].c);
                    _exit(c_array[i].c->status);
                } else {
                    c_array[i].ran = RUNNING;
                    pid_set[i] = pid;
                }
            }
        }

        int b = 0;
        for (i = start; i < size; i++) {
            if (c_array[i].ran != RUNNING)
                continue;
            if (limitwaitpid(pid_set[i], NULL, WNOHANG) > 0) {
                c_array[i].ran = RAN;
                b = 1;
            }
        }

        // skip already ran processes
        while (c_array[start].ran == RAN) {
            start++;
        }
    }

    return c_array[start-1].c->status;
}

