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
	
	int err = 0;
	char* cwd = NULL;
	
	#if defined(_WIN32)
		DWORD cwds = 0;
		
		#if defined(_UNICODE)
			wchar_t* wcwd = NULL;
			
			cwds = GetCurrentDirectoryW(0, NULL);
			
			if (cwds == 0) {
				err = -1;
				goto end;
			}
			
			wcwd = malloc((size_t) cwds * sizeof(*wcwd));
			
			if (wcwd == NULL) {
				err = -1;
				goto end;
			}
			
			cwds = GetCurrentDirectoryW(cwds, wcwd);
			
			if (cwds == 0) {
				err = -1;
				goto end;
			}
			
			cwds = (DWORD) WideCharToMultiByte(CP_UTF8, 0, wcwd, -1, NULL, 0, NULL, NULL);
			
			if (cwds == 0) {
				err = -1;
				goto end;
			}
			
			cwd = malloc((size_t) cwds);
			
			if (cwd == NULL) {
				err = -1;
				goto end;
			}
			
			if (WideCharToMultiByte(CP_UTF8, 0, wcwd, -1, cwd, (int) cwds, NULL, NULL) == 0) {
				err = -1;
				goto end;
			}
		#else
			cwds = GetCurrentDirectoryA(0, NULL);
			
			if (cwds == 0) {
				err = -1;
				goto end;
			}
			
			cwd = malloc((size_t) cwds);
			
			if (cwd == NULL) {
				err = -1;
				goto end;
			}
			
			cwds = GetCurrentDirectoryA(cwds, cwd);
			
			if (cwds == 0) {
				err = -1;
				goto end;
			}
		#endif
	#else
		cwd = malloc(PATH_MAX);
		
		if (cwd == NULL) {
			err = -1;
			goto end;
		}
		
		if (getcwd(cwd, PATH_MAX) == NULL) {
			err = -1;
			goto end;
		}
	#endif
	
	end:;
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(wcwd);
	#endif
	
	if (err != 0) {
		free(cwd);
		cwd = NULL;
	}
	
	return cwd;
	
}

int remove_file(const char* const filename) {
	/*
	Removes a file from disk.
	
	On Windows, ignores the read-only attribute.
	This does not fail if the file never existed in the first place.
	
	Returns (0) on success, (-1) on error.
	*/
	
	int err = 0;
	
	#if defined(_WIN32)
		BOOL status = FALSE;
		
		#if defined(_UNICODE)
			wchar_t* wfilename = NULL;
			
			/* This prefix is required to support long paths in Windows 10+ */
			const size_t prefixs = isabsolute(filename) ? wcslen(WIN10_LONG_PATH_PREFIX) : 0;
			
			const int wfilenames = MultiByteToWideChar(CP_UTF8, 0, filename, -1, NULL, 0);
			
			if (wfilenames == 0) {
				err = -1;
				goto end;
			}
			
			wfilename = malloc((prefixs + (size_t) wfilenames) * sizeof(*wfilename));
			
			if (prefixs > 0) {
				wcscpy(wfilename, WIN10_LONG_PATH_PREFIX);
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, filename, -1, wfilename + prefixs, wfilenames) == 0) {
				err = -1;
				goto end;
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
						err = -1;
						goto end;
					}
					
					/* Now that this attribute is gone, let's try to delete that file again. */
					#if defined(_UNICODE)
						status = DeleteFileW(wfilename);
					#else
						status = DeleteFileA(filename);
					#endif
					
					if (!status) {
						err = -1;
						goto end;
					}
				
					break;
				}
				case ERROR_FILE_NOT_FOUND:
				case ERROR_PATH_NOT_FOUND: {
					/* The file never existed in the first place; that's not an error. */
					break;
				}
				default: {
					err = -1;
					goto end;
				}
			}
		}
	#else
		if (unlink(filename) == -1 && errno != ENOENT) {
			err = -1;
			goto end;
		}
	#endif
	
	end:;
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(wfilename);
	#endif
	
	return err;
	
}

