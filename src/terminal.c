#include <stdio.h>

#if defined(_WIN32)
	#include <windows.h>
	#include <io.h>
#endif

#if !defined(_WIN32)
	#include <stdio.h>
	#include <unistd.h>
	#include <termios.h>
#endif

#include "terminal.h"

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
