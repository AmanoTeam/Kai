#include <stdlib.h>

#if defined(_WIN32)
	#include <windows.h>
#endif

#if !defined(_WIN32)
	#include <sys/wait.h>
#endif

#include "os/shell.h"

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
			code = -1;
			goto end;
		}
		
		wcommand = malloc(((size_t) wcommands) * sizeof(*wcommand));
		
		if (wcommand == NULL) {
			code = -1;
			goto end;
		}
		
		if (MultiByteToWideChar(CP_UTF8, 0, command, -1, wcommand, wcommands) == 0) {
			code = -1;
			goto end;
		}
		
		code = _wsystem(wcommand);
	#else
		code = system(command);
	#endif
	
	#if !defined(_WIN32)
		code = WIFSIGNALED(code) ? 128 + WTERMSIG(code) : WEXITSTATUS(code);
	#endif
	
	#if defined(_WIN32) && defined(_UNICODE)
		end:;
	#endif
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(wcommand);
	#endif
	
	return code;
	
}