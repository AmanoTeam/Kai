#if !defined(M3U8UTILS_H)
#define M3U8UTILS_H

#include "m3u8types.h"
#include "biggestint.h"

int namesafe(const char* const s);
int isuint(const char* const s);
int isfloat(const char* const s);
int isufloat(const char* const s);
int ishex(const char* const s);
int isqstring(const char* const s);
int isestring(const char* const s);
int isresolution(const char* const s);
int isbrange(const char* const s);
int isdtime(const char* const s);
int isleap(const int year);

char* btos(const bigfloat_t b, char* const s);

int m3u8uri_resolve_url(
	const char* const a,
	const char* const b,
	char** destination
);

int m3u8uri_resolve_path(
	const char* const a,
	const char* const b,
	char** destination
);

int m3u8uri_resolve_file(const char* const a, const char* const b, char** destination);
int m3u8uri_resolve_directory(const char* const a, const char* const b, char** destination);

int m3u8uri_resolve(
	const struct M3U8BaseURI* const base,
	const char* const source,
	char** destination
);

#define BTOS_MAX_SIZE 128

#endif
