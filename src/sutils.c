#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "biggestint.h"
#include "sutils.h"

char* strip(char* const s) {
	/*
	Strip leading and trailing whitespaces from string.
	*/
	
	char* start = s;
	char* end = strchr(start, '\0');
	
	char* position = end;
	
	position--;
	
	while (position != start) {
		const unsigned char ch = *position;
		
		if (!(iscntrl(ch) || isspace(ch))) {
			break;
		}
		
		*position = '\0';
		position--;
	}
	
	position = start;
	
	while (position != end) {
		const unsigned char ch = *position;
		
		if (!(iscntrl(ch) || isspace(ch))) {
			break;
		}
		
		*position = '\0';
		position++;
	}
	
	if (position != start) {
		memmove(start, position, strlen(position) + 1);
	}
	
	return s;
	
}

char fromhex(const char ch) {
	/*
	Converts a hexadecimal character to its corresponding
	decimal value.
	*/
	
	if (ch <= '9' && ch >= '0') {
		return ch - '0';
	}
	
	 if (ch <= 'f' && ch >= 'a') {
		return ch - ('a' - 10);
	}
	
	if (ch <= 'F' && ch >= 'A') {
		return ch - ('A' - 10);
	}
	
	return '0';
	
}

size_t intlen(const bigint_t value) {
	/*
	Calculates the number of digits required to represent this
	integer as a string.
	*/
	
	bigint_t val = value;
	size_t size = 0;
	
	do {
		val /= 10;
		size++;
	} while (val > 0);
	
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
	} while (val > 0);
	
	return size;
	
}

int isnumeric(const char* const s) {
	/*
	Check whether the string pointed to by "s" is numeric.
	
	A string is numeric if all characters in the string are numeric and
	there is at least one character in the string.
	*/
	
	size_t index = 0;
	const size_t size = strlen(s);
	
	if (size < 1) {
		return 0;
	}
	
	for (index = 0; index < size; index++) {
		const unsigned char ch = s[index];
		
		if (!isdigit(ch)) {
			return 0;
		}
	}
	
	return 1;
	
}

int shash(const char* const s) {
	/*
	Computes a numeric hash from a string.
	*/
	
	int value = 0;
	
	for (size_t index = 0; index < strlen(s); index++) {
		const int ch = s[index];
		value += ch + (int) index;
	}
	
	return value;
	
}