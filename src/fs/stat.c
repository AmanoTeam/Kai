#include <stdlib.h>

#if defined(_WIN32)
	#include <windows.h>
	#include <fileapi.h>
	#include <string.h>
#endif

#if !defined(_WIN32)
	#include <errno.h>
	#include <sys/stat.h>
#endif

#if defined(_WIN32)
	#include "fs/absrel.h"
	#include "fs/sep.h"
#endif

#include "fs/stat.h"

int get_file_type(const char* const filename) {
	
	#if defined(_WIN32)
		DWORD error = 0;
		DWORD attributes = 0;
		
		#if defined(_UNICODE)
			wchar_t* wfilename = NULL;
			
			const size_t prefixs = isabsolute(filename) ? wcslen(WIN10_LONG_PATH_PREFIX) : 0;
			
			const int wfilenames = MultiByteToWideChar(CP_UTF8, 0, filename, -1, NULL, 0);
			
			if (wfilenames == 0) {
				return FILETYPE_UNKNOWN;
			}
			
			wfilename = malloc((prefixs + (size_t) wfilenames) * sizeof(*wfilename));
			
			if (wfilename == NULL) {
				return FILETYPE_UNKNOWN;
			}
			
			if (prefixs > 0) {
				wcscpy(wfilename, WIN10_LONG_PATH_PREFIX);
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, filename, -1, wfilename + prefixs, wfilenames) == 0) {
				free(wfilename);
				return FILETYPE_UNKNOWN;
			}
			
			attributes = GetFileAttributesW(wfilename);
			free(wfilename);
		#else
			attributes = GetFileAttributesA(filename);
		#endif
		
		if (attributes == INVALID_FILE_ATTRIBUTES) {
			return FILETYPE_UNKNOWN;
		}
		
		if ((attributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0) {
			return FILETYPE_SYMLINK;
		}
		
		if ((attributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
			return FILETYPE_DIRECTORY;
		}
		
		if ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
			return FILETYPE_REGULAR;
		}
	#else
		struct stat st = {0};
		
		if (lstat(filename, &st) == -1) {
			return FILETYPE_UNKNOWN;
		}
		
		if (S_ISLNK(st.st_mode)) {
			return FILETYPE_SYMLINK;
		}
		
		if (S_ISREG(st.st_mode)) {
			return FILETYPE_REGULAR;
		}
		
		if (S_ISDIR(st.st_mode)) {
			return FILETYPE_DIRECTORY;
		}
		
		if (S_ISCHR(st.st_mode)) {
			return FILETYPE_CHARDEV;
		}
		
		if (S_ISBLK(st.st_mode)) {
			return FILETYPE_BLOCKDEV;
		}
		
		if (S_ISFIFO(st.st_mode)) {
			return FILETYPE_FIFO;
		}
		
		if (S_ISSOCK(st.st_mode)) {
			return FILETYPE_SOCKET;
		}
	#endif
	
	return FILETYPE_UNKNOWN;
	
}
