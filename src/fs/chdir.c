#if defined(_WIN32)
	#include <stdlib.h>
	#include <string.h>
	#include <windows.h>
#endif

#if !defined(_WIN32)
	#include <unistd.h>
#endif

#if defined(_WIN32)
	#include "fs/absrel.h"
	#include "fs/sep.h"
#endif

int set_current_directory(const char* const directory) {
	/*
	Change the current working directory of the calling
	process to the specified directory..
	
	Returns (0) on success, (-1) on error.
	*/
	
	int err = 0;
	
	#if defined(_WIN32)
		BOOL status = FALSE;
		
		#if defined(_UNICODE)
			wchar_t* wdirectory = NULL;
			
			/* This prefix is required to support long paths in Windows 10+ */
			const size_t prefixs = isabsolute(directory) ? wcslen(WIN10_LONG_PATH_PREFIX) : 0;
			
			const int wdirectorys = MultiByteToWideChar(CP_UTF8, 0, directory, -1, NULL, 0);
			
			if (wdirectorys == 0) {
				err = -1;
				goto end;
			}
			
			wdirectory = malloc((prefixs + (size_t) wdirectorys) * sizeof(*wdirectory));
			
			if (wdirectory == NULL) {
				err = -1;
				goto end;
			}
			
			if (prefixs > 0) {
				wcscpy(wdirectory, WIN10_LONG_PATH_PREFIX);
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, directory, -1, wdirectory + prefixs, wdirectorys) == 0) {
				err = -1;
				goto end;
			}
			
			status = SetCurrentDirectoryW(wdirectory);
		#else
			status = SetCurrentDirectoryA(directory);
		#endif
		
		if (!status) {
			err = -1;
			goto end;
		}
	#else
		if (chdir(directory) == -1) {
			err = -1;
			goto end;
		}
	#endif
	
	end:;
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(wdirectory);
	#endif
	
	return err;
	
}