#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
	#include <windows.h>
	#include <fileapi.h>
#endif

#if defined(__FreeBSD__)
	#include <sys/types.h>
#endif

#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__) || defined(__OpenBSD__)
	#include <sys/sysctl.h>
#endif

#if defined(__APPLE__)
	#include <sys/param.h>
	#include <copyfile.h>
	#include <mach-o/dyld.h>
#endif

#if defined(__HAIKU__)
	#include <FindDirectory.h>
#endif

#if !defined(__HAIKU__)
	#include <fcntl.h>
#endif

#if !defined(_WIN32)
	#include <unistd.h>
	#include <sys/stat.h>
	#include <errno.h>
	#include <limits.h>
	#include <fcntl.h>
#endif

#include "fstream.h"
#include "filesystem.h"
#include "walkdir.h"
#include "pathsep.h"

#if defined(_WIN32) || defined(__OpenBSD__)
	#include "path.h"
#endif

char* get_current_directory(void) {
	/*
	Returns the current working directory.
	
	Returns a null pointer on error.
	*/
	
	char* cwd = NULL;
	
	#if defined(_WIN32)
		DWORD cwds = 0;
		
		#if defined(_UNICODE)
			wchar_t* wcwd = NULL;
			
			cwds = GetCurrentDirectoryW(0, NULL);
			
			if (cwds == 0) {
				return NULL;
			}
			
			wcwd = malloc((size_t) cwds * sizeof(*wcwd));
			
			if (wcwd == NULL) {
				return NULL;
			}
			
			cwds = GetCurrentDirectoryW(cwds, wcwd);
			
			if (cwds == 0) {
				free(wcwd);
				return NULL;
			}
			
			cwds = (DWORD) WideCharToMultiByte(CP_UTF8, 0, wcwd, -1, NULL, 0, NULL, NULL);
			
			if (cwds == 0) {
				free(wcwd);
				return NULL;
			}
			
			cwd = malloc((size_t) cwds);
			
			if (cwd == NULL) {
				free(wcwd);
				return NULL;
			}
			
			if (WideCharToMultiByte(CP_UTF8, 0, wcwd, -1, cwd, (int) cwds, NULL, NULL) == 0) {
				free(wcwd);
				free(cwd);
				return NULL;
			}
			
			free(wcwd);
		#else
			cwds = GetCurrentDirectoryA(0, NULL);
			
			if (cwds == 0) {
				return NULL;
			}
			
			cwd = malloc((size_t) cwds);
			
			if (cwd == NULL) {
				return NULL;
			}
			
			cwds = GetCurrentDirectoryA(cwds, cwd);
			
			if (cwds == 0) {
				free(cwd);
				return NULL;
			}
		#endif
	#else
		cwd = malloc(PATH_MAX);
		
		if (cwd == NULL) {
			return NULL;
		}
		
		if (getcwd(cwd, PATH_MAX) == NULL) {
			free(cwd);
			return NULL;
		}
	#endif
	
	return cwd;
	
}

