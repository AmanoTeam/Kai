#include <stddef.h>
#include <stdio.h>

#include "term/screen.h"

void download_progress_callback(const size_t total, const size_t current) {
	
	erase_line();
	
	printf("+ Fetching packages: %zu out of %zu\r", current, total);
	
	fflush(stdout);
	
}
