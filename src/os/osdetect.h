#if !defined(OS_OSDETECT_H)
#define OS_OSDETECT_H

enum osdetect_t {
	OSDETECT_UNKNOWN,
	OSDETECT_WINDOWS,
	OSDETECT_DARWIN,
	OSDETECT_HAIKU,
	OSDETECT_ANDROID,
	OSDETECT_OPENBSD,
	OSDETECT_MIDNIGHTBSD,
	OSDETECT_FREEBSD,
	OSDETECT_NETBSD,
	OSDETECT_DRAGONFLY,
	OSDETECT_SERENITY,
	OSDETECT_LINUX
};

typedef enum osdetect_t osdetect_t;

osdetect_t osdetect_getsystem(void);
const char* osdetect_getdistro(void);
const char* osdetect_getplatform(void);

const char* osdetect_stringify(const osdetect_t value);

#endif
