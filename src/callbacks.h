#if !defined(CALLBACK_H)
#define CALLBACK_H

#include <stdlib.h>

#include "signals.h"

void progress_callback(const size_t total, const size_t current);
SIGNAL_HANDLER_RETURN sigint_handler(SIGNAL_HANDLER_ARGS);

#endif
