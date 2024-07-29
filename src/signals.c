#include <stdlib.h>
#include <signal.h>

#include "signals.h"

int signal_sethandler(
	const int code,
	SIGNAL_HANDLER_RETURN (*handler)(SIGNAL_HANDLER_ARGS)
) {
	
	#if defined(_WIN32)
		if (signal(code, handler) == SIG_ERR) {
			return -1;
		}
	#else
		const struct sigaction action = {
			.sa_handler = handler
		};
		
		if (sigaction(code, &action, NULL) == -1) {
			return -1;
		}
	#endif
	
	return 0;
	
}