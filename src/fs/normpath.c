#include <stddef.h>
#include <ctype.h>
#include <string.h>

#if !defined(_WIN32)
	#include <limits.h>
#endif

#if defined(_WIN32)
	#include <stdlib.h>
#endif

#if defined(_WIN32)
	#define NAME_MAX (_MAX_FNAME - 1)
#endif

#include "fs/sep.h"
#include "fs/absrel.h"

static const char* const INVALID_FILENAME_CHARS = "'%\" /\\:*?<>|^";

char* normpath(char* const path, const int single_component) {
	/*
	Normalize path by replacing invalid characters with underscore.
	*/
	
	size_t index = 0;
	size_t len = 0;
	size_t size = 0;
	
	unsigned char ch = 0;
	
	char* start = NULL;
	char* end = NULL;
	
	char* position = NULL;
	
	#if defined(_WIN32)
		index += (isabsolute(path) * 3);
	#endif
	
	start = path + index;
	end = strchr(start, '\0');
	
	len = strlen(start);
	
	for (; index < len; index++) {
		position = &path[index];
		ch = *position;
		
		if (!single_component && ch == PATHSEP) {
			continue;
		}
		
		if (!(iscntrl(ch) || strchr(INVALID_FILENAME_CHARS, ch) != NULL)) {
			continue;
		}
		
		*position = '_';
	}
	
	position = end;
	
	position--;
	
	while (position != start) {
		if (*position != '.') {
			break;
		}
		
		*position = '\0';
		position--;
	}
	
	position = start;
	
	while (position != end) {
		if (*position != '.') {
			break;
		}
		
		*position = '\0';
		position++;
	}
	
	if (position != start) {
		size = strlen(position) + 1;
		memmove(start, position, size);
	}
	
	len = strlen(path) + 1;
	
	start = path;
	
	for (index = 0; index < len; index++) {
		position = (path + index);
		ch = *position;
		
		if (!(ch == PATHSEP || ch == '\0')) {
			continue;
		}
		
		end = position;
		size = (size_t) (end - start);
		
		if (size <= NAME_MAX) {
			start = end + 1;
			continue;
		}
		
		start += NAME_MAX;
		*start = ch;
		start++;
		
		end = strchr(start, PATHSEP);
		end = ((end == NULL) ? strchr(start, '\0') : end + 1);
		
		size = strlen(end);
		memmove(start, end, size);
		
		end = (start + size);
		*end = '\0';
		
		start = path;
		
		index = 0;
		len = ((size_t) (end - start)) + 1;
	}
	
	return path;
	
}
