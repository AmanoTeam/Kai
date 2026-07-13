#if !defined(FS_STAT_H)
#define FS_STAT_H

#define FILETYPE_UNKNOWN (-1)
#define FILETYPE_SYMLINK 0
#define FILETYPE_REGULAR 1
#define FILETYPE_DIRECTORY 2
#define FILETYPE_CHARDEV 3
#define FILETYPE_BLOCKDEV  4
#define FILETYPE_FIFO  5
#define FILETYPE_SOCKET  6

int get_file_type(const char* const filename);

#endif
