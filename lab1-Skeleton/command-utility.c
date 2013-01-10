#include "command-internals.h"
#include "command-utility.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>


int iswordchar(char c)
{
  if (isalpha(c))
    return 1;
  switch (c) {
  case '!':
  case '%':
  case '+':
  case ',':
  case '-':
  case '.':
  case '/':
  case ':':
  case '@':
  case '^':
  case '_':
    return 1;
  default:
    return 0;
  }
}

/*
@postcondition  *s is the beginning of the word, or the next token char if 
                no word is found
@return length of word, or 0 if no word is found
*/
int get_next_word(char **s)
{
  char *ret = *s;
  //nullbyte at end of buffer
  while (!iswordchar(*ret)) {
    if (isspace(*ret)) {
      ret++;
    } else {
      *s = ret;
      return 0;
    }
  }

  int i = 0;
  while (iswordchar(ret[i])) {
    i++;
  }
  return i;
}

/*
@return pointer to next char of s that is either *s1 or *s2
*/
char* find_either(char *s, char *s1, char *s2)
{
  char *ret1 = strstr(s, s1);
  char *ret2 = strstr(s, s2);

  if (ret1 == NULL && ret2 == NULL)
    return NULL;
  else if (ret1 != NULL && ret2 != NULL)
    return (ret1 < ret2) ? ret1 : ret2;
  else if (ret1 != NULL)
    return ret1;
  else
    return ret2;
}

/*
creates tree of commands parsing s, with c as the head command
@param  n   size of *s
*/
void get_command(command_t c, char *s, int n) 
{
  if (get_subshell(c, s, n))
    return;
  
  if (get_sequence(c, s, n))
    return;

  char *comment = strchr(s, '#');
  if (comment != NULL) {
      *comment = '\0';
      n = comment - s;
  }

  if (get_andor(c, s, n))
    return;

  if (get_pipeline(c, s, n))
    return;

  get_simple(c, s, n);
}

int get_sequence(command_t c, char *s, int n)
{
  char* buffer = (char*)checked_malloc((size_t)(n+1));
  char* temp;
  strncpy(buffer, s, n);
  buffer[n] = '\0';
  char* save_ptr = buffer;
  while(isspace(*buffer))
  { 
    buffer++;
    n--;
  }

  if (buffer[0]='(')
  {
    //find matching parentheses
    int count = 1; 
    temp = buffer+1;
    while(count!=0)
    {
        temp = find_either(temp, "(",")");
        if (*temp = '(')
            count++;
        else if (*temp = ')')
            count --; 
        else 
            error(1, 0, "subshell syntax");
    }
    
    c->type = SEQUENCE_COMMAND;
    c->status = -1; 
    c->input = NULL;
    c->output = NULL;
    
    c->u.command[0]= (command_t) checked_malloc(sizeof(struct command)); 
    get_command(c->u.command[0], buffer, temp-buffer+1);
    c->u.command[1]= (command_t) checked_malloc(sizeof(struct command));
    get_command(c->u.command[1], temp+1, n-(temp-buffer+1)); 
  }
  char *found = find_either(buffer, ";", "\n");
  if (found == NULL)
  {
    free(save_ptr);
    return 0;
  }
  
  c->type = SEQUENCE_COMMAND;
  c->status = -1;
  c->input = NULL;
  c->output = NULL;
  
  c->u.command[0] = (command_t) checked_malloc(sizeof(struct command));
  get_command(c->u.command[0], buffer, (found - buffer));
  c->u.command[1] = (command_t) checked_malloc(sizeof(struct command));
  get_command(c->u.command[1], found + 1, n - (found - buffer));

  free(save_ptr);
  return 1;
}

