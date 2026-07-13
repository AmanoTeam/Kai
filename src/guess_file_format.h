#if !defined(GUESS_FILE_FORMAT_H)
#define GUESS_FILE_FORMAT_H

#define GUESS_FILE_FORMAT_XZ 0x01
#define GUESS_FILE_FORMAT_BZIP2 0x02
#define GUESS_FILE_FORMAT_GZIP 0x03
#define GUESS_FILE_FORMAT_ZSTD 0x04
#define GUESS_FILE_FORMAT_SOMETHING_ELSE 0x05

int format_guess_string(const char* const chunk);
int format_guess_file(const char* const filename);

#endif
