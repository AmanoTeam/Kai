#include <stdlib.h>
#include <stdio.h>

#include "terminal.h"
#include "callbacks.h"
#include "signals.h"

void progress_callback(const size_t total, const size_t current) {
	
	erase_line();
	printf("+ Fetching media segments: %zu out of %zu\r", current, total);
	fflush(stdout);
	
}

SIGNAL_HANDLER_RETURN sigint_handler(SIGNAL_HANDLER_ARGS) {
	
	show_cursor();
	
	SIGNAL_HANDLER_END
	
}
