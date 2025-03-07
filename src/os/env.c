#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
	#include <windows.h>
	#include <fileapi.h>
#endif

#include "path.h"
#include "pathsep.h"
#include "fs/exists.h"
#include "os/env.h"

#if defined(_WIN32)
	#if defined(_UNICODE)
		static const wchar_t WENV_APPDATA[] = L"APPDATA";
		static const wchar_t WENV_PATH[] = L"PATH";
	#else
		static const char ENV_USERPROFILE[] = "USERPROFILE";
		static const char ENV_APPDATA[] = "APPDATA";
		static const char ENV_PATH[] = "PATH";
	#endif
#endif

#if !defined(_WIN32)
	static const char ENV_XDG_CONFIG_HOME[] = "XDG_CONFIG_HOME";
	static const char ENV_HOME[] = "HOME";
	static const char ENV_PATH[] = "PATH";
	
	static const char* const ENV_TEMPORARY_DIRECTORY[] = {
		"TMPDIR",
		"TEMP",
		"TMP",
		"TEMPDIR"
	};
	
	static const char* const TEMPORARY_DIRECTORIES[] = {
		"/var/tmp",
		"/usr/tmp",
		"/tmp"
	};
	
	static const char DEFAULT_CONFIGURATION_DIRECTORY[] = ".config";
#endif


char* get_configuration_directory(void) {
	/*
	Returns the config directory of the current user for applications.
	
	On non-Windows OSs, this proc conforms to the XDG Base Directory
	spec. Thus, this proc returns the value of the "XDG_CONFIG_HOME" environment
	variable if it is set, otherwise it returns the default configuration directory ("~/.config/").
	
	Returns a null pointer on error.
	*/
	
	int err = 0;
	
	char* configuration_directory = NULL;
	
	#if defined(_WIN32)
		#if defined(_UNICODE)
			int directorys = 0;
			
			const wchar_t* const wdirectory = _wgetenv(WENV_APPDATA);
			
			if (wdirectory == NULL) {
				err = -1;
				goto end;
			}
			
			directorys = WideCharToMultiByte(CP_UTF8, 0, wdirectory, -1, NULL, 0, NULL, NULL);
			
			if (directorys == 0) {
				err = -1;
				goto end;
			}
			
			configuration_directory = malloc((size_t) directorys);
			
			if (configuration_directory == NULL) {
				err = -1;
				goto end;
			}
			
			if (WideCharToMultiByte(CP_UTF8, 0, wdirectory, -1, configuration_directory, directorys, NULL, NULL) == 0) {
				err = -1;
				goto end;
			}
		#else
			const char* const directory = getenv(ENV_APPDATA);
			
			if (directory == NULL) {
				err = -1;
				goto end;
			}
			
			configuration_directory = malloc(strlen(directory) + 1);
			
			if (configuration_directory == NULL) {
				err = -1;
				goto end;
			}
			
			strcpy(configuration_directory, directory);
		#endif
	#else
		const char* const directory = getenv(ENV_XDG_CONFIG_HOME);
		const char* const home = getenv(ENV_HOME);
		
		if (directory != NULL) {
			configuration_directory = malloc(strlen(directory) + 1);
			
			if (configuration_directory == NULL) {
				err = -1;
				goto end;
			}
			
			strcpy(configuration_directory, directory);
			
			goto end;
		}
		
		if (home == NULL) {
			err = -1;
			goto end;
		}
		
		configuration_directory = malloc(strlen(home) + strlen(PATHSEP_S) + strlen(DEFAULT_CONFIGURATION_DIRECTORY) + 1);
		
		if (configuration_directory == NULL) {
			err = -1;
			goto end;
		}
		
		strcpy(configuration_directory, home);
		strcat(configuration_directory, PATHSEP_S);
		strcat(configuration_directory, DEFAULT_CONFIGURATION_DIRECTORY);
	#endif
	
	end:;
	
	if (err != 0) {
		free(configuration_directory);
		configuration_directory = NULL;
	}
	
	if (err == 0) {
		strip_separator(configuration_directory);
	}
	
	return configuration_directory;
	
}

