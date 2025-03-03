#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
	#include <windows.h>
	#include <sysinfoapi.h>
#endif

#if defined(__OpenBSD__) || defined(__NetBSD__)
	#include <sys/param.h>
#endif

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__) || defined(__OpenBSD__)
	#include <sys/sysctl.h>
#endif

#if defined(__HAIKU__)
	#include <OS.h>
#endif

#if !defined(_WIN32)
	#include <unistd.h>
	#include <sys/wait.h>
	#include <sys/types.h>
#endif

#if defined(_WIN32)
	#if defined(_UNICODE)
		static const wchar_t WENV_APPDATA[] = L"APPDATA";
		static const wchar_t WENV_PATH[] = L"PATH";
		
		static const wchar_t WNTDLL[] = L"ntdll.dll";
	#else
		static const char ENV_USERPROFILE[] = "USERPROFILE";
		static const char ENV_APPDATA[] = "APPDATA";
		static const char ENV_PATH[] = "PATH";
		
		static const char NTDLL[] = "ntdll.dll";
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

#include "os.h"
#include "filesystem.h"
#include "pathsep.h"
#include "path.h"

int execute_shell_command(const char* const command) {
	/*
	Executes a shell command.
	
	Returns the exit code for the command, which is (0) on success.
	*/
	
	int code = 0;
	
	#if defined(_WIN32) && defined(_UNICODE)
		wchar_t* wcommand = NULL;
		
		const int wcommands = MultiByteToWideChar(CP_UTF8, 0, command, -1, NULL, 0);
		
		if (wcommands == 0) {
			return -1;
		}
		
		wcommand = malloc(((size_t) wcommands) * sizeof(*wcommand));
		
		if (wcommand == NULL) {
			return -1;
		}
		
		if (MultiByteToWideChar(CP_UTF8, 0, command, -1, wcommand, wcommands) == 0) {
			free(wcommand);
			return -1;
		}
		
		code = _wsystem(wcommand);
		
		free(wcommand);
	#else
		code = system(command);
	#endif
	
	#if !defined(_WIN32)
		code = WIFSIGNALED(code) ? 128 + WTERMSIG(code) : WEXITSTATUS(code);
	#endif
	
	return code;
	
}

#if !defined(__HAIKU__)
	int is_administrator(void) {
		/*
		Returns whether the caller's process is a member of the Administrators local
		group (on Windows) or a root (on POSIX), via "geteuid() == 0".
		
		Returns (1) on true, (0) on false, (-1) on error.
		*/
		
		#if defined(_WIN32)
			SID_IDENTIFIER_AUTHORITY authority = {SECURITY_NT_AUTHORITY};
			PSID group = {0};
			BOOL is_member = FALSE;
			BOOL status = FALSE;
			
			status = AllocateAndInitializeSid(
				&authority,
				2,
				SECURITY_BUILTIN_DOMAIN_RID,
				DOMAIN_ALIAS_RID_ADMINS,
				0,
				0,
				0,
				0,
				0,
				0,
				&group
			);
			
			if (!status) {
				return -1;
			}
			
			status = CheckTokenMembership(0, group, &is_member);
			
			FreeSid(group);
			
			if (!status) {
				return -1;
			}
			
			return (int) is_member;
		#else
			return geteuid() == 0;
		#endif
		
	}
#endif

#if defined(_WIN32)
	int is_wine(void) {
		/*
		Returns whether the caller's process is running under Wine.
		
		Returns (1) on true, (0) on false, (-1) on error.
		*/
		
		#if defined(_UNICODE)
			HMODULE module = GetModuleHandleW(WNTDLL);
		#else
			HMODULE module = GetModuleHandleA(NTDLL);
		#endif
		
		if (module == NULL) {
			return -1;
		}
		
		if (GetProcAddress(module, "wine_get_version") == NULL) {
			return 0;
		}
		
		return 1;
		
	}
#endif

