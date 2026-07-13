#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
	#include <windows.h>
	#include <io.h>
#endif

#if !defined(_WIN32)
	#include <unistd.h>
	#include <termios.h>
	#include <errno.h>
	#include <sys/ioctl.h>
	#include <fcntl.h>
#endif

#include "term/screen.h"

#define TERMSZ_HEIGHT 0x00
#define TERMSZ_WIDTH 0x01

#if defined(__ANDROID__) && __ANDROID_API__ < 26
	#define HAVE_CTERMID 0
#else
	#define HAVE_CTERMID 1
#endif

int erase_screen(void) {
	/*
	Erases the screen with the background colour and moves the cursor to home.
	
	Returns (0) on success, (-1) on error.
	*/
	
	#if defined(_WIN32)
		CONSOLE_SCREEN_BUFFER_INFO info = {0};
		DWORD value = 0;
		
		DWORD wrote = 0;
		COORD origin = {0};
		
		BOOL status = FALSE;
		
		const HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
		
		if (handle == INVALID_HANDLE_VALUE) {
			return -1;
		}
		
		if (GetConsoleScreenBufferInfo(handle, &info) == 0) {
			return -1;
		}
		
		value = (DWORD) (info.dwSize.X * info.dwSize.Y);
		
		#if defined(_UNICODE)
			status = FillConsoleOutputCharacterW(handle, L' ', value, origin, &wrote);
		#else
			status = FillConsoleOutputCharacterA(handle, ' ', value, origin, &wrote);
		#endif
		
		if (!status) {
			return -1;
		}
		
		if (FillConsoleOutputAttribute(handle, info.wAttributes, value, origin, &wrote) == 0) {
			return -1;
		}
		
		info.dwCursorPosition.X = 0;
		
		if (SetConsoleCursorPosition(handle, info.dwCursorPosition) == 0) {
			return -1;
		}
	#else
		const char seq[] = {0x1b, 0x5b, 0x32, 0x4a};
		const int fd = fileno(stdout);
		
		if (fd == -1) {
			return -1;
		}
		
		if (write(fd, seq, sizeof(seq)) == -1) {
			return -1;
		}
	#endif
	
	return 0;
	
}

int erase_line(void) {
	/*
	Erases the entire current line.
	
	Returns (0) on success, (-1) on error.
	*/
	
	#if defined(_WIN32)
		CONSOLE_SCREEN_BUFFER_INFO info = {0};
		
		COORD origin = {0};
		
		DWORD wrote = 0;
		DWORD value = 0;
		
		BOOL status = FALSE;
		
		const HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
		
		if (handle == INVALID_HANDLE_VALUE) {
			return -1;
		}
		
		if (GetConsoleScreenBufferInfo(handle, &info) == 0) {
			return -1;
		}
		
		origin = info.dwCursorPosition;
		origin.X = 0;
		
		if (SetConsoleCursorPosition(handle, origin) == 0) {
			return -1;
		}
		
		value = (DWORD) (info.dwSize.X - origin.X);
		
		#if defined(_UNICODE)
			status = FillConsoleOutputCharacterW(handle, L' ', value, origin, &wrote);
		#else
			status = FillConsoleOutputCharacterA(handle, ' ', value, origin, &wrote);
		#endif
		
		if (!status) {
			return -1;
		}
		
		if (FillConsoleOutputAttribute(handle, info.wAttributes, value, info.dwCursorPosition, &wrote) == 0) {
			return -1;
		}
	#else
		const char seq[] = {0x1b, 0x5b, 0x32, 0x4b, 0x1b, 0x5b, 0x31, 0x47};
		const int fd = fileno(stdout);
		
		if (fd == -1) {
			return -1;
		}
		
		if (write(fd, seq, sizeof(seq)) == -1) {
			return -1;
		}
	#endif
	
	return 0;
	
}


int hide_cursor(void) {
	/*
	Hides the cursor.
	
	Returns (0) on success, (-1) on error.
	*/
	
	#if defined(_WIN32)
		CONSOLE_CURSOR_INFO info = {0};
		
		const HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
		
		if (handle == INVALID_HANDLE_VALUE) {
			return -1;
		}
		
		if (!GetConsoleCursorInfo(handle, &info)) {
			return -1;
		}
		
		info.bVisible = FALSE;
		
		if (!SetConsoleCursorInfo(handle, &info)) {
			return -1;
		}
	#else
		const char seq[] = {0x1b, 0x5b, 0x3f, 0x32, 0x35, 0x6c};
		const int fd = fileno(stdin);
		
		if (fd == -1) {
			return -1;
		}
		
		if (write(fd, seq, sizeof(seq)) == -1) {
			return -1;
		}
	#endif
	
	return 0;
	
}