char* get_temporary_directory(void) {
	/*
	Returns the temporary directory of the current user for applications to
	save temporary files in.
	
	On Windows, it calls GetTempPath().
	On Posix based platforms, it will check "TMPDIR", "TEMP", "TMP" and
	"TEMPDIR" environment variables in order.
	
	Returns NULL on error.
	*/
	
	int err = 0;
	
	char* temporary_directory = NULL;
	
	#if defined(_WIN32)
		DWORD directorys = 0;
		
		#if defined(_UNICODE)
			wchar_t* wdirectory = NULL;
			
			directorys = GetTempPathW(0, NULL);
			
			if (directorys == 0) {
				return NULL;
			}
			
			directorys++;
			
			wdirectory = malloc((size_t) directorys * sizeof(*wdirectory));
			
			if (wdirectory == NULL) {
				err = -1;
				goto end;
			}
			
			directorys = GetTempPathW((DWORD) directorys, wdirectory);
			
			if (directorys == 0) {
				err = -1;
				goto end;
			}
			
			directorys = (DWORD) WideCharToMultiByte(CP_UTF8, 0, wdirectory, -1, NULL, 0, NULL, NULL);
			
			if (directorys == 0) {
				err = -1;
				goto end;
			}
			
			temporary_directory = malloc((size_t) directorys);
			
			if (temporary_directory == NULL) {
				err = -1;
				goto end;
			}
			
			if (WideCharToMultiByte(CP_UTF8, 0, wdirectory, -1, temporary_directory, (int) directorys, NULL, NULL) == 0) {
				err = -1;
				goto end;
			}
		#else
			directorys = GetTempPathA(0, NULL);
			
			if (directorys == 0) {
				err = -1;
				goto end;
			}
			
			directorys++;
			
			temporary_directory = malloc(directorys);
			
			if (temporary_directory == NULL) {
				err = -1;
				goto end;
			}
			
			directorys = GetTempPathA((DWORD) directorys, temporary_directory);
			
			if (directorys == 0) {
				err = -1;
				goto end;
			}
		#endif
		
		strip_separator(temporary_directory);
	#else
		size_t index = 0;
		
		const char* name = NULL;
		const char* directory = NULL;
		
		/*
		We should check first for the TEMP* and TMP* environment variables.
		*/
		for (index = 0; index < sizeof(ENV_TEMPORARY_DIRECTORY) / sizeof(*ENV_TEMPORARY_DIRECTORY); index++) {
			name = ENV_TEMPORARY_DIRECTORY[index];
			directory = getenv(name);
			
			if (directory == NULL) {
				continue;
			}
			
			if (!directory_exists(directory)) {
				continue;
			}
			
			temporary_directory = malloc(strlen(directory) + 1);
			
			if (temporary_directory == NULL) {
				err = -1;
				goto end;
			}
			
			strcpy(temporary_directory, directory);
			
			goto end;
		}
		
		/*
		The directories are checked in the following order:
		
		- /var/tmp
		- /usr/tmp
		- /tmp
		
		We prefer /var/tmp and /usr/tmp over /tmp because these locations are more
		suitable for storing large amounts of data.
		
		Since /tmp is commonly mounted as a ramdisk-based filesystem, its size is generally
		set to be 50% of the total amount of RAM. On embedded devices this size might be
		too low for us.
		*/
		for (index = 0; index < sizeof(TEMPORARY_DIRECTORIES) / sizeof(*TEMPORARY_DIRECTORIES); index++) {
			directory = TEMPORARY_DIRECTORIES[index];
			
			if (!directory_exists(directory)) {
				continue;
			}
			
			temporary_directory = malloc(strlen(directory) + 1);
			
			if (temporary_directory == NULL) {
				err = -1;
				goto end;
			}
			
			strcpy(temporary_directory, directory);
			
			goto end;
		}
	#endif
	
	end:;
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(wdirectory);
	#endif
	
	if (err != 0) {
		free(temporary_directory);
		temporary_directory = NULL;
	}
	
	return temporary_directory;
	
}