char* get_configuration_directory(void) {
	/*
	Returns the config directory of the current user for applications.
	
	On non-Windows OSs, this proc conforms to the XDG Base Directory
	spec. Thus, this proc returns the value of the "XDG_CONFIG_HOME" environment
	variable if it is set, otherwise it returns the default configuration directory ("~/.config/").
	
	Returns NULL on error.
	*/
	
	char* configuration_directory = NULL;
	
	#if defined(_WIN32)
		#if defined(_UNICODE)
			int directorys = 0;
			
			const wchar_t* const wdirectory = _wgetenv(WENV_APPDATA);
			
			if (wdirectory == NULL) {
				return NULL;
			}
			
			directorys = WideCharToMultiByte(CP_UTF8, 0, wdirectory, -1, NULL, 0, NULL, NULL);
			
			if (directorys == 0) {
				return NULL;
			}
			
			configuration_directory = malloc((size_t) directorys);
			
			if (configuration_directory == NULL) {
				return NULL;
			}
			
			if (WideCharToMultiByte(CP_UTF8, 0, wdirectory, -1, configuration_directory, directorys, NULL, NULL) == 0) {
				free(configuration_directory);
				return NULL;
			}
		#else
			const char* const directory = getenv(ENV_APPDATA);
			
			if (directory == NULL) {
				return NULL;
			}
			
			configuration_directory = malloc(strlen(directory) + 1);
			
			if (configuration_directory == NULL) {
				return NULL;
			}
			
			strcpy(configuration_directory, directory);
		#endif
	#else
		const char* const directory = getenv(ENV_XDG_CONFIG_HOME);
		
		if (directory == NULL) {
			const char* const home = getenv(ENV_HOME);
			
			if (home == NULL) {
				return NULL;
			}
			
			configuration_directory = malloc(strlen(home) + strlen(PATHSEP_S) + strlen(DEFAULT_CONFIGURATION_DIRECTORY) + 1);
			
			if (configuration_directory == NULL) {
				return NULL;
			}
			
			strcpy(configuration_directory, home);
			strcat(configuration_directory, PATHSEP_S);
			strcat(configuration_directory, DEFAULT_CONFIGURATION_DIRECTORY);
		} else {
			configuration_directory = malloc(strlen(directory) + 1);
			
			if (configuration_directory == NULL) {
				return NULL;
			}
			
			strcpy(configuration_directory, directory);
		}
	#endif
	
	strip_separator(configuration_directory);
	
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
				return NULL;
			}
			
			directorys = GetTempPathW((DWORD) directorys, wdirectory);
			
			if (directorys == 0) {
				free(wdirectory);
			}
			
			directorys = (DWORD) WideCharToMultiByte(CP_UTF8, 0, wdirectory, -1, NULL, 0, NULL, NULL);
			
			if (directorys == 0) {
				free(wdirectory);
				return NULL;
			}
			
			temporary_directory = malloc((size_t) directorys);
			
			if (temporary_directory == NULL) {
				free(wdirectory);
				return NULL;
			}
			
			if (WideCharToMultiByte(CP_UTF8, 0, wdirectory, -1, temporary_directory, (int) directorys, NULL, NULL) == 0) {
				free(wdirectory);
				free(temporary_directory);
				return NULL;
			}
			
			free(wdirectory);
		#else
			directorys = GetTempPathA(0, NULL);
			
			if (directorys == 0) {
				return NULL;
			}
			
			directorys++;
			
			temporary_directory = malloc(directorys);
			
			if (temporary_directory == NULL) {
				return NULL;
			}
			
			directorys = GetTempPathA((DWORD) directorys, temporary_directory);
			
			if (directorys == 0) {
				free(temporary_directory);
				return NULL;
			}
		#endif
		
		strip_separator(temporary_directory);
	#else
		size_t index = 0;
		
		/*
		We should check first for the TEMP* and TMP* environment variables.
		*/
		for (index = 0; index < sizeof(ENV_TEMPORARY_DIRECTORY) / sizeof(*ENV_TEMPORARY_DIRECTORY); index++) {
			const char* const name = ENV_TEMPORARY_DIRECTORY[index];
			const char* const directory = getenv(name);
			
			if (directory == NULL || !directory_exists(directory)) {
				continue;
			}
			
			temporary_directory = malloc(strlen(directory) + 1);
			
			if (temporary_directory == NULL) {
				return NULL;
			}
			
			strcpy(temporary_directory, directory);
			
			break;
		}
		
		if (temporary_directory != NULL) {
			return temporary_directory;
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
			const char* const directory = TEMPORARY_DIRECTORIES[index];
			
			if (!directory_exists(directory)) {
				continue;
			}
			
			temporary_directory = malloc(strlen(directory) + 1);
			
			if (temporary_directory == NULL) {
				return NULL;
			}
			
			strcpy(temporary_directory, directory);
			
			break;
		}
	#endif
	
	return temporary_directory;
	
}

