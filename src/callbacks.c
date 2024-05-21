#include <stdlib.h>
#include <stdio.h>

#include "terminal.h"
#include "callbacks.h"

void progress_callback(const size_t total, const size_t current) {
	
	erase_line();
	printf("+ Fetching media segments: %zu out of %zu\r", current, total);
	fflush(stdout);
	
}