static int remove_empty_directory(const char* const directory) {
	/*
	Deletes an existing empty directory.
	
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
			
			status = RemoveDirectoryW(wdirectory);
		#else
			status = RemoveDirectoryA(directory);
		#endif
		
		if (!status) {
			err = -1;
			goto end;
		}
	#else
		if (rmdir(directory) == -1) {
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

int remove_recursive(const char* const directory, const int remove_itself) {
	/*
	Recursively removes a directory from disk.
	
	Returns (0) on success, (-1) on error.
	*/
	
	int err = 0;
	
	char* path = NULL;
	
	struct WalkDir walkdir = {0};
	const struct WalkDirItem* item = NULL;
	
	if (walkdir_init(&walkdir, directory) == -1) {
		err = -1;
		goto end;
	}
	
	while (1) {
		item = walkdir_next(&walkdir);
		
		if (item == NULL) {
			goto end;
		}
		
		if (strcmp(item->name, ".") == 0 || strcmp(item->name, "..") == 0) {
			continue;
		}
		
		path = malloc(strlen(directory) + strlen(PATHSEP_S) + strlen(item->name) + 1);
		
		if (path == NULL) {
			err = -1;
			goto end;
		}
		
		strcpy(path, directory);
		strcat(path, PATHSEP_S);
		strcat(path, item->name);
		
		switch (item->type) {
			case WALKDIR_ITEM_DIRECTORY: {
				if (remove_recursive(path, 1) == -1) {
					err = -1;
					goto end;
				}
				
				break;
			}
			case WALKDIR_ITEM_FILE:
			case WALKDIR_ITEM_UNKNOWN: {
				if (remove_file(path) == -1) {
					err = -1;
					goto end;
				}
				
				break;
			}
		}
		
		free(path);
		path = NULL;
	}
	
	end:;
	
	free(path);
	walkdir_free(&walkdir);
	
	if (err == -1) {
		return err;
	}
	
	if (remove_itself && remove_empty_directory(directory) == -1) {
		err = -1;
	}
	
	return err;
	
}