char* get_home_directory(void) {
	/*
	Returns the home directory of the current user.
	
	On Windows, it returns the value from "USERPROFILE" environment variable.
	On Posix based platforms, it returns the value from "HOME" environment variable.
	
	Returns NULL on error.
	*/
	
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
				return NULL;
			}
			
			home = malloc((size_t) directorys);
			
			if (home == NULL) {
				return NULL;
			}
			
			if (WideCharToMultiByte(CP_UTF8, 0, wdirectory, -1, home, directorys, NULL, NULL) == 0) {
				free(home);
				return NULL;
			}
		#else
			const char* const directory = getenv(ENV_USERPROFILE);
			
			if (directory == NULL) {
				return NULL;
			}
			
			home = malloc(strlen(directory) + 1);
			
			if (home == NULL) {
				return NULL;
			}
			
			strcpy(home, directory);
		#endif
	#else
		const char* const directory = getenv(ENV_HOME);
		
		if (directory == NULL) {
			return NULL;
		}
		
		home = malloc(strlen(directory) + 1);
		
		if (home == NULL) {
			return NULL;
		}
		
		strcpy(home, directory);
	#endif
	
	return home;
	
}

char* find_exe(const char* const name) {
	
	char* executable = NULL;
	char* path = NULL;
	char* component = NULL;
	
	size_t index = 0;
	size_t size = 0;
	size_t length = 0;
	
	#if defined(_WIN32)
		const char* const executable_extension = ".exe";
		const char* const separator = ";";
	#else
		const char* const separator = ":";
		const char* const executable_extension = "";
	#endif
	
	#if defined(_WIN32) && defined(_UNICODE)
		int paths = 0;
		
		const wchar_t* const wpath = _wgetenv(WENV_PATH);
		
		if (wpath == NULL) {
			return NULL;
		}
		
		paths = WideCharToMultiByte(CP_UTF8, 0, wpath, -1, NULL, 0, NULL, NULL);
		
		if (paths == 0) {
			return NULL;
		}
		
		path = malloc((size_t) paths);
		
		if (path == NULL) {
			return NULL;
		}
		
		if (WideCharToMultiByte(CP_UTF8, 0, wpath, -1, path, paths, NULL, NULL) == 0) {
			free(path);
			return NULL;
		}
	#else
		path = getenv(ENV_PATH);
		
		if (path == NULL) {
			return NULL;
		}
	#endif
	
	component = path;
	length = strlen(path) + 1;
	
	for (index = 0; index < length; index++) {
		char* const ch = &path[index];
		
		const unsigned char a = *ch;
		
		if (!(a == *separator || a == '\0')) {
			continue;
		}
		
		size = (size_t) (ch - component);
		executable = malloc(size + strlen(PATHSEP_S) + strlen(name) + strlen(executable_extension) + 1);
		
		if (executable == NULL) {
			#if defined(_WIN32) && defined(_UNICODE)
				free(path);
			#endif
			
			return NULL;
		}
		
		memcpy(executable, component, size);
		executable[size] = '\0';
		
		strcat(executable, PATHSEP_S);
		strcat(executable, name);
		strcat(executable, executable_extension);
		
		if (file_exists(executable)) {
			break;
		}
		
		free(executable);
		executable = NULL;
		
		component = ch + 1;
	}
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(path);
	#endif
	
	return executable;
	
}

ssize_t get_nproc(void) {
	
	ssize_t processors = 0;
	
	#if defined(_WIN32)
		SYSTEM_INFO info = {0};
		GetSystemInfo(&info);
		
		processors = (ssize_t) info.dwNumberOfProcessors;
	#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__) || defined(__OpenBSD__)
		#if defined(__APPLE__)
			const int code = HW_AVAILCPU;
		#else
			const int code = HW_NCPU;
		#endif
		
		const int call[] = {CTL_HW, code};
		
		size_t size = sizeof(processors);
		
		if (sysctl(call, sizeof(call) / sizeof(*call), &processors, &size, NULL, 0) == -1) {
			return -1;
		}
	#elif defined(__HAIKU__)
		system_info info = {0};
		const status_t status = get_system_info(&info);
		
		if (status != B_OK) {
			return -1;
		}
		
		processors = (ssize_t) info.cpu_count;
	#else
		const long value = sysconf(_SC_NPROCESSORS_ONLN);
		
		if (value == -1) {
			return -1;
		}
		
		processors = (ssize_t) value;
	#endif
	
	return processors;
	
}
