#include <stdint.h>

#include "biggestint.h"

bigint_t ptobigint(const void* const pointer) {
	/*
	Convert a pointer to a bigint_t.
	*/
	
	const intptr_t value = (intptr_t) pointer;
	const bigint_t result = (bigint_t) value;
	
	return result;
	
}

biguint_t ptobiguint(const void* const pointer) {
	/*
	Convert a pointer to a biguint_t.
	*/
	
	const uintptr_t value = (uintptr_t) pointer;
	const biguint_t result = (biguint_t) value;
	
	return result;
	
}
