#include <stdlib.h>
#include <stdio.h>

#include "terminal.h"
#include "callbacks.h"
#include "signals.h"
#include "sleep.h"

#if defined(_WIN32)
	#include "wio.h"
#endif

static const char* const PROGRESS_STEPS[] = {
	"|",
	"/",
	"-",
	"\\",
};

static const char DOTS[] = "...";

void download_progress_callback(const size_t total, const size_t current) {
	
	erase_line();
	
	printf("+ Fetching media segments: %zu out of %zu\r", current, total);
	fflush(stdout);
	
}

SIGNAL_HANDLER_RETURN sigint_handler(SIGNAL_HANDLER_ARGS) {
	
	fprintf(stderr, "* exiting abnorminally due to SIGINT interruption\n");
	
	show_cursor();
	
	SIGNAL_HANDLER_END
	
}

void* loading_progress_callback(void* ptr) {
	
	size_t index = 0;
	int dotc = 0;
	size_t times = 0;
	
	while (1) {
		const char* const step = PROGRESS_STEPS[index++];
		
		erase_line();
		
		printf("+ [ %s ] Loading media playlist%.*s\r", step, dotc, DOTS);
		fflush(stdout);
		
		if (times == 5) {
			times = 0;
			dotc += 1;
			
			if (dotc > 3) {
				dotc = 0;
			}
		}
		
		tsleep(100);
		
		if (index >= sizeof(PROGRESS_STEPS) / sizeof(*PROGRESS_STEPS)) {
			index = 0;
		}
		
		times += 1;
		
		if (!(*(int*) ptr)) {
			break;
		}
	}
	
	erase_line();
	
	return NULL;
	
}