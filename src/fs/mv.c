#include <stdlib.h>

#if defined(_WIN32)
	#include <windows.h>
	#include <winbase.h>
	#include <string.h>
#endif

#if !defined(_WIN32)
	#include <errno.h>
	#include <stdio.h>
#endif

#if defined(_WIN32)
	#include "path.h"
	#include "pathsep.h"
#endif

#include "fs/cp.h"
#include "fs/rm.h"
#include "fs/mv.h"

int move_file(const char* const source, const char* const destination) {
	/*
	Moves a file from source to destination.
	
	Symlinks are not followed: if source is a symlink, it is itself moved, not it's target.
	If destination already exists, it will be overwritten.
	
	Returns (0) on success, (-1) on error.
	*/
	
	int err = 0;
	
	#if defined(_WIN32)
		BOOL status = FALSE;
		
		#if defined(_UNICODE)
			wchar_t* wsource = NULL;
			wchar_t* wdestination = NULL;
			
			int wfilenames = 0;
			
			/* This prefix is required to support long paths in Windows 10+ */
			size_t prefixs = isabsolute(source) ? wcslen(WIN10_LONG_PATH_PREFIX) : 0;
			
			wfilenames = MultiByteToWideChar(CP_UTF8, 0, source, -1, NULL, 0);
			
			if (wfilenames == 0) {
				err = -1;
				goto end;
			}
			
			wsource = malloc((prefixs + (size_t) wfilenames) * sizeof(*wsource));
			
			if (prefixs > 0) {
				wcscpy(wsource, WIN10_LONG_PATH_PREFIX);
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, source, -1, wsource + prefixs, wfilenames) == 0) {
				err = -1;
				goto end;
			}
			
			prefixs = isabsolute(destination) ? wcslen(WIN10_LONG_PATH_PREFIX) : 0;
			
			wfilenames = MultiByteToWideChar(CP_UTF8, 0, destination, -1, NULL, 0);
			
			if (wfilenames == 0) {
				err = -1;
				goto end;
			}
			
			wdestination = malloc((prefixs + (size_t) wfilenames) * sizeof(*wdestination));
			
			if (prefixs > 0) {
				wcscpy(wdestination, WIN10_LONG_PATH_PREFIX);
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, destination, -1, wdestination + prefixs, wfilenames) == 0) {
				err = -1;
				goto end;
			}
			
			status = MoveFileExW(wsource, wdestination, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING);
		#else
			status = MoveFileExA(source, destination, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING);
		#endif
		
		if (status) {
			goto end;
		}
		
		if (GetLastError() != ERROR_ACCESS_DENIED) {
			err = -1;
			goto end;
		}
		
		if (copy_file(source, destination) == -1) {
			err = -1;
			goto end;
		}
		
		if (remove_file(source) == -1) {
			err = -1;
			goto end;
		}
	#else
		if (rename(source, destination) == 0) {
			goto end;
		}
		
		if (errno != EXDEV) {
			err = -1;
			goto end;
		}
		
		if (copy_file(source, destination) == -1) {
			err = -1;
			goto end;
		}
		
		if (remove_file(source) == -1) {
			err = -1;
			goto end;
		}
	#endif
	
	end:;
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(wsource);
		free(wdestination);
	#endif
	
	return err;
	
}