int remove_file(const char* const filename) {
	/*
	Removes a file from disk.
	
	On Windows, ignores the read-only attribute.
	This does not fail if the file never existed in the first place.
	
	Returns (0) on success, (-1) on error.
	*/
	
	#if defined(_WIN32)
		BOOL status = FALSE;
		
		#if defined(_UNICODE)
			wchar_t* wfilename = NULL;
			
			/* This prefix is required to support long paths in Windows 10+ */
			const size_t prefixs = isabsolute(filename) ? wcslen(WIN10_LONG_PATH_PREFIX) : 0;
			
			const int wfilenames = MultiByteToWideChar(CP_UTF8, 0, filename, -1, NULL, 0);
			
			if (wfilenames == 0) {
				return -1;
			}
			
			wfilename = malloc((prefixs + (size_t) wfilenames) * sizeof(*wfilename));
			
			if (prefixs > 0) {
				wcscpy(wfilename, WIN10_LONG_PATH_PREFIX);
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, filename, -1, wfilename + prefixs, wfilenames) == 0) {
				free(wfilename);
				return -1;
			}
			
			status = DeleteFileW(wfilename);
		#else
			status = DeleteFileA(filename);
		#endif
		
		if (!status) {
			const DWORD error = GetLastError();
			
			switch (error) {
				case ERROR_ACCESS_DENIED: {
					/*
					We could not delete the file due to lack of permissions. There are two possible causes for this:
					
					* The file is owned by other user.
					* The file has the "read-only" attribute set.
					*/
					
					/* We will assume that this is the second case and try to remove that read-only attribute. */
					#if defined(_UNICODE)
						status = SetFileAttributesW(wfilename, FILE_ATTRIBUTE_NORMAL);
					#else
						status = SetFileAttributesA(filename, FILE_ATTRIBUTE_NORMAL);
					#endif
					
					if (!status) {
						#if defined(_UNICODE)
							free(wfilename);
						#endif
						
						return -1;
					}
					
					/* Now that this attribute is gone, let's try to delete that file again. */
					#if defined(_UNICODE)
						status = DeleteFileW(wfilename);
					#else
						status = DeleteFileA(filename);
					#endif
					
					#if defined(_UNICODE)
						free(wfilename);
					#endif
					
					if (!status) {
						return -1;
					}
				
					break;
				}
				case ERROR_FILE_NOT_FOUND:
				case ERROR_PATH_NOT_FOUND: {
					/* The file never existed in the first place; that's not an error. */
					
					#if defined(_UNICODE)
						free(wfilename);
					#endif
					
					break;
				}
				default: {
					#if defined(_UNICODE)
						free(wfilename);
					#endif
					
					return -1;
				}
			}
		}
	#else
		if (unlink(filename) == -1 && errno != ENOENT) {
			return -1;
		}
	#endif
	
	return 0;
	
}

static int remove_empty_directory(const char* const directory) {
	/*
	Deletes an existing empty directory.
	
	Returns (0) on success, (-1) on error.
	*/
	
	#if defined(_WIN32)
		BOOL status = FALSE;
		
		#if defined(_UNICODE)
			wchar_t* wdirectory = NULL;
			
			/* This prefix is required to support long paths in Windows 10+ */
			const size_t prefixs = isabsolute(directory) ? wcslen(WIN10_LONG_PATH_PREFIX) : 0;
			
			const int wdirectorys = MultiByteToWideChar(CP_UTF8, 0, directory, -1, NULL, 0);
			
			if (wdirectorys == 0) {
				return -1;
			}
			
			wdirectory = malloc((prefixs + (size_t) wdirectorys) * sizeof(*wdirectory));
			
			if (wdirectory == NULL) {
				return -1;
			}
			
			if (prefixs > 0) {
				wcscpy(wdirectory, WIN10_LONG_PATH_PREFIX);
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, directory, -1, wdirectory + prefixs, wdirectorys) == 0) {
				free(wdirectory);
				wdirectory = NULL;
				return -1;
			}
			
			status = RemoveDirectoryW(wdirectory);
			
			free(wdirectory);
			wdirectory = NULL;
		#else
			status = RemoveDirectoryA(directory);
		#endif
		
		if (!status) {
			return -1;
		}
	#else
		if (rmdir(directory) == -1) {
			return -1;
		}
	#endif
	
	return 0;
	
}

