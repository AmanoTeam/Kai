#if !defined(OS_CPUINFO_H)
#define OS_CPUINFO_H

#if !defined(_WIN32)
	#include <sys/types.h>
#endif

ssize_t get_nproc(void);

#endif
