// UCLA CS 111 Lab 1 command execution

#include "alloc.h"
#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
	pipe(fd);
	if (fork() == 0) {
		close(fd[1]);
		dup2(fd[0], STDOUT_FILENO);
		close(fd[0]);
		command_print(c->u.command[0]);
	} else {
		close(fd[0]);
		dup2(fd[1], STDIN_FILENO);
		close(fd[1]);
		
		command_print(c->u.command[1]);
	}
	c->status = c->u.command[1]->status;
	break;
      }
    case SIMPLE_COMMAND:
      {
	int status = 0;
	pid_t pid = fork();
	if (pid == 0) {
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
  	  execvp(c->u.word[0], c->u.word);
	  _exit(-1);
	} else {
	  wait(&status);
	}
	c->status = WEXITSTATUS(status);
	//c->status = !(WIFEXITED(status) && WEXITSTATUS(status)==0);
	break;
      }

    case SUBSHELL_COMMAND:
      command_print(c->u.subshell_command);
      c->status = c->u.subshell_command->status;
      break;
    }
}

void
execute_command (command_t c, bool time_travel)
{
  command_print(c);
}
