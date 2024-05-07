#if !defined(WIO_H)
#define WIO_H

#include <stdio.h>

int wio_printf(const char* const format, ...);
int wio_fprintf(FILE* const stream, const char* const format, ...);

int wio_setunicode(void);

#define printf wio_printf
#define fprintf wio_fprintf

#endif
