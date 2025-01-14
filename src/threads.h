#if !defined(THREADS_H)
#define THREADS_H

#if defined(_WIN32)
	#include <windows.h>
#endif

#if !defined(_WIN32)
	#include <pthread.h>
#endif

#if defined(_WIN32)
	struct platform_thread {
		int detach;
		DWORD id;
		HANDLE handle;
	};
	
	struct platform_thread_data {
		void*(*callback)(void*);
		void* argument;
	};
	
	DWORD WINAPI thread_callback(LPVOID param);
	
	typedef struct platform_thread_data thread_data_t;
#else
	struct platform_thread {
		int detach;
		pthread_t thread;
		pthread_attr_t attr;
	};
#endif

typedef struct platform_thread thread_t;

int thread_create(thread_t* const thread, void*(*callback)(void*), void* const argument);
int thread_wait(thread_t* const thread);
void thread_detach(thread_t* const thread);

#endif
