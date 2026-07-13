#include <stdlib.h>

#include <string.h>

#include "guess_file_format.h"
#include "fs/fstream.h"

static const char XZ_MAGIC[] = {
	0xfd, 0x37, 0x7a, 0x58
};

static const char BZIP2_MAGIC[] = {
	0x42, 0x5a, 0x68
};

static const char GZIP_MAGIC[] = {
	0x1f, 0x8b
};

static const char ZSTD_MAGIC[] = {
	0x28, 0xb5, 0x2f, 0xfd
};

int format_guess_string(const char* const chunk) {
	
	if (memcmp(chunk, XZ_MAGIC, sizeof(XZ_MAGIC)) == 0) {
		return GUESS_FILE_FORMAT_XZ;
	}
	
	if (memcmp(chunk, BZIP2_MAGIC, sizeof(BZIP2_MAGIC)) == 0) {
		return GUESS_FILE_FORMAT_BZIP2;
	}
	
	if (memcmp(chunk, GZIP_MAGIC, sizeof(GZIP_MAGIC)) == 0) {
		return GUESS_FILE_FORMAT_GZIP;
	}
	
	if (memcmp(chunk, ZSTD_MAGIC, sizeof(ZSTD_MAGIC)) == 0) {
		return GUESS_FILE_FORMAT_ZSTD;
	}
	
	return GUESS_FILE_FORMAT_SOMETHING_ELSE;
	
}

int format_guess_file(const char* const filename) {
	
	int err = GUESS_FILE_FORMAT_SOMETHING_ELSE;
	
	ssize_t rsize = 0;
	
	char chunk[9];
	fstream_t* stream = NULL;
	
	stream = fstream_open(filename, FSTREAM_READ);
	
	if (stream == NULL) {
		goto end;
	}
	
	rsize = fstream_read(stream, chunk, sizeof(chunk));
	
	if (rsize == -1) {
		goto end;
	}
	
	err = format_guess_string(chunk);
	
	end:;
	
	fstream_close(stream);
	
	return err;
	
}
