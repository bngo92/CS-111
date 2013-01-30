// UCLA CS 111 Lab 1 command execution

#include "alloc.h"
#include "command.h"
#include "command-internals.h"

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
					//c->status = !(WIFEXITED(status) && WEXITSTATUS(status)==0);
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
