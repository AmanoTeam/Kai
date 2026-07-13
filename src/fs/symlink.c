#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#if defined(_WIN32)
	#include <windows.h>
	#include <fileapi.h>
#endif

#if !defined(_WIN32)
	#include <sys/stat.h>
	#include <unistd.h>
	#include <limits.h>
#endif

#if defined(_WIN32)
	#include "fs/absrel.h"
	#include "fs/sep.h"
	#include "fs/exists.h"
#endif

int mklink(const char* const source, const char* const destination) {
	/*
	Create a symbolic link.
	
	Returns (0) on success, (-1) on error.
	*/
	
	int err = 0;
	
	#if defined(_WIN32)
		int flags = SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
		
		#if defined(_UNICODE)
			wchar_t* wsource = NULL;
			wchar_t* wdestination = NULL;
			
			int wfilenames = 0;
			size_t prefixs = 0;
		#endif
		
		if (directory_exists(source)) {
			flags |= SYMBOLIC_LINK_FLAG_DIRECTORY;
		}
		
		#if defined(_UNICODE)
			prefixs = isabsolute(source) ? wcslen(WIN10_LONG_PATH_PREFIX) : 0;
			
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
			
			if (CreateSymbolicLinkW(wsource, wdestination, flags) == FALSE) {
				err = -1;
				goto end;
			}
		#else
			if (CreateSymbolicLinkA(source, destination, flags) == FALSE) {
				err = -1;
				goto end;
			}
		#endif
	#else
		if (symlink(source, destination) == -1) {
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

char* get_symlink(const char* const path) {
	
	char* location = NULL;
	
	#if defined(_WIN32)
		return location;
	#else
		ssize_t size = 0;
		
		location = malloc(PATH_MAX);
		
		if (location == NULL) {
			goto end;
		}
		
		size = readlink(path, location, PATH_MAX);
		
		if (size == -1) {
			free(location);
			goto end;
		}
		
		location[(size_t) size] = '\0';
	#endif
	
	end:;
	
	return location;
	
}