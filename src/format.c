#include <stddef.h>
#include <math.h>
#include <stdio.h>

#include "biggestint.h"
#include "format.h"

char* btos(const bigfloat_t b, char* const s) {
	
	size_t index = 0;
	bigfloat_t value = b;
	
	const char* const units[] = {
		"", "K", "M", "G",
		"T", "P", "E", "Z"
	};
	
	const char* unit = NULL;
	bigfloat_t absolute = 0;
	
	for (index = 0; index < sizeof(units) / sizeof(*units); index++) {
		unit = units[index];
		absolute = fabsl(value);
		
		if (absolute >= 1024.0) {
			value /= 1024.0;
			continue;
		}
		
		if (snprintf(s, BTOS_MAX_SIZE, "%3.1"FORMAT_BIGGEST_FLOAT_T" %sB", value, unit) == -1) {
			return NULL;
		}
		
		return s;
	}
	
	if (snprintf(s, BTOS_MAX_SIZE, "%.1"FORMAT_BIGGEST_FLOAT_T" YB", value) == -1) {
		return NULL;
	}
	
	return s;
	
}