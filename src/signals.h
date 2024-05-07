#if !defined(SIGNALS_H)
#define SIGNALS_H

#if defined(_WIN32)
	#define SIGNAL_HANDLER_RETURN void
	#define SIGNAL_HANDLER_ARGS int value
	#define SIGNAL_HANDLER_END exit(1);
#else
	#define SIGNAL_HANDLER_RETURN void
	#define SIGNAL_HANDLER_ARGS
	#define SIGNAL_HANDLER_END exit(1);
#endif

int signal_sethandler(
	const int signal,
	SIGNAL_HANDLER_RETURN (*handler)(SIGNAL_HANDLER_ARGS)
);

#endif
