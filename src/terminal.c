#if defined(_WIN32)
	#include <windows.h>
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
		const int fd = fileno(stdout);
		
		if (fd == -1) {
			return -1;
		}
		
		if (write(fd, "\033[2J", 4) == -1) {
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
		const int fd = fileno(stdout);
		
		if (fd == -1) {
			return -1;
		}
		
		if (write(fd, "\033[2K", 4) == -1) {
			return -1;
		}
		
		if (write(fd, "\033[1G", 4) == -1) {
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
		const int fd = fileno(stdin);
		
		if (fd == -1) {
			return -1;
		}
		
		if (write(fd, "\e[?25l", 7) == -1) {
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
		const int fd = fileno(stdin);
		
		if (fd == -1) {
			return -1;
		}
		
		if (write(fd, "\e[?25h", 7) == -1) {
			return -1;
		}
	#endif
	
	return 0;
	
}