int directory_exists(const char* const directory) {
	/*
	Checks if directory exists.
	
	Returns (1) if directory exists, (0) if it does not exists, (-1) on error.
	*/
	
	int err = 0;
	
	#if defined(_WIN32)
		DWORD error = 0;
		DWORD attributes = 0;
		
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
			
			attributes = GetFileAttributesW(wdirectory);
		#else
			attributes = GetFileAttributesA(directory);
		#endif
		
		if (attributes == INVALID_FILE_ATTRIBUTES) {
			error = GetLastError();
			
			if (!(error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND)) {
				err = -1;
				goto end;
			}
			
			goto end;
		}
		
		err = (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	#else
		struct stat st = {0};
		
		if (stat(directory, &st) == -1) {
			if (errno != ENOENT) {
				err = -1;
				goto end;
			}
			
			goto end;
		}
		
		err = S_ISDIR(st.st_mode);
	#endif
	
	end:;
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(wdirectory);
	#endif
	
	return err;
	
}

int file_exists(const char* const filename) {
	/*
	Checks if file exists and is a regular file or symlink.
	
	Returns (1) if file exists, (0) if it does not exists, (-1) on error.
	*/
	
	int err = 0;
	
	#if defined(_WIN32)
		DWORD error = 0;
		DWORD attributes = 0;
		
		#if defined(_UNICODE)
			wchar_t* wfilename = NULL;
			
			/* This prefix is required to support long paths in Windows 10+ */
			const size_t prefixs = isabsolute(filename) ? wcslen(WIN10_LONG_PATH_PREFIX) : 0;
			
			const int wfilenames = MultiByteToWideChar(CP_UTF8, 0, filename, -1, NULL, 0);
			
			if (wfilenames == 0) {
				err = -1;
				goto end;
			}
			
			wfilename = malloc((prefixs + (size_t) wfilenames) * sizeof(*wfilename));
			
			if (wfilename == NULL) {
				err = -1;
				goto end;
			}
			
			if (prefixs > 0) {
				wcscpy(wfilename, WIN10_LONG_PATH_PREFIX);
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, filename, -1, wfilename + prefixs, wfilenames) == 0) {
				err = -1;
				goto end;
			}
			
			attributes = GetFileAttributesW(wfilename);
		#else
			attributes = GetFileAttributesA(filename);
		#endif
		
		if (attributes == INVALID_FILE_ATTRIBUTES) {
			error = GetLastError();
			
			if (!(error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND)) {
				err = -1;
				goto end;
			}
			
			goto end;
		}
		
		err = (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
	#else
		struct stat st = {0};
		
		if (stat(filename, &st) == -1) {
			if (errno != ENOENT) {
				err = -1;
				goto end;
			}
			
			goto end;
		}
		
		err = S_ISREG(st.st_mode);
	#endif
	
	end:;
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(wfilename);
	#endif
	
	return err;
	
}

static int raw_create_dir(const char* const directory) {
	/*
	Try to create one directory (not the whole path).
	
	This is a thin wrapper over mkdir() (or alternatives on other systems),
	so in case of a pre-existing path we don't check that it is a directory.
	
	Returns (1) on success, (0) if it already exists, (-1) on error.
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
			
			status = CreateDirectoryW(wdirectory, NULL);
		#else
			status = CreateDirectoryA(directory, NULL);
		#endif
		
		if (!status) {
			if (GetLastError() != ERROR_ALREADY_EXISTS) {
				err = -1;
				goto end;
			}
			
			goto end;
		}
	#else
		if (mkdir(directory, 0777) == -1) {
			if (errno == EEXIST) {
				goto end;
			}
			
			#if defined(__HAIKU__)
				if (errno == EROFS) {
					goto end;
				}
			#endif
			
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

int create_directory(const char* const directory) {
	/*
	Creates the directory.
	
	The directory may contain several subdirectories that do not exist yet.
	The full path is created.
	
	It does not fail if the directory already exists.
	
	Returns (0) on success, (-1) on error.
	*/
	
	int err = 0;
	
	size_t index = 0;
	size_t size = 0;
	size_t len = 0;
	
	char* path = NULL;
	const char* start = directory;
	
	#if defined(_WIN32)
		int omit_next = isabsolute(directory);
	#endif
	
	len = strlen(directory) + 1;
	
	for (index = 1; index < len; index++) {
		const char* const pos = &directory[index];
		
		const unsigned char cur = pos[0];
		const unsigned char prev = (index > 1) ? *(pos - 1) : PATHSEP;
		
		if (!(cur == PATHSEP || (cur == '\0' && prev != PATHSEP))) {
			continue;
		}
		
		#if defined(_WIN32)
			if (omit_next) {
				omit_next = 0;
				continue;
			}
		#endif
		
		size = (size_t) (pos - start);
		path = malloc(size + 1);
		
		if (path == NULL) {
			err = -1;
			goto end;
		}
		
		memcpy(path, start, size);
		path[size] = '\0';
		
		if (raw_create_dir(path) == -1) {
			err = -1;
			goto end;
		}
		
		free(path);
		path = NULL;
	}
	
	end:;
	
	free(path);
	
	return err;
	
}

int copy_file(const char* const source, const char* const destination) {
	/*
	Copies a file from source to destination.
	
	On the Windows platform this will copy the source file's attributes into destination.
	On Mac OS X, copyfile() C API will be used (available since OS X 10.5).
	
	If destination already exists, the file attributes will be preserved and the content overwritten.
	
	Returns (0) on success, (-1) on error.
	*/
	
	int err = 0;
	
	#if defined(_WIN32)
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
			
			if (CopyFileW(wsource, wdestination, FALSE) == 0) {
				err = -1;
				goto end;
			}
		#else
			if (CopyFileA(source, destination, FALSE) == 0) {
				err = -1;
				goto end;
			}
		#endif
	#elif defined(__APPLE__)
		copyfile_state_t state = copyfile_state_alloc();
		
		if (copyfile(source, destination, state, COPYFILE_DATA) != 0) {
			err = -1;
			goto end;
		}
	#else
		/* Fallback implementation which works for any platform */
		struct FStream* input = fstream_open(source, FSTREAM_READ);
		struct FStream* output = NULL;
		
		ssize_t size = 0;
		
		char chunk[8192];
		
		if (input == NULL) {
			err = -1;
			goto end;
		}
		
		output = fstream_open(destination, FSTREAM_WRITE);
		
		if (output == NULL) {
			err = -1;
			goto end;
		}
		
		while (1) {
			size = fstream_read(input, chunk, sizeof(chunk));
			
			switch (size) {
				case FSTREAM_ERROR: {
					err = -1;
					goto end;
				}
				case FSTREAM_EOF: {
					goto end;
				}
				default: {
					if (fstream_write(output, chunk, (size_t) size) == -1) {
						err = -1;
						goto end;
					}
					
					break;
				}
			}
		}
	#endif
	
	end:;
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(wsource);
		free(wdestination);
	#elif defined(__APPLE__)
		copyfile_state_free(state);
	#else
		fstream_close(input);
		fstream_close(output);
	#endif
	
	return err;

}

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

char* get_app_filename(void) {
	/*
	Returns the filename of the application's executable.
	
	Returns a null pointer on error.
	*/
	
	int err = 0;
	
	char* app_filename = NULL;
	
	#if defined(_WIN32)
		#if defined(_UNICODE)
			wchar_t* wfilename = NULL;
			DWORD filenames = 0;
			
			filenames = GetModuleFileNameW(0, NULL, 0);
			
			if (filenames == 0) {
				err = -1;
				goto end;
			}
			
			filenames++;
			
			wfilename = malloc(((size_t) filenames) * sizeof(*wfilename));
			
			if (wfilename == NULL) {
				err = -1;
				goto end;
			}
			
			filenames = GetModuleFileNameW(0, wfilename, filenames);
			
			if (filenames == 0) {
				err = -1;
				goto end;
			}
			
			app_filename = malloc((size_t) filenames);
			
			if (app_filename == NULL) {
				err = -1;
				goto end;
			}
			
			if (WideCharToMultiByte(CP_UTF8, 0, wfilename, -1, app_filename, (int) filenames, NULL, NULL) == 0) {
				err = -1;
				goto end;
			}
		#else
			app_filename = malloc(PATH_MAX);
			
			if (app_filename == NULL) {
				err = -1;
				goto end;
			}
			
			if (GetModuleFileNameA(0, app_filename, PATH_MAX) == 0) {
				err = -1;
				goto end;
			}
		#endif
	#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
		#if defined(__NetBSD__)
			const int call[] = {
				CTL_KERN,
				KERN_PROC_ARGS,
				-1,
				KERN_PROC_PATHNAME
			};
		#else
			const int call[] = {
				CTL_KERN,
				KERN_PROC,
				KERN_PROC_PATHNAME,
				-1
			};
		#endif
		
		size_t size = PATH_MAX;
		app_filename = malloc(size);
		
		if (app_filename == NULL) {
			err = -1;
			goto end;
		}
		
		if (sysctl(call, sizeof(call) / sizeof(*call), app_filename, &size, NULL, 0) == -1) {
			err = -1;
			goto end;
		}
	#elif defined(__OpenBSD__)
		const pid_t pid = getpid();
		
		const int call[] = {
			CTL_KERN,
			KERN_PROC_ARGS,
			pid,
			KERN_PROC_ARGV
		};
		
		const char* path = NULL;
		const char* name = NULL;
		const char* start = NULL;
		
		char** argv = NULL;
		char* cwd = NULL;
		char* tmp = NULL;
		
		int status = 0;
		int relative = 0;
		size_t index = 0;
		size_t size = 0;
		
		app_filename = malloc(PATH_MAX);
		
		if (app_filename == NULL) {
			err = -1;
			goto end;
		}
		
		app_filename[0] = '\0';
		
		if (sysctl(call, sizeof(call) / sizeof(*call), NULL, &size, NULL, 0) == -1) {
			err = -1;
			goto end;
		}
		
		argv = malloc(size);
		
		if (argv == NULL) {
			err = -1;
			goto end;
		}
		
		if (sysctl(call, sizeof(call) / sizeof(*call), argv, &size, NULL, 0) == -1) {
			err = -1;
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
			const char unsigned ch = name[index];
			
			status = (ch == PATHSEP);
			
			if (status) {
				break;
			}
		}
		
		if (status) {
			cwd = malloc(PATH_MAX);
			
			if (cwd == NULL) {
				err = -1;
				goto end;
			}
			
			if (getcwd(cwd, PATH_MAX) == NULL) {
				err = -1;
				goto end;
			}
			
			tmp = malloc(strlen(cwd) + strlen(PATHSEP_S) + strlen(name) + 1);
			
			if (tmp == NULL) {
				err = -1;
				goto end;
			}
			
			strcpy(tmp, cwd);
			strcat(tmp, PATHSEP_S);
			strcat(tmp, name);
			
			realpath(tmp, app_filename);
			
			goto end;
		}
		
		path = getenv("PATH");
		
		if (path == NULL) {
			err = -1;
			goto end;
		}
		
		start = path;
		
		for (index = 0; index < strlen(path) + 1; index++) {
			const char* const pos = &path[index];
			const unsigned char ch = *pos;
			
			if (!(ch == ':' || ch == '\0')) {
				continue;
			}
			
			size = (size_t) (pos - start);
			
			tmp = malloc(size + strlen(PATHSEP_S) + strlen(name) + 1);
			
			if (tmp == NULL) {
				goto end;
			}
			
			memcpy(tmp, start, size);
			tmp[size] = '\0';
			
			strcat(tmp, PATHSEP_S);
			strcat(tmp, name);
			
			status = file_exists(tmp);
			
			if (status) {
				realpath(tmp, app_filename);
				goto end;
			}
			
			if (status == -1) {
				err = -1;
				goto end;
			}
			
			free(tmp);
			tmp = NULL;
			
			start = (pos + 1);
		}
	#elif defined(__APPLE__)
		uint32_t paths = PATH_MAX;
		char* path = malloc((size_t) paths);
		
		if (path == NULL) {
			err = -1;
			goto end;
		}
		
		if (_NSGetExecutablePath(path, &paths) == -1) {
			err = -1;
			goto end;
		}
		
		app_filename = malloc(PATH_MAX);
		
		if (app_filename == NULL) {
			err = -1;
			goto end;
		}
		
		if (realpath(path, app_filename) == NULL) {
			err = -1;
			goto end;
		}
	#elif defined(__HAIKU__)
		app_filename = malloc(PATH_MAX);
		
		if (app_filename == NULL) {
			err = -1;
			goto end;
		}
		
		if (find_path(NULL, B_FIND_PATH_IMAGE_PATH, NULL, app_filename, PATH_MAX) != B_OK) {
			err = -1;
			goto end;
		}
	#else
		ssize_t wsize = 0;
		
		app_filename = malloc(PATH_MAX);
		
		if (app_filename == NULL) {
			err = -1;
			goto end;
		}
		
		wsize = readlink("/proc/self/exe", app_filename, PATH_MAX);
		
		if (wsize == -1) {
			err = -1;
			goto end;
		}
		
		app_filename[wsize] = '\0';
	#endif
	
	end:;
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(wfilename);
	#endif
	
	#if defined(__OpenBSD__)
		free(argv);
		free(cwd);
		free(tmp);
	#endif
	
	#if defined(__APPLE__)
		free(path);
	#endif
	
	if (err != 0) {
		free(app_filename);
		app_filename = NULL;
	}
	
	return app_filename;
	
}

char* expand_filename(const char* const filename) {
	/*
	Returns the full absolute path of a filename.
	
	Returns NULL on error.
	*/
	
	int err = 0;
	
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
				err = -1;
				goto end;
			}
			
			wfilename = malloc((prefixs + (size_t) wfilenames) * sizeof(*wfilename));
			
			if (wfilename == NULL) {
				err = -1;
				goto end;
			}
			
			if (prefixs > 0) {
				wcscpy(wfilename, WIN10_LONG_PATH_PREFIX);
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, filename, -1, wfilename + prefixs, wfilenames) == 0) {
				err = -1;
				goto end;
			}
			
			size = GetFullPathNameW(wfilename, 0, NULL, NULL);
			
			if (size == 0) {
				err = -1;
				goto end;
			}
			
			wexpanded_filename = malloc(((size_t) size) * sizeof(*wexpanded_filename));
			
			if (wexpanded_filename == NULL) {
				err = -1;
				goto end;
			}
			
			size = GetFullPathNameW(wfilename, size, wexpanded_filename, NULL);
			
			free(wfilename);
			
			if (size == 0) {
				err = -1;
				goto end;
			}
			
			expanded_filenames = (DWORD) WideCharToMultiByte(CP_UTF8, 0, wexpanded_filename, -1, NULL, 0, NULL, NULL);
			
			if (expanded_filenames == 0) {
				err = -1;
				goto end;
			}
			
			expanded_filename = malloc((size_t) expanded_filenames);
			
			if (expanded_filename == NULL) {
				err = -1;
				goto end;
			}
			
			if (WideCharToMultiByte(CP_UTF8, 0, wexpanded_filename, -1, expanded_filename, (int) expanded_filenames, NULL, NULL) == 0) {
				err = -1;
				goto end;
			}
		#else
			size = GetFullPathNameA(filename, 0, NULL, NULL);
			
			if (size == 0) {
				err = -1;
				goto end;
			}
			
			expanded_filename = malloc((size_t) size);
			
			if (expanded_filename == NULL) {
				err = -1;
				goto end;
			}
			
			size = GetFullPathNameA(filename, size, expanded_filename, NULL);
			
			if (size == 0) {
				err = -1;
				goto end;
			}
		#endif
	#else
		int fd = 0;
		
		errno = 0;
		
		expanded_filename = realpath(filename, NULL);
		
		/* FIXME: avoid creating a file */
		if (errno == ENOENT) {
			fd = open(filename, O_WRONLY | O_CREAT, 0666);
			
			if (fd == -1) {
				err = -1;
				goto end;
			}
			
			close(fd);
			
			expanded_filename = realpath(filename, NULL);
			
			unlink(filename);
		}
		
		if (expanded_filename == NULL) {
			err = -1;
			goto end;
		}
	#endif
	
	end:;
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(wfilename);
		free(wexpanded_filename);
	#endif
	
	if (err != 0) {
		free(expanded_filename);
		expanded_filename = NULL;
	}
	
	return expanded_filename;
	
}
