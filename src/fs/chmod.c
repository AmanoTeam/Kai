#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#if defined(_WIN32)
	#include <windows.h>
	#include <fileapi.h>
#endif

#if !defined(_WIN32)
	#include <sys/stat.h>
#endif

#if defined(_WIN32)
	#include "fs/absrel.h"
	#include "fs/sep.h"
#endif

#include "fs/chmod.h"

int chmod_getmode(const char* const path) {
	
	int err = 0;
	
	#if defined(_WIN32)
		DWORD attributes = 0;
		
		#if defined(_UNICODE)
			wchar_t* wpath = NULL;
			
			/* This prefix is required to support long paths in Windows 10+ */
			const size_t prefixs = isabsolute(path) ? wcslen(WIN10_LONG_PATH_PREFIX) : 0;
			
			const int wpaths = MultiByteToWideChar(CP_UTF8, 0, path, -1, NULL, 0);
			
			if (wpaths == 0) {
				err = -1;
				goto end;
			}
			
			wpath = malloc((prefixs + (size_t) wpaths) * sizeof(*wpath));
			
			if (wpath == NULL) {
				err = -1;
				goto end;
			}
			
			if (prefixs > 0) {
				wcscpy(wpath, WIN10_LONG_PATH_PREFIX);
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath + prefixs, wpaths) == 0) {
				err = -1;
				goto end;
			}
			
			attributes = GetFileAttributesW(wpath);
		#else
			attributes = GetFileAttributesA(path);
		#endif
		
		if (attributes == INVALID_FILE_ATTRIBUTES) {
			err = -1;
			goto end;
		}
		
		err = (
			CHMOD_USER_EXEC | CHMOD_USER_READ | CHMOD_GROUP_EXEC |
			CHMOD_GROUP_READ | CHMOD_OTHERS_EXEC | CHMOD_OTHERS_READ
		);
		
		if ((attributes & FILE_ATTRIBUTE_READONLY) == 0) {
			err |= CHMOD_USER_WRITE | CHMOD_GROUP_WRITE | CHMOD_OTHERS_WRITE;
		}
	#else
		struct stat st = {0};
		
		if (lstat(path, &st) == -1) {
			err = -1;
			goto end;
		}
		
		if ((st.st_mode & S_IRUSR) != 0) {
			err |= CHMOD_USER_READ;
		}
		
		if ((st.st_mode & S_IWUSR) != 0) {
			err |= CHMOD_USER_WRITE;
		}
		
		if ((st.st_mode & S_IXUSR) != 0) {
			err |= CHMOD_USER_EXEC;
		}
		
		if ((st.st_mode & S_IRGRP) != 0) {
			err |= CHMOD_GROUP_READ;
		}
		
		if ((st.st_mode & S_IWGRP) != 0) {
			err |= CHMOD_GROUP_WRITE;
		}
		
		if ((st.st_mode & S_IXGRP) != 0) {
			err |= CHMOD_GROUP_EXEC;
		}
		
		if ((st.st_mode & S_IROTH) != 0) {
			err |= CHMOD_OTHERS_READ;
		}
		
		if ((st.st_mode & S_IWOTH) != 0) {
			err |= CHMOD_OTHERS_WRITE;
		}
		
		if ((st.st_mode & S_IXOTH) != 0) {
			err |= CHMOD_OTHERS_EXEC;
		}
	#endif
	
	end:;
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(wpath);
	#endif
	
	return err;
	
}

int chmod_setmode(const char* const path, const int value) {
	
	int err = 0;
	
	#if defined(_WIN32)
		BOOL status = FALSE;
		int attribute = FILE_ATTRIBUTE_NORMAL;
		
		#if defined(_UNICODE)
			wchar_t* wpath = NULL;
			int wpaths = 0;
			
			size_t prefixs = 0;
		#endif
		
		int write = ((value & (CHMOD_USER_WRITE | CHMOD_GROUP_WRITE | CHMOD_OTHERS_WRITE)) != 0);
		int read = ((value & (CHMOD_USER_READ | CHMOD_GROUP_READ | CHMOD_OTHERS_READ)) != 0);
		
		if (!(read || write)) {
			goto end;
		}
		
		if (read && !write) {
			attribute = FILE_ATTRIBUTE_READONLY;
		}
		
		#if defined(_UNICODE)
			prefixs = isabsolute(path) ? wcslen(WIN10_LONG_PATH_PREFIX) : 0;
			
			wpaths = MultiByteToWideChar(CP_UTF8, 0, path, -1, NULL, 0);
			
			if (wpaths == 0) {
				err = -1;
				goto end;
			}
			
			wpath = malloc((prefixs + (size_t) wpaths) * sizeof(*wpath));
			
			if (wpath == NULL) {
				err = -1;
				goto end;
			}
			
			if (prefixs > 0) {
				wcscpy(wpath, WIN10_LONG_PATH_PREFIX);
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath + prefixs, wpaths) == 0) {
				err = -1;
				goto end;
			}
			
			status = SetFileAttributesW(wpath, attribute);
		#else
			status = SetFileAttributesA(path, attribute);
		#endif
		
		if (!status) {
			err = -1;
			goto end;
		}
	#else
		int mode = 0;
		
		/* User */
		if ((value & CHMOD_USER_READ) != 0) {
			mode |= S_IRUSR;
		}
		
		if ((value & CHMOD_USER_WRITE) != 0) {
			mode |= S_IWUSR;
		}
		
		if ((value & CHMOD_USER_EXEC) != 0) {
			mode |= S_IXUSR;
		}
		
		/* Group */
		if ((value & CHMOD_GROUP_READ) != 0) {
			mode |= S_IRGRP;
		}
		
		if ((value & CHMOD_GROUP_WRITE) != 0) {
			mode |= S_IWGRP;
		}
		
		if ((value & CHMOD_GROUP_EXEC) != 0) {
			mode |= S_IXGRP;
		}
		
		/* Others */
		if ((value & CHMOD_OTHERS_READ) != 0) {
			mode |= S_IROTH;
		}
		
		if ((value & CHMOD_OTHERS_WRITE) != 0) {
			mode |= S_IWOTH;
		}
		
		if ((value & CHMOD_OTHERS_EXEC) != 0) {
			mode |= S_IXOTH;
		}
		
		if (chmod(path, mode) == -1) {
			err = -1;
			goto end;
		}
	#endif
	
	end:;
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(wpath);
	#endif
	
	return err;
	
}