#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
	#include <windows.h>
	#include <libloaderapi.h>
#endif

#if !defined(_WIN32)
	#include "fs/fstream.h"
	#include "query.h"
#endif

#include "osdetect.h"

#if defined(_WIN32)
	#if defined(_UNICODE)
		static const wchar_t WNTDLL[] = L"ntdll.dll";
	#else
		static const char NTDLL[] = "ntdll.dll";
	#endif
	
	static const char WINE[] = "Wine";
#endif

static const char* distribution = NULL;

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

const char* osdetect_getdistro(void) {
	
	char* name = NULL;
	
	#if !defined(_WIN32)
		int err = 0;
		
		char* ptr = NULL;
		char* value = NULL;
		
		hquery_t query = {0};
	#endif
	
	if (distribution != NULL) {
		return distribution;
	}
	
	#if defined(_WIN32)
		if (is_wine() != 1) {
			return NULL;
		}
		
		name = malloc(strlen(WINE) + 1);
		
		if (name == NULL) {
			return name;
		}
		
		strcpy(name, WINE);
	#else
		query_init(&query, '\n', NULL);
		err = query_load_file(&query, "/etc/os-release");
		
		if (err != 0) {
			err = query_load_file(&query, "/usr/lib/os-release");
		}
		
		if (err != 0) {
			goto end;
		}
		
		value = query_get_string(&query, "NAME");
		
		if (value == NULL || strlen(value) <= 2) {
			err = -1;
			goto end;
		}
		
		ptr = value;
		
		if (*ptr == '"') {
			value++;
		}
		
		ptr = strchr(value, '\0');
		ptr--;
		
		if (*ptr == '"') {
			*ptr = '\0';
		}
		
		name = malloc(strlen(value) + 1);
		
		if (name == NULL) {
			err = -1;
			goto end;
		}
		
		strcpy(name, value);
		
		end:;
		
		query_free(&query);
	#endif
	
	distribution = name;
	
	return distribution;
	
}

osdetect_t osdetect_getsystem(void) {
	
	#if defined(_WIN32)
		return OSDETECT_WINDOWS;
	#elif defined(__APPLE__)
		return OSDETECT_DARWIN;
	#elif defined(__HAIKU__)
		return OSDETECT_HAIKU;
	#elif defined(__ANDROID__)
		return OSDETECT_ANDROID;
	#elif defined(__OpenBSD__)
		return OSDETECT_OPENBSD;
	#elif defined(__MidnightBSD__)
		return OSDETECT_MIDNIGHTBSD;
	#elif defined(__FreeBSD__)
		return OSDETECT_FREEBSD;
	#elif defined(__NetBSD__)
		return OSDETECT_NETBSD;
	#elif defined(__DragonFly__)
		return OSDETECT_DRAGONFLY;
	#elif defined(__serenity__)
		return OSDETECT_SERENITY;
	#elif defined(__linux__)
		return OSDETECT_LINUX;
	#else
		return OSDETECT_UNKNOWN;
	#endif
	
}

const char* osdetect_stringify(const osdetect_t value) {
	
	switch (value) {
		case OSDETECT_WINDOWS:
			return "Windows";
		case OSDETECT_DARWIN:
			return "Darwin";
		case OSDETECT_HAIKU:
			return "Haiku";
		case OSDETECT_ANDROID:
			return "Android";
		case OSDETECT_OPENBSD:
			return "OpenBSD";
		case OSDETECT_MIDNIGHTBSD:
			return "MidnightBSD";
		case OSDETECT_FREEBSD:
			return "FreeBSD";
		case OSDETECT_NETBSD:
			return "NetBSD";
		case OSDETECT_DRAGONFLY:
			return "DragonFly BSD";
		case OSDETECT_SERENITY:
			return "SerenityOS";
		case OSDETECT_LINUX:
			return "Linux";
		default:
			return "Unknown";
	}
	
}

const char* osdetect_getplatform(void) {
	
	osdetect_t system = osdetect_getsystem();
	const char* distro = osdetect_getdistro();
	
	if (distro != NULL) {
		return distro;
	}
	
	return osdetect_stringify(system);
	
}
