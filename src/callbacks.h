#if !defined(CALLBACK_H)
#define CALLBACK_H

#include <stdlib.h>

#include "signals.h"

void* loading_progress_callback(void* ptr);
SIGNAL_HANDLER_RETURN sigint_handler(SIGNAL_HANDLER_ARGS);

#endif
