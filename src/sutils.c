#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "biggestint.h"
#include "sutils.h"

size_t intlen(const bigint_t value) {
	/*
	Calculates the number of digits required to represent this
	integer as a string.
	*/
	
	bigint_t val = value;
	size_t size = 0;
	
	if (val < 0) {
		size++;
	}
	
	do {
		val /= 10;
		size++;
	} while (val != 0);
	
	return size;
	
}

size_t uintlen(const biguint_t value) {
	/*
	Calculates the number of digits required to represent this
	unsigned integer as a string.
	*/
	
	biguint_t val = value;
	size_t size = 0;
	
	do {
		val /= 10;
		size++;
	} while (val != 0);
	
	return size;
	
}

size_t intptrlen(const intptr_t value) {
	/*
	Calculates the number of digits required to represent this
	integer as a string.
	*/
	
	intptr_t val = value;
	size_t size = 0;
	
	if (val < 0) {
		size++;
	}
	
	do {
		val /= 10;
		size++;
	} while (val != 0);
	
	return size;
	
}

size_t uintptrlen(const uintptr_t value) {
	/*
	Calculates the number of digits required to represent this
	unsigned integer as a string.
	*/
	
	uintptr_t val = value;
	size_t size = 0;
	
	do {
		val /= 10;
		size++;
	} while (val != 0);
	
	return size;
	
}

char* strip_whitespaces(char* const s) {
	
	size_t size = 0;
	
	const char* start = s;
	const char* end = strchr(s, '\0');
	
	while (start != end) {
		const char ch = *start;
		
		if (!isspace(ch)) {
			break;
		}
		
		start++;
	}
	
	if (start != end) {
		end--;
	}
	
	while (end != start) {
		const unsigned char ch = *end;
		
		if (!isspace(ch)) {
			break;
		}
		
		end--;
	}
	
	if (*end != '\0') {
		end++;
	}
	
	size = (size_t) (start - s);
	
	if (size != 0) {
		size = (size_t) (end - start);
		memmove(s, start, size);
		s[size] = '\0';
	}
	
	return s;
	
}
