// UCLA CS 111 Lab 1 command execution

#include "alloc.h"
#include "command.h"
#include "command-internals.h"

#include <error.h>
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
	if (!c->u.command[0]->status) {
	  c->status = 0;
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
	break;
      }
    case OR_COMMAND:
      {
        command_print(c->u.command[0]);
	if (c->u.command[0]->status) {
	  c->status = 1;
	  return;
	}
	command_print(c->u.command[1]);
	c->status = c->u.command[1]->status;
	break;
      }
    case PIPE_COMMAND:
      {
	if (c->u.command[0]->output == NULL && c->u.command[1]->input == NULL) {
	  char buffer[80];
	  sprintf(buffer, "%d.tmp", getpid());
	  c->u.command[0]->output = checked_malloc(strlen(buffer) + 1);
	  strcpy(c->u.command[0]->output, buffer);
	  c->u.command[1]->input = checked_malloc(strlen(buffer) + 1);
	  strcpy(c->u.command[1]->input, buffer);
	}
	command_print(c->u.command[0]);
	if (c->u.command[0]->status)
	  command_print(c->u.command[1]);
	break;
      }
    case SIMPLE_COMMAND:
      {
	int status = 0;
	pid_t pid = fork();
	if (pid == 0) {
	  if (c->input)
	    freopen(c->input, "r", stdin);
	  if (c->output)
	    freopen(c->output, "w", stdout);
  	  execvp(c->u.word[0], c->u.word);
	  _exit(-1);
	} else {
	  wait(&status);
	}
	c->status = WIFEXITED(status) && !WEXITSTATUS(status);
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
