#include <stddef.h>
#include <sys/types.h>

int wildcard_match(const char* const pattern, const char* const string) {
	
	size_t index = 0;
	size_t subindex = 0;
	
	ssize_t star_index = -1;
	ssize_t star_subindex = -1;
	
	unsigned char a = 0;
	unsigned char b = 0;

	while (1) {
		a = string[subindex];
		
		if (a == '\0') {
			break;
		}
		
		b = pattern[index];
		
		if (b == a || b == '?') {
			index++;
			subindex++;
		} else if (b == '*') {
			star_index = ++index;
			star_subindex = subindex;
		} else if (star_index != -1) {
			index = star_index;
			subindex = ++star_subindex;
		} else {
			return 0;
		}
	}
	
	b = pattern[index];
	
	while (b == '*') {
		b = pattern[++index];
	}
	
	b = pattern[index];
	
	return b == '\0';
	
}