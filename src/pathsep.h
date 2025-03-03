#if !defined(PATHSEP_H)
#define PATHSEP_H

#if defined(_WIN32)
	static const unsigned char PATHSEP = '\\';
#else
	static const unsigned char PATHSEP = '/';
#endif

static const char PATHSEP_S[] = {PATHSEP, '\0'};

#if defined(_WIN32) && defined(_UNICODE)
	static const wchar_t WIN10_LONG_PATH_PREFIX[] = L"\\\\?\\";
#endif

#endif
