#if !defined(FS_SYMLINK_H)
#define FS_SYMLINK_H

int mklink(const char* const source, const char* const destination);
char* get_symlink(const char* const path);

#endif
