#include <stdlib.h>

#if defined(_WIN32)
	#include <windows.h>
#endif

#if !defined(_WIN32)
	#include <pthread.h>
#endif

#include "threads.h"

#if defined(_WIN32)
	DWORD WINAPI thread_callback(LPVOID param) {
		
		thread_data_t* data = (thread_data_t*) param;
		(*data->callback)(data->argument);
		
		return 0;
		
	}
#endif

int thread_create(thread_t* const thread, void*(*callback)(void*), void* const argument) {
	
	#if defined(_WIN32)
		thread_data_t* data = malloc(sizeof(thread_data_t));
		
		if (data == NULL) {
			return -1;
		}
		
		data->callback = callback;
		data->argument = argument;
		
		thread->handle = CreateThread(NULL, 0, thread_callback, (void*) data, 0, &thread->id);
		
		if (thread->handle == NULL) {
			return -1;
		}
		
		if (thread->detach) {
			const BOOL status = CloseHandle(thread->handle);
			
			if (status == 0) {
				return -1;
			}
		}
	#else
		int status = 0;
		
		if (thread->detach) {
			status = pthread_attr_init(&thread->attr);
			
			if (status != 0) {
				return -1;
			}
			
			status = pthread_attr_setdetachstate(&thread->attr, PTHREAD_CREATE_DETACHED); 
			
			if (status != 0) {
				return -1;
			}
		}
		
		status = pthread_create(
			&thread->thread,
			(thread->detach) ? &thread->attr : NULL,
			callback,
			argument
		);
		
		if (status != 0) {
			pthread_attr_destroy(&thread->attr);
			return -1;
		}
		
		status = pthread_attr_destroy(&thread->attr);
		
		if (status != 0) {
			return -1;
		}
	#endif
	
	return 0;
	
}

int thread_wait(thread_t* const thread) {
	
	#if defined(_WIN32)
		if (WaitForSingleObject(thread->handle, INFINITE) == WAIT_FAILED) {
			return -1;
		}
	#else
		if (pthread_join(thread->thread, NULL) != 0) {
			return -1;
		}
	#endif
	
	return 0;
	
}

void thread_detach(thread_t* const thread) {
	
	thread->detach = 1;
	
}