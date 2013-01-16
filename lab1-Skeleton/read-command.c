// UCLA CS 111 Lab 1 command reading
#include "command.h"
#include "command-internals.h"

#include <ctype.h>
#include <error.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "alloc.h"

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
  RIGHT_BRACKET,
  END
};

struct token
{
  char *word;
  enum token_type type;
  int line_number;
};

char* strdcpy(const char *src, size_t n)
{
  char* buffer = (char*) checked_malloc(n+1);
  strncpy(buffer, src, n);
  buffer[n]='\0';
  return buffer;
}

struct token *find_token(struct token *a, enum token_type t, int n)
{
  int i = 0;
  for (; i < n; i++)
    if (a[i].type == t)
      return a + i;
  return NULL;
}

struct token *find_last_token(struct token *a, enum token_type t, int n)
{
  int i = n-1;
  for (; i >= 0; i--)
    if (a[i].type == t)
      return a + i;
  return NULL;
}

struct token create_token(enum token_type type, int line_number)
{
  struct token ret;
  ret.type = type;
  ret.line_number = line_number;
  return ret;
}

struct token create_word_token(const char* s, int n, int line_number)
{
  struct token ret;
  ret.type = WORD;
  ret.word = strdcpy(s, n);
  ret.line_number = line_number;
  return ret;
}

void get_command(command_t c, struct token *s, int n);
int get_sequence(command_t c, struct token *s, int n);
int get_subshell(command_t c, struct token *s, int n);
int get_andor(command_t c, struct token *s, int n);
int get_pipeline(command_t c, struct token *s, int n);
void get_simple(command_t c, struct token *s, int n);

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
@precondition s is pointing to a left parenthesis
@return pointer to matching parenthesis
*/
struct token* find_matching_paren(struct token* s, int n)
{
  int line_number = -1;
  int depth = 0;
  int i = 0; 
  while(i < n) {
    if (s[i].type == LEFT_PAREN) {
      depth++;
    } else if (s[i].type == RIGHT_PAREN) {
      depth--;
    }
    line_number = s[i].line_number;
    i++;
  }
  if (depth != 0)
    error(1, 0, "line number: %d", line_number);
  return s+i;
}

/*
creates tree of commands parsing s, with c as the head command
@param  n   size of *s
*/
void get_command(command_t c, struct token *s, int n) 
{
  if (get_subshell(c, s, n))
    return;
  if (get_sequence(c, s, n))
    return;
  if (get_andor(c, s, n))
    return;
  if (get_pipeline(c, s, n))
    return;
  get_simple(c, s, n);
}

int get_sequence(command_t c, struct token *s, int n)
{
  struct token *found = find_token(s, SEMICOLON, n);
  if (found == NULL)
    return 0;
  
  c->type = SEQUENCE_COMMAND;
  c->status = -1;
  c->input = NULL;
  c->output = NULL;
  
  c->u.command[0] = (command_t) checked_malloc(sizeof(struct command));
  get_command(c->u.command[0], s, (found - s));
  c->u.command[1] = (command_t) checked_malloc(sizeof(struct command));
  get_command(c->u.command[1], found + 1, n - (found+1- s));
  return 1;
}

int get_subshell(command_t c, struct token *s, int n)
{
  if (s->type != LEFT_PAREN) {
    return 0;
  }
  if(find_matching_paren(s, n) != s + n) {
    return 0;
  }

  c->type = SUBSHELL_COMMAND;
  c->status = -1;
  c->input = NULL;
  c->output = NULL;
 
  c->u.subshell_command = (command_t) checked_malloc(sizeof(struct command*));
  get_command(c->u.subshell_command, s + 1, n - 2);
  return 1;
}

