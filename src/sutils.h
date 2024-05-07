#if !defined(SUTILS_H)
#define SUTILS_H

#include <stdlib.h>

#include "biggestint.h"

char* strip(char* const s);
char fromhex(const char ch);
size_t intlen(const bigint_t value);
size_t uintlen(const biguint_t value);
int isnumeric(const char* const s);
int shash(const char* const s);

#endif