int remove_recursive(const char* const directory, const int remove_itself) {
	/*
	Recursively removes a directory from disk.
	
	Returns (0) on success, (-1) on error.
	*/
	
	int status = 0;
	
	struct WalkDir walkdir = {0};
	
	if (walkdir_init(&walkdir, directory) == -1) {
		return -1;
	}
	
	while (1) {
		char* path = NULL;
		
		const struct WalkDirItem* const item = walkdir_next(&walkdir);
		
		if (item == NULL) {
			break;
		}
		
		if (strcmp(item->name, ".") == 0 || strcmp(item->name, "..") == 0) {
			continue;
		}
		
		path = malloc(strlen(directory) + strlen(PATHSEP) + strlen(item->name) + 1);
		
		if (path == NULL) {
			status = -1;
			break;
		}
		
		strcpy(path, directory);
		strcat(path, PATHSEP);
		strcat(path, item->name);
		
		switch (item->type) {
			case WALKDIR_ITEM_DIRECTORY: {
				if (remove_recursive(path, 1) == -1) {
					status = -1;
				}
				
				break;
			}
			case WALKDIR_ITEM_FILE:
			case WALKDIR_ITEM_UNKNOWN: {
				if (remove_file(path) == -1) {
					status = -1;
				}
				
				break;
			}
		}
		
		free(path);
		path = NULL;
		
		if (status == -1) {
			break;
		}
	}
	
	walkdir_free(&walkdir);
	
	if (status == -1) {
		return status;
	}
	
	if (remove_itself) {
		if (remove_empty_directory(directory) == -1) {
			status = -1;
		}
	}
	
	return status;
	
}

int directory_exists(const char* const directory) {
	/*
	Checks if directory exists.
	
	Returns (1) if directory exists, (0) if it does not exists, (-1) on error.
	*/
	
	#if defined(_WIN32)
		 DWORD attributes = 0;
		
		#if defined(_UNICODE)
			wchar_t* wdirectory = NULL;
			
			/* This prefix is required to support long paths in Windows 10+ */
			const size_t prefixs = isabsolute(directory) ? wcslen(WIN10_LONG_PATH_PREFIX) : 0;
			
			const int wdirectorys = MultiByteToWideChar(CP_UTF8, 0, directory, -1, NULL, 0);
			
			if (wdirectorys == 0) {
				return -1;
			}
			
			wdirectory = malloc((prefixs + (size_t) wdirectorys) * sizeof(*wdirectory));
			
			if (wdirectory == NULL) {
				return -1;
			}
			
			if (prefixs > 0) {
				wcscpy(wdirectory, WIN10_LONG_PATH_PREFIX);
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, directory, -1, wdirectory + prefixs, wdirectorys) == 0) {
				free(wdirectory);
				return -1;
			}
			
			attributes = GetFileAttributesW(wdirectory);
			
			free(wdirectory);
		#else
			attributes = GetFileAttributesA(directory);
		#endif
		
		if (attributes == INVALID_FILE_ATTRIBUTES) {
			const DWORD error = GetLastError();
			
			if (!(error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND)) {
				return -1;
			}
			
			return 0;
		}
		
		return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	#else
		struct stat st = {0};
		
		if (stat(directory, &st) == -1) {
			if (errno != ENOENT) {
				return -1;
			}
			
			return 0;
		}
		
		return S_ISDIR(st.st_mode);
	#endif
	
}

int file_exists(const char* const filename) {
	/*
	Checks if file exists and is a regular file or symlink.
	
	Returns (1) if file exists, (0) if it does not exists, (-1) on error.
	*/
	
	#if defined(_WIN32)
		DWORD attributes = 0;
		
		#if defined(_UNICODE)
			wchar_t* wfilename = NULL;
			
			/* This prefix is required to support long paths in Windows 10+ */
			const size_t prefixs = isabsolute(filename) ? wcslen(WIN10_LONG_PATH_PREFIX) : 0;
			
			const int wfilenames = MultiByteToWideChar(CP_UTF8, 0, filename, -1, NULL, 0);
			
			if (wfilenames == 0) {
				return -1;
			}
			
			wfilename = malloc((prefixs + (size_t) wfilenames) * sizeof(*wfilename));
			
			if (wfilename == NULL) {
				return -1;
			}
			
			if (prefixs > 0) {
				wcscpy(wfilename, WIN10_LONG_PATH_PREFIX);
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, filename, -1, wfilename + prefixs, wfilenames) == 0) {
				free(wfilename);
				return -1;
			}
			
			attributes = GetFileAttributesW(wfilename);
			
			free(wfilename);
		#else
			attributes = GetFileAttributesA(filename);
		#endif
		
		if (attributes == INVALID_FILE_ATTRIBUTES) {
			const DWORD error = GetLastError();
			
			if (!(error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND)) {
				return -1;
			}
			
			return 0;
		}
		
		return (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
	#else
		struct stat st = {0};
		
		if (stat(filename, &st) == -1) {
			if (errno != ENOENT) {
				return -1;
			}
			
			return 0;
		}
		
		return S_ISREG(st.st_mode);
	#endif
	
}

