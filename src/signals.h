#if !defined(SIGNALS_H)
#define SIGNALS_H

#if defined(_WIN32)
	#define SIGNAL_HANDLER_RETURN int
	#define SIGNAL_HANDLER_ARGS int value, int subcode
	#define SIGNAL_HANDLER_END return 0;
#else
	#define SIGNAL_HANDLER_RETURN void
	#define SIGNAL_HANDLER_ARGS
	#define SIGNAL_HANDLER_END
#endif

#endif
