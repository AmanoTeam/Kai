#if !defined(PATHSEP_H)
#define PATHSEP_H

#if defined(_WIN32)
	static const char PATHSEP[] = "\\";
#else
	static const char PATHSEP[] = "/";
#endif

#if defined(_WIN32) && defined(_UNICODE)
	static const wchar_t WIN10_LONG_PATH_PREFIX[] = L"\\\\?\\";
#endif

#endif
