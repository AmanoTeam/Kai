#ifndef TERMINAL_H
#define TERMINAL_H 1

#include <stdio.h>

int erase_screen(void);
int erase_line(void);
int hide_cursor(void);
int show_cursor(void);
int is_atty(FILE* file);

#endif