int get_andor(command_t c, struct token *s, int n) {
  struct token *found;
  struct token *found1 = find_last_token(s, AND, n);
  struct token *found2 = find_last_token(s, OR, n);
  if (found1 == NULL && found2 == NULL)
    return 0;
  else if (found1 != NULL && found2 != NULL)
    found = (found1 > found2) ? found1 : found2;
  else if (found1 != NULL)
    found = found1;
  else
    found = found2;
  
  if (found->type == AND)
    c->type = AND_COMMAND;
  else if (found->type == OR)
    c->type = OR_COMMAND;
  else
    error(1, 0, "line number: %d", found->line_number);

  c->status = -1; 
  c->input = NULL;
  c->output = NULL;

  c->u.command[0] = (command_t) checked_malloc(sizeof(struct command)); 
  get_command(c->u.command[0], s, found-s);
  c->u.command[1] = (command_t) checked_malloc(sizeof(struct command));
  get_command(c->u.command[1], found+1, n-(found+1-s));
  return 1;
}

int get_pipeline(command_t c, struct token *s, int n) {
  struct token *found = find_token(s, PIPE, n);
  if (found == NULL)
    return 0;

  c->type = PIPE_COMMAND;
  c->status = -1; 
  c->input = NULL;
  c->output = NULL;

  c->u.command[0] = (command_t) checked_malloc(sizeof(struct command)); 
  get_command(c->u.command[0], s, (found - s)); 
  c->u.command[1] = (command_t) checked_malloc(sizeof(struct command));
  get_command(c->u.command[1], found+1, n-(found+1 - s));

  return 1;

}

void get_simple(command_t c, struct token *s, int n) 
{
  if (n == 0)
    error(1, 0, "line number: %d", 42);

  c->type = SIMPLE_COMMAND;
  c->status = -1;
  c->input = NULL;
  c->output = NULL;

  size_t size = 1 * sizeof(char *);
  size_t word_count = 0;
  c->u.word = (char **) checked_malloc(sizeof(char *));
  int i = 0;
  if (s[i].type != WORD)
    error(1, 0, "line number: %d", s[i].line_number);
  while(s[i].type == WORD) {
    c->u.word[word_count] = s[i].word;
    word_count++;
    if (word_count * sizeof(char *) == size)
      c->u.word = (char **) checked_grow_alloc(c->u.word, &size);
    i++;
  }
  c->u.word[word_count] = NULL;

  if (s[i].type == LEFT_BRACKET) {
    i++;
    if (s[i].type != WORD)
      error(1, 0, "line number: %d", s[i].line_number);
    c->input = s[i].word;
    i++;
  }
  if (s[i].type == RIGHT_BRACKET) {
    i++;
    if (s[i].type != WORD)
      error(1, 0, "line number: %d", s[i].line_number);
    c->output = s[i].word;
    i++;
  }
  if (i != n)
    error(1, 0, "line number: %d", s[i].line_number);
}
       

struct command_stream
{
    command_t root;
};


