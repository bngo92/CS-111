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

void fill_input(command_t c, char *s)
{
    switch (c->type) {
    case AND_COMMAND:
    case SEQUENCE_COMMAND:
    case OR_COMMAND:
    case PIPE_COMMAND:
      {
	fill_output(c->u.command[0], s);
	break;
      }
    case SIMPLE_COMMAND:
      {
	if (c->input == NULL)
	  c->input = s;
	break;
      }

    case SUBSHELL_COMMAND:
      {
	fill_output(c->u.subshell_command, s);
	break;
      }
    }
}

void fill_input(command_t c, char *s)
{
    switch(c->type){
	case AND_COMMAND:
	case SEQUENCE_COMMAND:
	case OR_COMMAND:
	    fill_input(c->u.command[1],s);
	case PIPE_COMMAND:
	{
	    fill_input(c->u.command[0],s);
	    break;
	}
	case SIMPLE_COMMAND:
	{
	    if (c->input == NULL)
		c->input = s;
	    break;
	}
	case SUBSHELL_COMMAND:
	{
	    fill_input(c->u.subshell_command, s);
	    break;
	}
    }

}

void fill_output(command_t c, char *s) 
{
    switch (c->type) {
    case AND_COMMAND:
    case SEQUENCE_COMMAND:
    case OR_COMMAND:
	fill_output(c->u.command[0], s);
    case PIPE_COMMAND:
      {
	fill_output(c->u.command[1], s);
	break;
      }
    case SIMPLE_COMMAND:
      {
	if (c->output == NULL)
	  c->output = s;
	break;
      }

    case SUBSHELL_COMMAND:
      {
	fill_output(c->u.subshell_command, s);
	break;
      }
    }
}

static void
command_print (command_t c)
{
  static int file_counter = 1000;
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
	char buffer[80];
	sprintf(buffer, "%d.tmp", getpid());
        if ((c->u.command[0]->type == SUBSHELL_COMMAND || c->u.command[0].type == SIMPLE_COMMAND) &&
			((c->u.command[1]->type == SUBSHELL_COMMAND || c->u.command[1].type == SIMPLE_COMMAND)) {
	if (c->u.command[0]->output == NULL && c->u.command[1]->input == NULL) {
	  c->u.command[0]->output = checked_malloc(strlen(buffer) + 1);
	  strcpy(c->u.command[0]->output, buffer);
	  c->u.command[1]->input = checked_malloc(strlen(buffer) + 1);
	  strcpy(c->u.command[1]->input, buffer);
	}
	command_print(c->u.command[0]);
	command_print(c->u.command[1]);
	c->status = c->u.command[1]->status;
	remove(buffer);
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
