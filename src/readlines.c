#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "readlines.h"

static const char LF[] = "\n";

void readlines_init(struct ReadLines* const readlines, const char* const s) {
	
	readlines->s = s;
	readlines->send = strchr(readlines->s, '\0');
	
	readlines->lbegin = readlines->s;
	readlines->lend = strstr(readlines->lbegin, LF);
	
	readlines->eof = 0;
	
}

const struct Line* readlines_next(struct ReadLines* const readlines, struct Line* const line) {
	
	size_t line_size = 0;
	
	if (readlines->eof) {
		return NULL;
	}
	
	if (readlines->lend == NULL) {
		readlines->lend = readlines->send;
	}
	
	line_size = (size_t) (readlines->lend - readlines->lbegin);
	
	line->begin = readlines->lbegin;
	line->size = line_size;
	
	if (line_size > 0) {
		const char* start = readlines->lbegin;
		const char* end = readlines->lend;
		
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
		
		line->begin = NULL;
		line->size = 0;
		
		if (end != start) {
			line->begin = start;
			line->size = ((size_t) (end - start)) + 1;
		}
	}
	
	readlines->eof = (readlines->lend == readlines->send);
	
	if (readlines->eof && line_size < 1) {
		return NULL;
	}
	
	if (readlines->lbegin != readlines->s) {
		line->index++;
	}
	
	if (!readlines->eof) {
		readlines->lbegin = readlines->lend;
		readlines->lbegin++;
		
		readlines->lend = strstr(readlines->lbegin, LF);
	}
	
	return line;
	
}
