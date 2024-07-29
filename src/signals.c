#include "signals.h"

void signal_sethandler(
	const int signal,
	SIGNAL_HANDLER_RETURN (*handler)(SIGNAL_HANDLER_ARGS)
) {
	
	#if defined(_WIN32)
	
	#else
	
		const struct sigaction action = {
			.sa_handler = handler
        };
		
		if (sigaction(signal, &action, NULL) == -1) {

		}
	#endif
	
}