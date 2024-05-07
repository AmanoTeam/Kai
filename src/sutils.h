#if !defined(SUTILS_H)
#define SUTILS_H

#include <stdlib.h>

#include "biggestint.h"

size_t intlen(const bigint_t value);
size_t uintlen(const biguint_t value);
size_t intptrlen(const intptr_t value);
size_t uintptrlen(const uintptr_t value);

char* strip_whitespaces(char* const s);

#endif
