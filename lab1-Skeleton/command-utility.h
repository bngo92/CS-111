#include "command.h"

void get_command(command_t c, char *s, int n);
int get_sequence(command_t c, char *s, int n);
int get_subshell(command_t c, char *s, int n);
int get_andor(command_t c, char *s, int n);
int get_pipeline(command_t c, char *s, int n);
void get_simple(command_t c, char *s, int n);
