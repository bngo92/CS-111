// UCLA CS 111 Lab 1 command reading
#include "command.h"
#include "command-internals.h"

#include <error.h>

#include "alloc.h"
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

/* Make tree */
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

int line_number = 0;

enum token_type
{
  WORD,
  SEMICOLON,
  PIPE,
  AND,
  OR,
  LEFT_PAREN,
  RIGHT_PAREN,
  LEFT_BRACKET,
  RIGHT_BRACKET
};

struct token
{
  enum token_type type;
  int line_number;
};

void get_command(command_t c, char *s, int n);
int get_sequence(command_t c, char *s, int n);
int get_subshell(command_t c, char *s, int n);
int get_andor(command_t c, char *s, int n);
int get_pipeline(command_t c, char *s, int n);
void get_simple(command_t c, char *s, int n);

char* strdcpy(const char *src, size_t n)
{
  char* buffer = (char*) checked_malloc(n+1);
  strncpy(buffer, src, n);
  buffer[n]='\0';
  return buffer;
}

int iswordchar(char c)
{
  if (isalpha(c) || isdigit(c))
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

int isToken(char c)
{
    switch (c) {
    case ';':
    case '&':
    case '|':
    case '<':
    case '>':
    case '(':
    case ')':
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
    if (isspace(*ret))
      ret++;
    else {
      *s = ret;
      return 0;
    }
  }

  int i = 0;
  while (iswordchar(ret[i])) {
    i++;
  }
  *s = ret;
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
@precondition s is pointing to a left parenthesis
@return pointer to matching parenthesis
*/
char* find_matching_paren(char* s)
{
  s++; // skip starting paren

  int depth = 1;
  while (depth != 0) {
    s += strcspn(s, "()");
    if (*s == '\0') {
      error(1, 0, "line number: %d", line_number);
    } else if (*s == '(') {
      depth++;
      s++;
    } else {
      depth--;
      s++;
    }
  }
  return s;
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

  //TODO: errors occur if the entire line is a comment.
  /*char *comment = strchr(s, '#');
  if (comment != NULL) {
      *comment = '\0';
      n = comment - s;
  }
*/
  if (get_andor(c, s, n))
    return;

  if (get_pipeline(c, s, n))
    return;

  get_simple(c, s, n);
}

int get_sequence(command_t c, char *s, int n)
{
  while(isspace(*s))
  { 
    s++;
    n--;
  }
  while(isspace(s[n-1]))
  {
    n--;
  }

  char *buffer = strdcpy(s, n);
  
  char *found = buffer + strcspn(buffer, "(;\n");
  while (1) {
    if (*found == '\0') {
      free(buffer);
      return 0;
    } 
    else if (*found == '(') {
      found = find_matching_paren(found);
    } 
    else if (*found =='\n')
    {
      char prev = *( found-1 );
      if(prev!=';' && prev!='&'
      && prev!='|' && prev!='('
      && prev!=')' )
      {
        line_number++;
        break;
      }
      else if(prev=='<' || prev=='>')
        error(1, 0, "line number: %d", line_number);
      found += strcspn(found , "(;\n");
    }
    else
      break;
  }
  
  c->type = SEQUENCE_COMMAND;
  c->status = -1;
  c->input = NULL;
  c->output = NULL;
  
  c->u.command[0] = (command_t) checked_malloc(sizeof(struct command));
  get_command(c->u.command[0], buffer, (found - buffer));
  c->u.command[1] = (command_t) checked_malloc(sizeof(struct command));
  get_command(c->u.command[1], found + 1, n - (found - buffer));

  free(buffer);
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

  if (*s != '(') {
    return 0;
  }

  char *buffer = strdcpy(s, n);
  if(find_matching_paren(buffer) - buffer != n-1)
  {
    free(buffer);
    return 0;
  }

  c->type = SUBSHELL_COMMAND;
  c->status = -1;
  c->input = NULL;
  c->output = NULL;
 
  c->u.subshell_command = (command_t) checked_malloc(sizeof(struct command*));
  get_command(c->u.subshell_command, buffer + 1, n - 2);

  free(buffer);
  return 1;
}

int get_andor(command_t c, char *s, int n) {
  char *buffer = strdcpy(s, n);

  char *found = find_either(buffer, "&&", "||");
  if(found == NULL)
  {
    free(buffer);
    return 0;
  }
  
  if(*found == '&')
    c->type = AND_COMMAND;
  else if(*found == '|')
    c->type = OR_COMMAND;
  else
    error(1, 0, "line number: %d", line_number);

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
  char* buffer = strdcpy(s, n);

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
  char *buffer = strdcpy(s, n);
  char* save_ptr = buffer;

  c->type = SIMPLE_COMMAND;
  c->status = -1;
  c->input = NULL;
  c->output = NULL;

  size_t size = 0;
  int word_count = 0;
  c->u.word = (char **) checked_malloc(sizeof(char *));
  int i=0;
  while(1) {
    i = get_next_word(&buffer);
    if (i < 1) {
      if (word_count == 0) 
        error(1, 0, "line number: %d", line_number);
      if (*buffer == '\0' || *buffer == '<' || *buffer == '>')
        break;
      error(1, 0, "line number: %d", line_number);
    }
    
    word_count++;
    size = size + sizeof(char *);
    c->u.word = (char **) checked_realloc(c->u.word, size);
    c->u.word[word_count-1] = (char *) checked_malloc((i + 1) * sizeof(char));
    strncpy(c->u.word[word_count-1], buffer, i);
    
    c->u.word[word_count-1][i] = '\0';
    buffer = buffer+i;
  
  }
  if (*buffer == '<') {
    buffer++;
    i = get_next_word(&buffer);
    if (i < 1)
      error(1, 0, "line number: %d", line_number);

    c->input = (char *) checked_malloc((i + 1) * sizeof(char));
    strncpy(c->input, buffer, i);
    c->input[n] = '\0';
    buffer = buffer+i;
    while (isspace(*buffer))
      buffer++;
  }

  if (*buffer == '>') {
    buffer++;
    int i = get_next_word(&buffer);
    if (i < 1)
      error(1, 0, "line number: %d", line_number);

    c->input = (char *) checked_malloc((i + 1) * sizeof(char));
    strncpy(c->input, buffer, i);
    c->input = '\0';
    buffer = buffer+i;
  }

  c->u.word[word_count] = NULL;

  while (isspace(*buffer))
    buffer++;

  if (*buffer != '\0')
    error(1, 0, "line number: %d", line_number);

  free(save_ptr);
}
       

struct command_stream
{
    command_t root;
    int i;
};


command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  command_stream_t ret_stream = (command_stream_t) checked_malloc(sizeof(struct command_stream));
  int size = 10;
  char* buf = (char*) checked_malloc((size_t) size);
  int pos = 0;
  int nextchar =  get_next_byte(get_next_byte_argument);
  while( nextchar != -1) 
  {
    //update buffer size as necessary
    if(pos==size)
    {
      checked_realloc(buf, (size_t)(2*size) );
      size = 2*size;
    }
    //squeeze whitespace
    if(nextchar=='\n' && isspace(buf[pos-1]))
    {
       buf[pos-1]=nextchar;
    }
    else if ((nextchar=='\t' || nextchar==' ') && isspace(buf[pos-1]))
    {
    }
    //remove comments
    else if (nextchar=='#')
    {
        while(nextchar!='\n')
            nextchar = get_next_byte(get_next_byte_argument);

        nextchar = get_next_byte(get_next_byte_argument);
    }
    else
    {
        buf[pos] = (char) nextchar;
        pos++;
    }
    nextchar = get_next_byte(get_next_byte_argument);
  }
 
  if(pos==size)
  {
    checked_realloc(buf, (size_t)(size+1));
    size++;
  }

  //trailing space?
  buf[pos] = '\0'; 
  int bt = pos-1;
  while (isspace(buf[bt]) || buf[bt] == ';') {
    buf[bt] = '\0';
    bt--;
  }

  ret_stream->i = 0;
  ret_stream->root = (command_t) checked_malloc(sizeof(struct command));
  get_command(ret_stream->root, buf, pos );


  return ret_stream;
}

command_t
read_command_stream (command_stream_t s)
{
  if (s->i == 0) {
    s->i++;
    return s->root;
  }
  return NULL;
}