static int raw_create_dir(const char* const directory) {
	/*
	Try to create one directory (not the whole path).
	
	This is a thin wrapper over mkdir() (or alternatives on other systems),
	so in case of a pre-existing path we don't check that it is a directory.
	
	Returns (1) on success, (0) if it already exists, (-1) on error.
	*/
	
	#if defined(_WIN32)
		BOOL status = FALSE;
		
		#if defined(_UNICODE)
			wchar_t* wdirectory = NULL;
			
			/* This prefix is required to support long paths in Windows 10+ */
			const size_t prefixs = isabsolute(directory) ? wcslen(WIN10_LONG_PATH_PREFIX) : 0;
			
			const int wdirectorys = MultiByteToWideChar(CP_UTF8, 0, directory, -1, NULL, 0);
			
			if (wdirectorys == 0) {
				return -1;
			}
			
			wdirectory = malloc((prefixs + (size_t) wdirectorys) * sizeof(*wdirectory));
			
			if (wdirectory == NULL) {
				return -1;
			}
			
			if (prefixs > 0) {
				wcscpy(wdirectory, WIN10_LONG_PATH_PREFIX);
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, directory, -1, wdirectory + prefixs, wdirectorys) == 0) {
				free(wdirectory);
				return -1;
			}
			
			status = CreateDirectoryW(wdirectory, NULL);
			
			free(wdirectory);
		#else
			status = CreateDirectoryA(directory, NULL);
		#endif
		
		if (!status) {
			if (GetLastError() != ERROR_ALREADY_EXISTS) {
				return -1;
			}
			
			return 0;
		}
	#else
		if (mkdir(directory, 0777) == -1) {
			if (errno == EEXIST) {
				return 0;
			}
			
			#if defined(__HAIKU__)
				if (errno == EROFS) {
					return 0;
				}
			#endif
			
			return -1;
		}
	#endif
	
	return 1;
	
}

int create_directory(const char* const directory) {
	/*
	Creates the directory.
	
	The directory may contain several subdirectories that do not exist yet.
	The full path is created.
	
	It does not fail if the directory already exists.
	
	Returns (0) on success, (-1) on error.
	*/
	
	int omit_next = 0;
	size_t index = 0;
	
	const char* start = directory;
	
	#if defined(_WIN32)
		omit_next = isabsolute(directory);
	#endif
	
	for (index = 1; index < strlen(directory) + 1; index++) {
		const char* const ch = &directory[index];
		
		if (!(*ch == *PATHSEP || (*ch == '\0' && index > 1 && *(ch - 1) != *PATHSEP))) {
			continue;
		}
		
		if (omit_next) {
			omit_next = 0;
		} else {
			const size_t size = (size_t) (ch - start);
			char* path = malloc(size + 1);
			
			if (path == NULL) {
				return -1;
			}
			
			memcpy(path, start, size);
			path[size] = '\0';
			
			if (raw_create_dir(path) == -1) {
				free(path);
				return -1;
			}
			
			free(path);
		}
	}
	
	return 0;
	
}

