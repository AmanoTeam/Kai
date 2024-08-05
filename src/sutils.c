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
