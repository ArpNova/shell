#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>
#include <limits.h>

char *arsh_read_line(FILE *stream);
void disableRawMode();
void enableRawMode();
void add_to_history(char *line);
void arsh_print_prompt();

#endif