int copy_file(const char* const source, const char* const destination) {
	/*
	Copies a file from source to destination.
	
	On the Windows platform this will copy the source file's attributes into destination.
	On Mac OS X, copyfile() C API will be used (available since OS X 10.5).
	
	If destination already exists, the file attributes will be preserved and the content overwritten.
	
	Returns (0) on success, (-1) on error.
	*/
	
	#if defined(_WIN32)
		#if defined(_UNICODE)
			wchar_t* wsource = NULL;
			wchar_t* wdestination = NULL;
			
			int wfilenames = 0;
			
			/* This prefix is required to support long paths in Windows 10+ */
			size_t prefixs = isabsolute(source) ? wcslen(WIN10_LONG_PATH_PREFIX) : 0;
			
			wfilenames = MultiByteToWideChar(CP_UTF8, 0, source, -1, NULL, 0);
			
			if (wfilenames == 0) {
				return -1;
			}
			
			wsource = malloc((prefixs + (size_t) wfilenames) * sizeof(*wsource));
			
			if (prefixs > 0) {
				wcscpy(wsource, WIN10_LONG_PATH_PREFIX);
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, source, -1, wsource + prefixs, wfilenames) == 0) {
				free(wsource);
				return -1;
			}
			
			prefixs = isabsolute(destination) ? wcslen(WIN10_LONG_PATH_PREFIX) : 0;
			
			wfilenames = MultiByteToWideChar(CP_UTF8, 0, destination, -1, NULL, 0);
			
			if (wfilenames == 0) {
				free(wsource);
				return -1;
			}
			
			wdestination = malloc((prefixs + (size_t) wfilenames) * sizeof(*wdestination));
			
			if (prefixs > 0) {
				wcscpy(wdestination, WIN10_LONG_PATH_PREFIX);
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, destination, -1, wdestination + prefixs, wfilenames) == 0) {
				free(wsource);
				free(wdestination);
				return -1;
			}
			
			if (CopyFileW(wsource, wdestination, FALSE) == 0) {
				free(wsource);
				free(wdestination);
				return -1;
			}
			
			free(wsource);
			free(wdestination);
		#else
			if (CopyFileA(source, destination, FALSE) == 0) {
				return -1;
			}
		#endif
	#elif defined(__APPLE__)
		copyfile_state_t state = copyfile_state_alloc();
		
		if (copyfile(source, destination, state, COPYFILE_DATA) != 0) {
			copyfile_state_free(state);
			return -1;
		}
		
		if (copyfile_state_free(state) != 0) {
			return -1;
		}
	#else
		/* Fallback implementation which works for any platform */
		struct FStream* istream = fstream_open(source, FSTREAM_READ);
		struct FStream* ostream = NULL;
		
		char chunk[8192];
		
		if (istream == NULL) {
			return -1;
		}
		
		ostream = fstream_open(destination, FSTREAM_WRITE);
		
		if (ostream == NULL) {
			fstream_close(istream);
			return -1;
		}
		
		while (1) {
			const ssize_t size = fstream_read(istream, chunk, sizeof(chunk));
			
			switch (size) {
				case FSTREAM_ERROR: {
					fstream_close(istream);
					fstream_close(ostream);
					
					return -1;
				}
				case FSTREAM_EOF: {
					if (fstream_close(istream) == -1 || fstream_close(ostream) == -1) {
						return -1;
					}
					
					break;
				}
				default: {
					if (fstream_write(ostream, chunk, (size_t) size) == -1) {
						fstream_close(istream);
						fstream_close(ostream);
						return -1;
					}
					
					break;
				}
			}
			
			if (size == FSTREAM_EOF) {
				break;
			}
		}
	#endif
	
	return 0;

}

