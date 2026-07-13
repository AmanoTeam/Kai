#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
	#include <windows.h>
	#include <conio.h>
#endif

#if !defined(_WIN32)
	#include <termios.h>
	#include <unistd.h>
#endif

#include "term/keyboard.h"

int cir_init(cir_t* const reader) {
	/*
	Initializes the Console Input Reader struct.
	
	On Unix, this will set the standard input to raw mode.
	Under Windows, this will remove the ENABLE_PROCESSED_INPUT bit from the standard input flags.
	
	Returns (0) on success, (-1) on error.
	*/
	
	#if defined(_WIN32)
		DWORD attributes = 0;
		
		const HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
		
		if (handle == INVALID_HANDLE_VALUE) {
			return -1;
		}
		
		if (GetConsoleMode(handle, &attributes) == 0) {
			return -1;
		}
		
		reader->attributes = attributes;
		
		attributes &= (DWORD) ~ENABLE_PROCESSED_INPUT;
		
		if (SetConsoleMode(handle, attributes) == 0) {
			return -1;
		}
	#else
		struct termios attributes = {0};
		
		int flags = TCSAFLUSH;
		const int fd = fileno(stdin);
		
		if (fd == -1) {
			return -1;
		}
		
		if (tcgetattr(fd, &attributes) == -1) {
			return -1;
		}
		
		reader->attributes = attributes;
		
		cfmakeraw(&attributes);
		
		attributes.c_cc[VTIME] = 0;
		attributes.c_cc[VMIN] = 1;
		
		#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__) || defined(__OpenBSD__)
			flags |= TCSASOFT;
		#endif
		
		if (tcsetattr(fd, flags, &attributes) == -1) {
			return -1;
		}
		
		fflush(stdin);
	#endif
	
	reader->status = 1;
	
	return 0;
	
}

const cir_key_t* cir_get(cir_t* const reader) {
	/*
	Listen for key presses on keyboard.
	*/
	
	size_t index = 0;
	
	#if defined(_WIN32)
		HANDLE handle = 0;
		
		INPUT_RECORD inputs[128];
		DWORD records_count = 0;
		
		BOOL status = FALSE;
		
		const INPUT_RECORD* input = NULL;
		const KEY_EVENT_RECORD* event = NULL;
		
		size_t left = 0;
	#endif
	
	#if defined(_WIN32) && defined(_UNICODE)
		wchar_t ws[2] = {L'\0', L'\0'};
	#endif
	
	#if !defined(_WIN32)
		int fd = 0;
	#endif
	
	const cir_key_t* key = NULL;
	
	memset(reader->buffer, '\0', sizeof(reader->buffer));
	
	#if defined(_WIN32)
		handle = GetStdHandle(STD_INPUT_HANDLE);
		
		if (handle == INVALID_HANDLE_VALUE) {
			return NULL;
		}
		
		#if defined(_UNICODE)
			status = ReadConsoleInputW(handle, inputs, sizeof(inputs) / sizeof(*inputs), &records_count);
		#else
			status = ReadConsoleInputA(handle, inputs, sizeof(inputs) / sizeof(*inputs), &records_count);
		#endif
		
		if (!status) {
			return NULL;
		}
		
		for (index = 0; index < (size_t) records_count; index++) {
			input = &inputs[index];
			event = (KEY_EVENT_RECORD*) &input->Event;
			
			left = sizeof(reader->buffer) - index;
			
			if (left < 2) {
				return NULL;
			}
			
			if (!(input->EventType == KEY_EVENT && event->bKeyDown)) {
				break;
			}
			
			#if defined(_UNICODE)
				if (event->uChar.UnicodeChar == L'\0') {
					reader->buffer[index] = (char) event->wVirtualKeyCode;
				} else {
					ws[0] = event->uChar.UnicodeChar;
					ws[1] = L'\0';
					
					if (WideCharToMultiByte(CP_UTF8, 0, ws, -1, reader->buffer + index, left, NULL, NULL) == 0) {
						return NULL;
					}
				}
			#else
				reader->buffer[index] = (
					(event->uChar.AsciiChar == '\0') ?
					(char) event->wVirtualKeyCode :
					event->uChar.AsciiChar
				);
			#endif
		}
		
		if (reader->buffer[0] == '\0') {
			return &KEYBOARD_KEY_EMPTY;
		}
	#else
		fd = fileno(stdin);
		
		if (fd == -1) {
			return NULL;
		}
		
		if (read(fd, reader->buffer, sizeof(reader->buffer)) == -1) {
			return NULL;
		}
	#endif
	
	for (index = 0; index < (sizeof(keys) / sizeof(*keys)); index++) {
		key = &keys[index];
		
		#if defined(_WIN32)
			if (*key->code == (DWORD) *reader->buffer) {
				return key;
			}
		#else
			if (memcmp(key->code, reader->buffer, strlen(reader->buffer)) == 0) {
				return key;
			}
		#endif
	}
	
	return &KEYBOARD_KEY_UNKNOWN;
	
}

int cir_free(cir_t* const reader) {
	/*
	Free the Console Input Reader struct.
	
	On Unix, this will reset the standard input attributes back to "sane mode".
	Under Windows, this will add back the ENABLE_PROCESSED_INPUT bit to the standard input flags.
	
	Returns (0) on success, (-1) on error.
	*/
	
	#if defined(_WIN32)
		const HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
		
		if (handle == INVALID_HANDLE_VALUE) {
			return -1;
		}
		
		if (!reader->status) {
			return 0;
		}
		
		if (SetConsoleMode(handle, reader->attributes) == 0) {
			return -1;
		}
	#else
		int flags = TCSAFLUSH;
		const int fd = fileno(stdin);
		
		if (fd == -1) {
			return -1;
		}
		
		if (!reader->status) {
			return 0;
		}
		
		#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__) || defined(__OpenBSD__)
			flags |= TCSASOFT;
		#endif
		
		if (tcsetattr(fd, flags, &reader->attributes) == -1) {
			return -1;
		}
	#endif
	
	memset(reader->buffer, '\0', sizeof(reader->buffer));
	
	return 0;
	
}
