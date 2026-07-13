#if !defined(FS_CHMOD_H)
#define FS_CHMOD_H

#define CHMOD_USER_EXEC (0x00000001)
#define CHMOD_USER_WRITE (0x00000002)
#define CHMOD_USER_READ (0x00000004)
#define CHMOD_GROUP_EXEC (0x00000010)
#define CHMOD_GROUP_WRITE (0x00000020)
#define CHMOD_GROUP_READ (0x00000040)
#define CHMOD_OTHERS_EXEC (0x00000080)
#define CHMOD_OTHERS_WRITE (0x00000100)
#define CHMOD_OTHERS_READ (0x00000200)

int chmod_getmode(const char* const path);
int chmod_setmode(const char* const path, const int value);

#endif