int move_file(const char* const source, const char* const destination) {
	/*
	Moves a file from source to destination.
	
	Symlinks are not followed: if source is a symlink, it is itself moved, not it's target.
	If destination already exists, it will be overwritten.
	
	Returns (0) on success, (-1) on error.
	*/
	
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
				return -1;
			}
			
			wsource = malloc((prefixs + (size_t) wfilenames) * sizeof(*wsource));
			
			if (prefixs > 0) {
				wcscpy(wsource, WIN10_LONG_PATH_PREFIX);
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, source, -1, wsource + prefixs, wfilenames) == 0) {
				free(wsource);
				return -1;
			}
			
			prefixs = isabsolute(destination) ? wcslen(WIN10_LONG_PATH_PREFIX) : 0;
			
			wfilenames = MultiByteToWideChar(CP_UTF8, 0, destination, -1, NULL, 0);
			
			if (wfilenames == 0) {
				free(wsource);
				return -1;
			}
			
			wdestination = malloc((prefixs + (size_t) wfilenames) * sizeof(*wdestination));
			
			if (prefixs > 0) {
				wcscpy(wdestination, WIN10_LONG_PATH_PREFIX);
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, destination, -1, wdestination + prefixs, wfilenames) == 0) {
				free(wsource);
				free(wdestination);
				return -1;
			}
			
			status = MoveFileExW(wsource, wdestination, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING);
			
			free(wsource);
			free(wdestination);
		#else
			status = MoveFileExA(source, destination, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING);
		#endif
		
		if (!status) {
			if (GetLastError() != ERROR_ACCESS_DENIED) {
				return -1;
			}
			
			if (copy_file(source, destination) == -1) {
				return -1;
			}
			
			if (remove_file(source) == -1) {
				return -1;
			}
		}
	#else
		if (rename(source, destination) == -1) {
			if (errno != EXDEV) {
				return -1;
			}
			
			if (copy_file(source, destination) == -1) {
				return -1;
			}
			
			if (remove_file(source) == -1) {
				return -1;
			}
		}
	#endif
	
	return 0;
	
}

