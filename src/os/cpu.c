#if defined(_WIN32)
	#include <windows.h>
	#include <sysinfoapi.h>
#endif

#if !defined(_WIN32)
	#include <unistd.h>
	#include <sys/types.h>
#endif

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__) || defined(__OpenBSD__)
	#include <sys/sysctl.h>
#endif

#if defined(__HAIKU__)
	#include <OS.h>
#endif

#include "os/cpu.h"

ssize_t get_nproc(void) {
	/*
	Get the number of processing units available to the current process,
	
	Returns (>=0) on success, (-1) on error.
	*/
	
	ssize_t processors = 0;
	
	#if defined(_WIN32)
		SYSTEM_INFO info = {0};
		GetSystemInfo(&info);
		
		processors = (ssize_t) info.dwNumberOfProcessors;
	#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__) || defined(__OpenBSD__)
		#if defined(__APPLE__)
			const int code = HW_AVAILCPU;
		#else
			const int code = HW_NCPU;
		#endif
		
		const int call[] = {CTL_HW, code};
		
		size_t size = sizeof(processors);
		
		if (sysctl(call, sizeof(call) / sizeof(*call), &processors, &size, NULL, 0) == -1) {
			return -1;
		}
	#elif defined(__HAIKU__)
		system_info info = {0};
		const status_t status = get_system_info(&info);
		
		if (status != B_OK) {
			return -1;
		}
		
		processors = (ssize_t) info.cpu_count;
	#else
		const long value = sysconf(_SC_NPROCESSORS_ONLN);
		
		if (value == -1) {
			return -1;
		}
		
		processors = (ssize_t) value;
	#endif
	
	return processors;
	
}
