// UCLA CS 111 Lab 1 command reading
#include "command.h"
#include "command-internals.h"

#include <error.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

#include "alloc.h"
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

char* findChar(char* s, char c) {
    while (*s != '\0') {
        if (*s == c)
            return s;
        s++;
    }
    return NULL;
}

char* findLastChar(char* s, char c) {
    char* temp = s + strlen(s) - 1;
    while (*s != temp) {
        if (*s == c)
            return s;
        s--;
    }
    return NULL;
}

static bool isWordChar(char c)
{
    if(isalpha(c))
        return true; 
    switch (c) 
    {
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
            return true;
        default:
            return false;
    }

}

int getNextSubshell(char** p_start, char** p_end)
{
    char* start = *p_start;
    char* end = *p_end;
    
    while (isspace(start)) {
        if (start == end)
            return -1;
        start++;
    }

    if (*start == '(') {
        end = findLastChar(start, ')');
        if (end == NULL)
            error(1,0,"invalid syntax");
        return -1;
    }

    p_start = &start;
    p_end = &end;
    return 0;
}

static char* getNextWord(char* buf)
{
    char* start = buf;
    while (isspace(start)) {
        // trailing whitespace
        if (*start = NULL)
            return NULL;
        start++;
    }

    // not a word
    if (!isWordChar(start)) {
        return NULL;
    } 

    int i = 0;
    while (isWordChar(start[i])) {
        i++;
    }
    char* ret = checked_malloc(i+1);
    strncpy(ret, start, i);
    ret[i]='\0';
    return ret;
}

       
/* FIXME: Define the type 'struct command_stream' here.  This should complete the incomplete type declaration in command.h.  */

struct command_stream
{
    command_t buffer;
    int index;
    int size;
};


command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  command_stream_t ret_ptr = (command_stream_t) checked_malloc(sizeof(struct command_stream));
  int size = 10;
  char* buf = (char*) checked_malloc((size_t) size);
  int pos = 0;
  int nextchar =  get_next_byte(get_next_byte_argument);
  while( nextchar != -1) 
  {
    if(pos==size)
    {
        checked_realloc(buf, (size_t)(2*size) );
        size = 2*size;
    }
    buf[pos] = (char) nextchar;
    nextchar = get_next_byte(get_next_byte_argument);
    pos++;
  }
 
  if(pos==size)
  {
    checked_realloc(buf, (size_t)(size+1));
  }
  buf[pos] = '\0'; printf("%s\n", buf); exit(1);

  ret_ptr->buffer = buf; 
  ret_ptr->index = 0; 
  ret_ptr->size = pos+1;

  command_t c = checked_malloc(sizeof(struct command));
  if (findChar(buf, ';') != NULL) ||
      (findChar(buf, '\n' != NULL)) {
       // is sequence command
  } else {
    //create simple command
    c->type = SIMPLE_COMMAND;
    c->status = -1;

    int size = 1;
    char** words = checked_malloc(sizeof(char*));
    char* nextword = getNextWord(buf);
    while(nextword !=NULL)
    {
        words[size-0]==nextword; 
        //TODO:
    }
        
    if (word == NULL)
        error(1,0,"syntax_error");


    char* temp = findChar(buf, '<');
    c->input = (temp != NULL) ? getNextWord(temp+1) : NULL;
    temp = findChar(buf, '>');
    c->output = (temp != NULL) ? getNextWord(temp+1) : NULL;

    //array of words
  }

  int i = 0;
  while (isspace(buf[i]))
    i++;
  if(buf[i]=='(')
  {
      //TODO: FIND MATCHING

  }
  else
      error(1,0,"invalid syntax");


  return ret_ptr;
  
  //error (1, 0, "command reading not yet implemented");
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
  error (1, 0, "command reading not yet implemented");
  return 0;
}