command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  command_stream_t ret_stream = (command_stream_t) checked_malloc(sizeof(struct command_stream));

  size_t buffer_pos = 0;
  size_t buffer_size = 80;
  char *buffer = (char *) checked_malloc(buffer_size);
  memset(buffer, 0, buffer_size * sizeof(char));

  size_t token_pos = 0;
  size_t token_size = 10 * sizeof(struct token);
  struct token *tokens = (struct token *) checked_malloc(token_size);
  
  int line_number = 1;
  int newline = 0;
  char nextchar = (char) get_next_byte(get_next_byte_argument);
  while(nextchar != EOF) {
    //update buffer size as necessary
    if (token_pos * sizeof(struct token) == token_size) {
      tokens = checked_grow_alloc(tokens, &token_size);
    }
    if (nextchar=='#') {
      while(nextchar!='\n')
        nextchar = get_next_byte(get_next_byte_argument);
    } 
    if (isspace(nextchar)) {
      if (buffer_pos != 0) {
        tokens[token_pos] = create_word_token(buffer, buffer_pos, line_number);
        token_pos++;

        memset(buffer, 0, buffer_pos);
        buffer_pos = 0;
      }
      if (nextchar != '\n') {
        newline = 0;
      } else { enum token_type prev_token = tokens[token_pos-1].type;
        if (prev_token == SEMICOLON || prev_token == AND || prev_token == OR ||
            prev_token == PIPE || prev_token == LEFT_PAREN ||
            prev_token == RIGHT_PAREN) {
        } else if (prev_token == LEFT_BRACKET || prev_token == RIGHT_BRACKET) {
          error(1, 0, "line number: %d", line_number);
        } else {
          tokens[token_pos] = create_token(SEMICOLON, line_number);
          token_pos++;
        }
        line_number++;
        newline = 1;
      } 
    } else if (iswordchar(nextchar)) {
      buffer[buffer_pos] = nextchar;
      buffer_pos++;
      if (buffer_pos == buffer_size) {
        buffer = (char *) checked_grow_alloc(buffer, &buffer_size);
      }
      newline = 0;
    } else if (isToken(nextchar)) {
      if (buffer_pos != 0) {
        tokens[token_pos] = create_word_token(buffer, buffer_pos, line_number);
        token_pos++;

        memset(buffer, 0, buffer_pos);
        buffer_pos = 0;

        if (token_pos * sizeof(struct token) == token_size) {
          tokens = checked_grow_alloc(tokens, &token_size);
        }
      }
      if (tokens[token_pos-1].type != WORD && token_pos != 0)
        error(1, 0, "line number: %d", line_number);
      if (nextchar == '(') {
        tokens[token_pos] = create_token(LEFT_PAREN, line_number);
        token_pos++;
      } else if (nextchar == ')') {
        tokens[token_pos] = create_token(RIGHT_PAREN, line_number);
        token_pos++;
      } else if (newline) {
        error(1, 0, "line number: %d", line_number);
      } else if (nextchar == ';') {
        tokens[token_pos] = create_token(SEMICOLON, line_number);
        token_pos++;
      } else if (nextchar == '&') {
        nextchar = get_next_byte(get_next_byte_argument);
        if (nextchar != '&') {
          error(1, 0, "line number: %d", line_number);
        }
        tokens[token_pos] = create_token(AND, line_number);
        token_pos++;
      } else if (nextchar == '|') {
        nextchar = get_next_byte(get_next_byte_argument);
        if (nextchar == '|') {
          tokens[token_pos] = create_token(OR, line_number);
          token_pos++;
        } else {
          tokens[token_pos] = create_token(PIPE, line_number);
          token_pos++;
          continue;
        }
      } else if (nextchar == '<') {
        tokens[token_pos] = create_token(LEFT_BRACKET, line_number);
        token_pos++;
      } else if (nextchar == '>') {
        tokens[token_pos] = create_token(RIGHT_BRACKET, line_number);
        token_pos++;
      } else {
        error(1, 0, "line number: %d", line_number);
      }
      newline = 0;
    } else {
      error(1, 0, "line number: %d", line_number);
    }
    nextchar = get_next_byte(get_next_byte_argument);
  }

  if (buffer_pos != 0) {
    tokens[token_pos] = create_word_token(buffer, buffer_pos, line_number);
    token_pos++;
  }
  // optional following semicolon
  if (tokens[token_pos-1].type == SEMICOLON) {
    token_pos--;
  }
  if (token_pos == 0)
    error(1, 0, "line number: %d", line_number);
  ret_stream->root = (command_t) checked_malloc(sizeof(struct command));
  get_command(ret_stream->root, tokens, token_pos);
  free(tokens);
  free(buffer);
  return ret_stream;
}

command_t
read_command_stream (command_stream_t s)
{
  if (s == NULL || s->root == NULL)
    return NULL;

  command_t ret;
  if (s->root->type != SEQUENCE_COMMAND) {
    ret = s->root;
    s->root = NULL;
  } else {
    ret = s->root->u.command[0];
    s->root = s->root->u.command[1];
  }

  return ret;
}
