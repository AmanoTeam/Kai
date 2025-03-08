#if !defined(OS_CPU_H)
#define OS_CPU_H

#if !defined(_WIN32)
	#include <sys/types.h>
#endif

ssize_t get_nproc(void);

#endif
