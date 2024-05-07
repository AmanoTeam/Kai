#if !defined(WALKDIR_H)
#define WALKDIR_H

#include <stdlib.h>

#if defined(_WIN32)
	#include <windows.h>
#endif

#if !defined(_WIN32)
	#include <dirent.h>
#endif

enum WalkDirType {
	WALKDIR_ITEM_DIRECTORY,
	WALKDIR_ITEM_FILE,
	WALKDIR_ITEM_UNKNOWN
};

struct WalkDirItem {
	enum WalkDirType type;
	size_t index;
	char* name;
};

struct WalkDir {
	struct WalkDirItem item;
#if defined(_WIN32)
	HANDLE handle;
	WIN32_FIND_DATA data;
#else
	DIR* dir;
#endif
#if defined(__HAIKU__)
	char* directory;
#endif
};

int walkdir_init(struct WalkDir* const walkdir, const char* const directory);
const struct WalkDirItem* walkdir_next(struct WalkDir* const walkdir);
void walkdir_free(struct WalkDir* const walkdir);

#endif