char* get_app_filename(void) {
	/*
	Returns the filename of the application's executable.
	
	Returns a null pointer on error.
	*/
	
	char* app_filename = NULL;
	
	#if defined(_WIN32)
		#if defined(_UNICODE)
			wchar_t* wfilename = NULL;
			DWORD filenames = 0;
			
			filenames = GetModuleFileNameW(0, NULL, 0);
			
			if (filenames == 0) {
				return NULL;
			}
			
			filenames++;
			
			wfilename = malloc(((size_t) filenames) * sizeof(*wfilename));
			
			if (wfilename == NULL) {
				return NULL;
			}
			
			filenames = GetModuleFileNameW(0, wfilename, filenames);
			
			if (filenames == 0) {
				free(wfilename);
			}
			
			app_filename = malloc((size_t) filenames);
			
			if (app_filename == NULL) {
				free(wfilename);
				return NULL;
			}
			
			if (WideCharToMultiByte(CP_UTF8, 0, wfilename, -1, app_filename, (int) filenames, NULL, NULL) == 0) {
				free(wfilename);
				free(app_filename);
				return NULL;
			}
			
			free(wfilename);
		#else
			app_filename = malloc(PATH_MAX);
			
			if (app_filename == NULL) {
				return NULL;
			}
			
			if (GetModuleFileNameA(0, app_filename, PATH_MAX) == 0) {
				free(app_filename);
				return NULL;
			}
		#endif
	#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
		#if defined(__NetBSD__)
			const int call[] = {CTL_KERN, KERN_PROC_ARGS, -1, KERN_PROC_PATHNAME};
		#else
			const int call[] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
		#endif
		
		size_t size = PATH_MAX;
		app_filename = malloc(size);
		
		if (app_filename == NULL) {
			return NULL;
		}
		
		if (sysctl(call, sizeof(call) / sizeof(*call), app_filename, &size, NULL, 0) == -1) {
			free(app_filename);
			return NULL;
		}
	#elif defined(__OpenBSD__)
		const pid_t pid = getpid();
		
		const int call[] = {CTL_KERN, KERN_PROC_ARGS, pid, KERN_PROC_ARGV};
		
		const char* path = NULL;
		const char* name = NULL;
		const char* start = NULL;
		
		char** argv = NULL;
		char* cwd = NULL;
		char* tmp = NULL;
		
		int relative = 0;
		size_t index = 0;
		size_t size = 0;
		
		app_filename = malloc(PATH_MAX);
		
		if (app_filename == NULL) {
			goto end;
		}
		
		app_filename[0] = '\0';
		
		if (sysctl(call, sizeof(call) / sizeof(*call), NULL, &size, NULL, 0) == -1) {
			goto end;
		}
		
		argv = malloc(size);
		
		if (argv == NULL) {
			goto end;
		}
		
		if (sysctl(call, sizeof(call) / sizeof(*call), argv, &size, NULL, 0) == -1) {
			goto end;
		}
		
		name = argv[0];
		
		if (isabsolute(name)) {
			realpath(name, app_filename);
			goto end;
		}
		
		/*
		Not an absolute path, check if it's relative to the current
		working directory.
		*/
		for (index = 1; index < strlen(name); index++) {
			const char ch = name[index];
			relative = (ch == PATHSEP[0]);
			
			if (relative) {
				break;
			}
		}
		
		if (relative) {
			cwd = malloc(PATH_MAX);
			
			if (cwd == NULL || getcwd(cwd, PATH_MAX) == NULL) {
				goto end;
			}
			
			tmp = malloc(strlen(cwd) + strlen(PATHSEP) + strlen(name) + 1);
			
			if (tmp == NULL) {
				goto end;
			}
			
			strcpy(tmp, cwd);
			strcat(tmp, PATHSEP);
			strcat(tmp, name);
			
			realpath(tmp, app_filename);
			
			goto end;
		}
		
		path = getenv("PATH");
		
		if (path == NULL) {
			goto end;
		}
		
		start = path;
		
		for (index = 0; index < strlen(path) + 1; index++) {
			const char* const ch = &path[index];
			
			if (!(*ch == ':' || *ch == '\0')) {
				continue;
			}
			
			size = (size_t) (ch - start);
			
			tmp = malloc(size + strlen(PATHSEP) + strlen(name) + 1);
			
			if (tmp == NULL) {
				goto end;
			}
			
			memcpy(tmp, start, size);
			tmp[size] = '\0';
			
			strcat(tmp, PATHSEP);
			strcat(tmp, name);
			
			switch (file_exists(tmp)) {
				case 1: {
					realpath(tmp, app_filename);
					goto end;
				}
				case -1: {
					goto end;
				}
			}
			
			start = (ch + 1);
		}
		
		errno = ENOENT;
		
		end:;
		
		free(argv);
		free(cwd);
		free(tmp);
		
		if (app_filename != NULL && app_filename[0] == '\0') {
			free(app_filename);
			app_filename = NULL;
		}
	#elif defined(__APPLE__)
		uint32_t paths = PATH_MAX;
		char* path = malloc((size_t) paths);
		
		if (path == NULL) {
			return NULL;
		}
		
		if (_NSGetExecutablePath(path, &paths) == -1) {
			free(path);
			return NULL;
		}
		
		app_filename = malloc(PATH_MAX);
		
		if (app_filename == NULL) {
			free(path);
			return NULL;
		}
		
		if (realpath(path, app_filename) == NULL) {
			free(path);
			free(app_filename);
			return NULL;
		}
		
		free(path);
	#elif defined(__HAIKU__)
		app_filename = malloc(PATH_MAX);
		
		if (app_filename == NULL) {
			return NULL;
		}
		
		if (find_path(NULL, B_FIND_PATH_IMAGE_PATH, NULL, app_filename, PATH_MAX) != B_OK) {
			free(app_filename);
			return NULL;
		}
	#else
		app_filename = malloc(PATH_MAX);
		
		if (app_filename == NULL) {
			return NULL;
		}
		
		if (readlink("/proc/self/exe", app_filename, PATH_MAX) == -1) {
			free(app_filename);
			return NULL;
		}
	#endif
	
	return app_filename;
	
}