char* get_home_directory(void) {
	/*
	Returns the home directory of the current user.
	
	On Windows, it returns the value from "USERPROFILE" environment variable.
	On Posix based platforms, it returns the value from "HOME" environment variable.
	
	Returns NULL on error.
	*/
	
	int err = 0;
	
	char* home = NULL;
	
	#if defined(_WIN32)
		#if defined(_UNICODE)
			int directorys = 0;
			
			const wchar_t* const wdirectory = _wgetenv(WENV_APPDATA);
			
			if (wdirectory == NULL) {
				return NULL;
			}
			
			directorys = WideCharToMultiByte(CP_UTF8, 0, wdirectory, -1, NULL, 0, NULL, NULL);
			
			if (directorys == 0) {
				err = -1;
				goto end;
			}
			
			home = malloc((size_t) directorys);
			
			if (home == NULL) {
				err = -1;
				goto end;
			}
			
			if (WideCharToMultiByte(CP_UTF8, 0, wdirectory, -1, home, directorys, NULL, NULL) == 0) {
				err = -1;
				goto end;
			}
		#else
			const char* const directory = getenv(ENV_USERPROFILE);
			
			if (directory == NULL) {
				err = -1;
				goto end;
			}
			
			home = malloc(strlen(directory) + 1);
			
			if (home == NULL) {
				err = -1;
				goto end;
			}
			
			strcpy(home, directory);
		#endif
	#else
		const char* const directory = getenv(ENV_HOME);
		
		if (directory == NULL) {
			err = -1;
			goto end;
		}
		
		home = malloc(strlen(directory) + 1);
		
		if (home == NULL) {
			err = -1;
			goto end;
		}
		
		strcpy(home, directory);
	#endif
	
	end:;
	
	if (err != 0) {
		free(home);
		home = NULL;
	}
	
	return home;
	
}

char* find_exe(const char* const name) {
	
	char* executable = NULL;
	char* path = NULL;
	const char* component = NULL;
	
	int err = 0;
	
	size_t index = 0;
	size_t size = 0;
	size_t length = 0;
	
	#if defined(_WIN32)
		const char* const executable_extension = ".exe";
		const unsigned char separator = ';';
	#else
		const char* const executable_extension = "";
		const unsigned char separator = ':';
	#endif
	
	#if defined(_WIN32) && defined(_UNICODE)
		int paths = 0;
		
		const wchar_t* const wpath = _wgetenv(WENV_PATH);
		
		if (wpath == NULL) {
			err = -1;
			goto end;
		}
		
		paths = WideCharToMultiByte(CP_UTF8, 0, wpath, -1, NULL, 0, NULL, NULL);
		
		if (paths == 0) {
			err = -1;
			goto end;
		}
		
		path = malloc((size_t) paths);
		
		if (path == NULL) {
			err = -1;
			goto end;
		}
		
		if (WideCharToMultiByte(CP_UTF8, 0, wpath, -1, path, paths, NULL, NULL) == 0) {
			err = -1;
			goto end;
		}
	#else
		path = getenv(ENV_PATH);
		
		if (path == NULL) {
			err = -1;
			goto end;
		}
	#endif
	
	component = path;
	length = strlen(path) + 1;
	
	for (index = 0; index < length; index++) {
		const char* const pos = &path[index];
		const unsigned char ch = *pos;
		
		if (!(ch == separator || ch == '\0')) {
			continue;
		}
		
		size = (size_t) (pos - component);
		executable = malloc(size + strlen(PATHSEP_S) + strlen(name) + strlen(executable_extension) + 1);
		
		if (executable == NULL) {
			err = -1;
			goto end;
		}
		
		memcpy(executable, component, size);
		executable[size] = '\0';
		
		strcat(executable, PATHSEP_S);
		strcat(executable, name);
		strcat(executable, executable_extension);
		
		if (file_exists(executable)) {
			goto end;
		}
		
		free(executable);
		executable = NULL;
		
		component = (pos + 1);
	}
	
	end:;
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(path);
	#endif
	
	if (err != 0) {
		free(executable);
		executable = NULL;
	}
	
	return executable;
	
}
