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



       
/* FIXME: Define the type 'struct command_stream' here.  This should complete the incomplete type declaration in command.h.  */

struct command_stream
{
    command_t root;
};


command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  command_stream_t ret_stream = (command_stream_t) checked_malloc(sizeof(struct command_stream));
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
    size++;
  }
  buf[pos] = '\0'; 
  //pos++

  ret_stream->root = (command_t) checked_malloc(sizeof(command_t));
  get_command(ret_stream->root, buf, pos );


  return ret_stream;
}

command_t
read_command_stream (command_stream_t s)
{
  return s->root;
}