char* expand_filename(const char* const filename) {
	/*
	Returns the full absolute path of a filename.
	
	Returns NULL on error.
	*/
	
	char* expanded_filename = NULL;
	
	#if defined(_WIN32)
		DWORD size = 0;
		
		#if defined(_UNICODE)
			wchar_t* wfilename = NULL;
			wchar_t* wexpanded_filename = NULL;
			DWORD expanded_filenames = 0;
			
			/* This prefix is required to support long paths in Windows 10+ */
			const size_t prefixs = isabsolute(filename) ? wcslen(WIN10_LONG_PATH_PREFIX) : 0;
			
			const int wfilenames = MultiByteToWideChar(CP_UTF8, 0, filename, -1, NULL, 0);
			
			if (wfilenames == 0) {
				return NULL;
			}
			
			wfilename = malloc((prefixs + (size_t) wfilenames) * sizeof(*wfilename));
			
			if (wfilename == NULL) {
				return NULL;
			}
			
			if (prefixs > 0) {
				wcscpy(wfilename, WIN10_LONG_PATH_PREFIX);
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, filename, -1, wfilename + prefixs, wfilenames) == 0) {
				free(wfilename);
				return NULL;
			}
			
			size = GetFullPathNameW(wfilename, 0, NULL, NULL);
			
			if (size == 0) {
				free(wfilename);
				return NULL;
			}
			
			wexpanded_filename = malloc(((size_t) size) * sizeof(*wexpanded_filename));
			
			if (wexpanded_filename == NULL) {
				free(wfilename);
				return NULL;
			}
			
			size = GetFullPathNameW(wfilename, size, wexpanded_filename, NULL);
			
			free(wfilename);
			
			if (size == 0) {
				free(wexpanded_filename);
				return NULL;
			}
			
			expanded_filenames = (DWORD) WideCharToMultiByte(CP_UTF8, 0, wexpanded_filename, -1, NULL, 0, NULL, NULL);
			
			if (expanded_filenames == 0) {
				free(wexpanded_filename);
				return NULL;
			}
			
			expanded_filename = malloc((size_t) expanded_filenames);
			
			if (expanded_filename == NULL) {
				free(wexpanded_filename);
				return NULL;
			}
			
			if (WideCharToMultiByte(CP_UTF8, 0, wexpanded_filename, -1, expanded_filename, (int) expanded_filenames, NULL, NULL) == 0) {
				free(wexpanded_filename);
				free(wexpanded_filename);
				return NULL;
			}
			
			free(wexpanded_filename);
		#else
			size = GetFullPathNameA(filename, 0, NULL, NULL);
			
			if (size == 0) {
				return NULL;
			}
			
			expanded_filename = malloc((size_t) size);
			
			if (expanded_filename == NULL) {
				return NULL;
			}
			
			size = GetFullPathNameA(filename, size, expanded_filename, NULL);
			
			if (size == 0) {
				free(expanded_filename);
				return NULL;
			}
		#endif
	#else
		int fd = 0;
		
		errno = 0;
		
		expanded_filename = realpath(filename, NULL);
		
		if (errno == ENOENT) {
			fd = open(filename, O_WRONLY | O_CREAT, 0666);
			
			if (fd == -1) {
				return NULL;
			}
			
			close(fd);
			
			expanded_filename = realpath(filename, NULL);
			
			unlink(filename);
		}
		
		if (expanded_filename == NULL) {
			return NULL;
		}
	#endif
	
	return expanded_filename;
	
}
