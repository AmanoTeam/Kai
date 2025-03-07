#if !defined(M3U8PARSER_H)
#define M3U8PARSER_H

int m3u8parser_getuint(const char* const source, void** destination);
int m3u8parser_getfloat(const char* const source, void** destination);
int m3u8parser_getufloat(const char* const source, void** destination);
int m3u8parser_getbrange(const char* const source, void** destination);
int m3u8parser_getdtime(const char* const source, void** destination);
int m3u8parser_getestring(const char* const source, void** destination);
int m3u8parser_getustring(const char* const source, void** destination);
int m3u8parser_getqstring(const char* const source, void** destination);
int m3u8parser_gethexseq(const char* const source, void** destination);
int m3u8parser_getresolution(const char* const source, void** destination);

#endif