int get_subshell(command_t c, char *s, int n)
{
  // strip leading and trailing whitespace
  while (isspace(s[0])) {
    s++;
    n--;
  }
  while (isspace(s[n-1])) {
    n--;
  }

  char *buffer = (char *) checked_malloc(n+1);
  strncpy(buffer, s, n);
  buffer[n] = '\0';
  char* save_ptr = buffer;

  if (buffer[0] != '(')
  {
    free(buffer);
    return 0;
  }

  char *tmp = strchr(buffer, ')');
  if (tmp == NULL)
    error(1, 0, "subshell syntax");

  if (s[n-1] != ')')
    return 0;

  c->type = SUBSHELL_COMMAND;
  c->status = -1;
  c->input = NULL;
  c->output = NULL;
  
  c->u.subshell_command = (command_t) checked_malloc(sizeof(struct command*));
  get_command(c->u.subshell_command, buffer + 1, n - 2);
  free(save_ptr);
  return 1;
}

int get_andor(command_t c, char *s, int n) {
  char* buffer = (char*) checked_malloc(n+1);
  strncpy(buffer, s, n);
  buffer[n]='\0';
    
  char *found = find_either(buffer, "&&", "||");
  if(found == NULL)
  {
    free(buffer);
    return 0;
  }
  
  if(*found = '&')
    c->type = AND_COMMAND;
  else if(*found = '|')
    c->type = OR_COMMAND;
  else
    error(1, 0, "AND_OR syntax; find function misbehaved");

  c->status = -1; 
  c->input = NULL;
  c->output = NULL;

  c->u.command[0] = (command_t) checked_malloc(sizeof(struct command)); 
  get_command(c->u.command[0], buffer, found-buffer);
  c->u.command[1] = (command_t) checked_malloc(sizeof(struct command));
  get_command(c->u.command[1], found+2, n-(found+2-buffer));

  free(buffer);

  return 1;
}


int get_pipeline(command_t c, char *s, int n) {
  char* buffer = (char*) checked_malloc(n+1);
  strncpy(buffer, s, n);
  buffer[n]='\0';

  char* found = strchr(buffer, '|');
  if(found==NULL)
  {
    free(buffer);
    return 0;
  }

  c->type = PIPE_COMMAND;
  c->status = -1; 
  c->input = NULL;
  c->output = NULL;

  c->u.command[0] = (command_t) checked_malloc(sizeof(struct command)); 
  get_command(c->u.command[0], buffer, (found - buffer)); 
  c->u.command[1] = (command_t) checked_malloc(sizeof(struct command));
  get_command(c->u.command[1], found+1, n-(found - buffer));

  free(buffer);
  return 1;

}


void get_simple(command_t c, char *s, int n) 
{
  char *buffer = (char *) checked_malloc(n+1);
  strncpy(buffer, s, n);
  buffer[n] = '\0';

  c->type = SIMPLE_COMMAND;
  c->status = -1;
  c->input = NULL;
  c->output = NULL;

  int word_count = 0;
  while(1) {
    int n = get_next_word(&buffer);
    if (n < 1) {
      if (word_count == 0) 
        error(1, 0, "simple syntax");
      if (*buffer == '\0' || *buffer == '<' || *buffer == '>')
        break;
    }

    c->u.word = (char **) checked_realloc(word_count * sizeof(char *));
    c->u.word[word_count] = (char *) checked_malloc((n + 1) * sizeof(char));
    strncpy(c->u.word[word_count], s, n);
    c->u.word[word_count][n] = '\0';
    word_count++;
  
  }
  if (*buffer == '<') {
    buffer++;
    if (get_next_word(&buffer) == 0)
      error(1, 0, "simple syntax");

    c->u.word = (char **) checked_realloc(word_count * sizeof(char *));
    c->u.word[word_count] = (char *) checked_malloc((n + 1) * sizeof(char));
    strncpy(c->u.word[word_count], s, n);
    c->u.word[word_count][n] = '\0';
    word_count++;
  }

  if (*buffer == '>') {
    buffer++;
    if (get_next_word(&buffer) == 0)
      error(1, 0, "simple syntax");

    c->u.word = (char **) checked_realloc(word_count * sizeof(char *));
    c->u.word[word_count] = (char *) checked_malloc((n + 1) * sizeof(char));
    strncpy(c->u.word[word_count], s, n);
    c->u.word[word_count][n] = '\0';
    word_count++;
  }

  while (isspace(*buffer))
    buffer++;

  if (*buffer != '\0')
    error(1, 0, "simple syntax");

  free(buffer);
}
