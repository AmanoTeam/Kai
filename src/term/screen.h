#if !defined(TERMINAL_SCREEN_H)
#define TERMINAL_SCREEN_H

#include <stdio.h>
#include <stddef.h>

int erase_screen(void);
int erase_line(void);
int hide_cursor(void);
int show_cursor(void);
int is_atty(FILE* file);
size_t get_terminal_height(void);
size_t get_terminal_width(void);

#endif
