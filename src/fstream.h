#if !defined(FSTREAM_H)
#define FSTREAM_H

#include <stdlib.h>

#if defined(_WIN32)
	#include <windows.h>
#endif

#if !defined(_WIN32)
	#include <stdio.h>
	#include <sys/types.h>
#endif

#define FSTREAM_ERROR (-1)
#define FSTREAM_EOF (0)

enum FStreamMode {
	FSTREAM_WRITE,
	FSTREAM_READ,
	FSTREAM_APPEND
};

enum FStreamSeek {
	FSTREAM_SEEK_BEGIN,
	FSTREAM_SEEK_CURRENT,
	FSTREAM_SEEK_END
};

struct FStream {
	enum FStreamMode mode;
#if defined(_WIN32)
	HANDLE stream;
#else
	FILE* stream;
#endif
};

struct FStream* fstream_open(const char* const filename, const enum FStreamMode mode);
int fstream_lock(struct FStream* const stream);
ssize_t fstream_read(struct FStream* const stream, char* const buffer, const size_t size);
int fstream_write(struct FStream* const stream, const char* const buffer, const size_t size);
int fstream_seek(struct FStream* const stream, const long int offset, const enum FStreamSeek method);
long int fstream_tell(struct FStream* const stream);
int fstream_close(struct FStream* const stream);

#endif
