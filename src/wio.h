#if !defined(WIO_H)
#define WIO_H

#include <stdio.h>

int __printf(const char* const format, ...);
int __fprintf(FILE* const stream, const char* const format, ...);

#define printf __printf
#define fprintf __fprintf

#endif
