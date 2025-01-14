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

#include "cir.h"

#if !defined(_WIN32)
	static const int EXTRA_TCSETATTR_ACTIONS = (
		#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__) || defined(__OpenBSD__)
			TCSASOFT
		#else
			0
		#endif
	);
#endif

int cir_init(struct CIR* const reader) {
	/*
	Initializes the Console Input Reader struct.
	
	On Linux, this will set the standard input to raw mode.
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
		
		if (tcsetattr(fd, TCSAFLUSH | EXTRA_TCSETATTR_ACTIONS, &attributes) == -1) {
			return -1;
		}
		
		fflush(stdin);
	#endif
	
	reader->initialized = 1;
	
	return 0;
	
}

const struct CIKey* cir_get(struct CIR* const reader) {
	/*
	Listen for key presses on keyboard.
	*/
	
	size_t index = 0;
	
	#if defined(_WIN32)
		HANDLE handle = 0;
		
		INPUT_RECORD input_records[128];
		DWORD records_count = 0;
		
		BOOL status = FALSE;
	#endif
	
	#if !defined(_WIN32)
		int fd = 0;
	#endif
	
	memset(reader->tmp, '\0', sizeof(reader->tmp));
	
	#if defined(_WIN32)
		handle = GetStdHandle(STD_INPUT_HANDLE);
		
		if (handle == INVALID_HANDLE_VALUE) {
			return NULL;
		}
		
		#if defined(_UNICODE)
			status = ReadConsoleInputW(handle, input_records, sizeof(input_records) / sizeof(*input_records), &records_count);
		#else
			status = ReadConsoleInputA(handle, input_records, sizeof(input_records) / sizeof(*input_records), &records_count);
		#endif
		
		if (!status) {
			return NULL;
		}
		
		for (index = 0; index < (size_t) records_count; index++) {
			const INPUT_RECORD input_record = input_records[index];
			const KEY_EVENT_RECORD* const key_event = (KEY_EVENT_RECORD*) &input_record.Event;
			
			const size_t remaining = sizeof(reader->tmp) - index;
			
			if (remaining < 2) {
				return NULL;
			}
			
			if (!(input_record.EventType == KEY_EVENT && key_event->bKeyDown)) {
				break;
			}
			
			#if defined(_UNICODE)
				if (key_event->uChar.UnicodeChar == L'\0') {
					reader->tmp[index] = (char) key_event->wVirtualKeyCode;
				} else {
					wchar_t ws[2] = {L'\0', L'\0'};
					ws[0] = key_event->uChar.UnicodeChar;
					
					if (WideCharToMultiByte(CP_UTF8, 0, ws, -1, reader->tmp + index, remaining, NULL, NULL) == 0) {
						return NULL;
					}
				}
			#else
				reader->tmp[index] = key_event->uChar.AsciiChar == '\0' ? (char) key_event->wVirtualKeyCode : key_event->uChar.AsciiChar;
			#endif
		}
		
		if (reader->tmp[0] == '\0') {
			return &KEYBOARD_KEY_EMPTY;
		}
	#else
		fd = fileno(stdin);
		
		if (fd == -1) {
			return NULL;
		}
		
		if (read(fd, reader->tmp, sizeof(reader->tmp)) == -1) {
			return NULL;
		}
	#endif
	
	for (index = 0; index < (sizeof(keys) / sizeof(*keys)); index++) {
		const struct CIKey* const key = &keys[index];
		
		#if defined(_WIN32)
			if (*key->code == (DWORD) *reader->tmp) {
				return key;
			}
		#else
			if (memcmp(key->code, reader->tmp, strlen(reader->tmp)) == 0) {
				return key;
			}
		#endif
	}
	
	return &KEYBOARD_KEY_UNKNOWN;
	
}

int cir_free(struct CIR* const reader) {
	/*
	Free the Console Input Reader struct.
	
	On Linux, this will reset the standard input attributes back to "sane mode".
	Under Windows, this will add back the ENABLE_PROCESSED_INPUT bit to the standard input flags.
	
	Returns (0) on success, (-1) on error.
	*/
	
	#if defined(_WIN32)
		const HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
		
		if (handle == INVALID_HANDLE_VALUE) {
			return -1;
		}
		
		if (!reader->initialized) {
			return 0;
		}
		
		if (SetConsoleMode(handle, reader->attributes) == 0) {
			return -1;
		}
	#else
		const int fd = fileno(stdin);
		
		if (fd == -1) {
			return -1;
		}
		
		if (!reader->initialized) {
			return 0;
		}
		
		if (tcsetattr(fd, TCSAFLUSH | EXTRA_TCSETATTR_ACTIONS, &reader->attributes) == -1) {
			return -1;
		}
	#endif
	
	memset(reader->tmp, '\0', sizeof(reader->tmp));
	
	return 0;
	
}