int show_cursor(void) {
	/*
	Shows the cursor.
	
	Returns (0) on success, (-1) on error.
	*/
	
	#if defined(_WIN32)
		CONSOLE_CURSOR_INFO info = {0};
		
		const HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
		
		if (handle == INVALID_HANDLE_VALUE) {
			return -1;
		}
		
		if (!GetConsoleCursorInfo(handle, &info)) {
			return -1;
		}
		
		info.bVisible = TRUE;
		
		if (!SetConsoleCursorInfo(handle, &info)) {
			return -1;
		}
	#else
		const char seq[] = {0x1b, 0x5b, 0x3f, 0x32, 0x35, 0x68};
		const int fd = fileno(stdin);
		
		if (fd == -1) {
			return -1;
		}
		
		if (write(fd, seq, sizeof(seq)) == -1) {
			return -1;
		}
	#endif
	
	return 0;
	
}

int is_atty(FILE* file) {
	
	int fd = 0;
	int status = 0;
	
	#if defined(_WIN32)
		fd = _fileno(file);
	#else
		fd = fileno(file);
	#endif
	
	if (fd == -1) {
		return -1;
	}
	
	#if defined(_WIN32)
		status = _isatty(fd);
	#else
		status = isatty(fd);
	#endif
	
	return status;
	
}

static size_t get_terminal_size(const int spec) {
	
	long int size = 0;
	size_t index = 0;
	
	#if defined(_WIN32)
		DWORD file = 0;
		HANDLE handle = 0;
		
		CONSOLE_SCREEN_BUFFER_INFO info = {0};
		
		DWORD fds[3] = {
			STD_INPUT_HANDLE,
			STD_OUTPUT_HANDLE,
			STD_ERROR_HANDLE
		};
		
		for (index = 0; index < sizeof(fds) / sizeof(*fds); index++) {
			file = fds[index];
			
			handle = GetStdHandle(file);
			
			if (handle == INVALID_HANDLE_VALUE) {
				continue;
			}
			
			if (GetConsoleScreenBufferInfo(handle, &info) == 0) {
				continue;
			}
			
			if (spec == TERMSZ_HEIGHT) {
				size = (info.srWindow.Bottom - info.srWindow.Top + 1);
			} else {
				size = (info.srWindow.Right - info.srWindow.Left + 1);
			}
			
			return (size_t) size;
		}
	#else
		int fd = 0;
		int status = 0;
		
		FILE* file = NULL;
		FILE* const fds[3] = {stdin, stdout, stderr};
		
		const char* const value = getenv((spec == TERMSZ_HEIGHT) ? "LINES" : "COLUMNS");
		
		struct winsize win = {0};
		
		char cterm[L_ctermid];
		
		if (value != NULL) {
			size = strtol(value, NULL, 10);
			
			if (errno != ERANGE && size > 0) {
				return (size_t) size;
			}
		}
		
		for (index = 0; index < sizeof(fds) / sizeof(*fds); index++) {
			file = fds[index];
			
			fd = fileno(file);
			
			if (fd == -1) {
				continue;
			}
			
			status = ioctl(fd, TIOCGWINSZ, &win);
			
			if (status != 0) {
				continue;
			}
			
			return ((spec == TERMSZ_HEIGHT) ? win.ws_row : win.ws_col);
		}
		
		#if HAVE_CTERMID
			ctermid(cterm);
		#else
			strcpy(cterm, "/dev/tty");
		#endif
		
		fd = open(cterm, O_RDONLY);
		
		if (fd == -1) {
			return 0;
		}
		
		status = ioctl(fd, TIOCGWINSZ, &win);
		
		close(fd);
		
		if (status == 0) {
			return ((spec == TERMSZ_HEIGHT) ? win.ws_row : win.ws_col);
		}
	#endif
	
	if (spec == TERMSZ_WIDTH) {
		return 80;
	}
	
	return 0;
	
}

size_t get_terminal_height(void) {
	
	return get_terminal_size(TERMSZ_HEIGHT);
	
}

size_t get_terminal_width(void) {
	
	return get_terminal_size(TERMSZ_WIDTH);
	
}
