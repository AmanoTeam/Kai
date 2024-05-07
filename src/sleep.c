#if defined(_WIN32)
	#include <synchapi.h>
#endif

#if !defined(_WIN32)
	#include <time.h>
#endif

#include "sleep.h"

int tsleep(const long milliseconds) {
	/*
	Block execution for the specified amount of milliseconds
	*/
	
	#if defined(_WIN32)
		Sleep((DWORD) milliseconds);
	#else
		struct timespec spec = {0};
		
		spec.tv_sec = milliseconds / 1000;
		spec.tv_nsec = (milliseconds % 1000) * 1000000;
		
		if (nanosleep(&spec, &spec) != 0) {
			return -1;
		}
	#endif
	
	return 0;
	
}
