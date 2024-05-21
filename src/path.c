#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if !defined(_WIN32)
	#include <limits.h>
	#include <sys/types.h>
#endif

#include "path.h"
#include "pathsep.h"

#if defined(_WIN32)
	#define NAME_MAX (_MAX_FNAME - 1)
#endif

static const char* const INVALID_FILENAME_CHARS = "'%\" /\\:*?<>|^";

int isabsolute(const char* const path) {
	
	#if defined(_WIN32)
		const int status = (*path == *PATHSEP || (isalpha(path[0]) && path[1] == ':' && path[2] == *PATHSEP));
	#else
		const int status = (*path == *PATHSEP);
	#endif
	
	return status;
	
}

int isrelative(const char* const path) {
	return !isabsolute(path);
}

char* basename(const char* const path) {
	/*
	Returns the final component of a path.
	*/
	
	char* last_comp = (char*) path;
	
	while (1) {
		char* slash_at = strchr(last_comp, PATHSEP[0]);
		
		if (slash_at == NULL) {
			break;
		}
		
		last_comp = slash_at + 1;
	}
	
	return last_comp;
	
}

char* get_file_extension(const char* const filename) {
	/*
	Returns the extension of a filename.
	*/
	
	size_t index = 0;
	
	const char* const last_part = basename(filename);
	
	char* start = strstr(last_part, ".");
	
	if (start == NULL) {
		return NULL;
	}
	
	while (1) {
		char* const tmp = strstr(start + 1, ".");
		
		if (tmp == NULL) {
			break;
		}
		
		start = tmp;
	}
	
	if (start == filename) {
		return NULL;
	}
	
	start++;
	
	if (*start == '\0') {
		return NULL;
	}
	
	for (index = 0; index < strlen(start); index++) {
		const unsigned char ch = start[index];
		
		if (!isalnum(ch)) {
			return NULL;
		}
	}
	
	return start;
	
}

char* remove_file_extension(char* const filename) {
	/*
	Removes extension of the filename.
	*/
	
	while (1) {
		char* const file_extension = get_file_extension(filename);
		
		if (file_extension == NULL) {
			break;
		}
		
		*(file_extension - 1) = '\0';
	}
	
	return filename;
	
}

char* normalize_filename(char* const filename) {
	/*
	Normalize filename by replacing invalid characters with underscore.
	*/
	
	size_t index = 0;
	
	for (index = 0; index < strlen(filename); index++) {
		const unsigned char ch = filename[index];
		
		if (iscntrl(ch) || strchr(INVALID_FILENAME_CHARS, ch) != NULL) {
			filename[index] = '_';
		}
	}
	
	char* start = filename;
	char* end = strchr(start, '\0');
	
	char* position = end;
	
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
		memmove(start, position, strlen(position) + 1);
	}
	
	return filename;
	
}

char* normalize_directory(char* directory) {
	/*
	Normalize directory name by replacing invalid characters with underscore.
	*/
	
	normalize_filename(directory);
	
	if (strlen(directory) > NAME_MAX) {
		directory[NAME_MAX - 1] = '\0';
	}
	
	return directory;
	
}

size_t get_parent_directory(const char* const source, char* const destination, const size_t maxdepth) {
	/*
	Get the parent directory up to the specified "maxdepth" depth of a path.
	
	Returns:
	
	- If the "destination" parameter is NULL, this will return the required size
	  for the buffer (not including the null-terminator).

	- If the "destination" parameter is not NULL, this will return the number of
	  characters written into the buffer (not including the null-terminator).
	*/
	
	ssize_t index = 0;
	size_t depth = 1;
	size_t wsize = 0;
	
	if (destination != NULL) {
		destination[0] = '\0';
	}
	
	for (index = (ssize_t) strlen(source) - 1; index >= 0; index--) {
		const char ch = source[index];
		
		if (ch == PATHSEP[0] && depth++ == maxdepth) {
			const size_t size = (size_t) ((source + index) - source);
			
			if (destination != NULL) {
				if (size > 0) {
					memcpy(destination, source, size);
					destination[size] = '\0';
				} else {
					strncat(destination, PATHSEP, 1);
				}
			}
			
			wsize += (size > 0) ? size : 1;
			
			break;
		}
		
		if (index == 0 && isabsolute(source)) {
			#if defined(_WIN32)
				const size_t size = 3;
			#else
				const size_t size = 1;
			#endif
			
			wsize += size;
			
			if (destination != NULL) {
				memcpy(destination, source, size);
				destination[size] = '\0';
			}
			
			break;
		}
	}
	
	return wsize;
	
}
/*
int main() {
	
	
	size_t sz = get_parent_directory("../system/etc/hosts", NULL, 8);
	char s[sz + 1];
	get_parent_directory("../system/etc/hosts", s, 8);
	
	printf("%s -> %zu\n", s, sz);
	
}
*/