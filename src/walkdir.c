#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
	#include <windows.h>
#endif

#if !defined(_WIN32)
	#include <dirent.h>
#endif

#if defined(__HAIKU__)
	#include <sys/stat.h>
#endif

#include "walkdir.h"

#if defined(_WIN32)
	#include "filesystem.h"
	#include "path.h"
#endif

#if defined(_WIN32) || defined(__HAIKU__)
	#include "pathsep.h"
#endif

#if defined(_WIN32)
	static const char ASTERISK[] = "*";
#endif

int walkdir_init(struct WalkDir* const walkdir, const char* const directory) {
	
	#if defined(_WIN32)
		#if defined(_UNICODE)
			int wpatterns = 0;
			int wpathseps = 0;
			int wasterisks = 0;
			
			wchar_t* wpathsep = NULL;
			wchar_t* wasterisk = NULL;
			wchar_t* wpattern = NULL;
			
			/* This prefix is required to support long paths in Windows 10+ */
			const size_t prefixs = isabsolute(directory) ? wcslen(WIN10_LONG_PATH_PREFIX) : 0;
			
			wpatterns = MultiByteToWideChar(CP_UTF8, 0, directory, -1, NULL, 0);
			
			if (wpatterns == 0) {
				return -1;
			}
			
			wpathseps = MultiByteToWideChar(CP_UTF8, 0, PATHSEP_S, -1, NULL, 0);
			
			if (wpathseps == 0) {
				return -1;
			}
			
			wpathsep = malloc(((size_t) wpathseps) * sizeof(*wpathsep));
			
			if (wpathsep == NULL) {
				return -1;
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, PATHSEP_S, -1, wpathsep, wpathseps) == 0) {
				free(wpathsep);
				return -1;
			}
			
			wasterisks = MultiByteToWideChar(CP_UTF8, 0, ASTERISK, -1, NULL, 0);
			
			if (wasterisks == 0) {
				return -1;
			}
			
			wasterisk = malloc(((size_t) wasterisks) * sizeof(*wasterisk));
			
			if (wasterisk == NULL) {
				free(wpathsep);
				return -1;
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, ASTERISK, -1, wasterisk, wasterisks) == 0) {
				free(wpathsep);
				free(wasterisk);
				return -1;
			}
			
			wpattern = malloc((prefixs + (size_t) wpatterns + wcslen(wpathsep) + wcslen(wasterisk)) * sizeof(*wpattern));
			
			if (wpattern == NULL) {
				free(wpathsep);
				free(wasterisk);
				return -1;
			}
			
			if (prefixs > 0) {
				wcscpy(wpattern, WIN10_LONG_PATH_PREFIX);
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, directory, -1, wpattern + prefixs, wpatterns) == 0) {
				free(wpathsep);
				free(wasterisk);
				free(wpattern);
				return -1;
			}
			
			wcscat(wpattern, wpathsep);
			wcscat(wpattern, wasterisk);
			
			free(wpathsep);
			free(wasterisk);
			
			walkdir->handle = FindFirstFileW(wpattern, &walkdir->data);
			
			free(wpattern);
		#else
			char* pattern = malloc(strlen(directory) + strlen(PATHSEP_S) + strlen(ASTERISK) + 1);
			
			if (pattern == NULL) {
				return -1;
			}
			
			strcpy(pattern, directory);
			strcat(pattern, PATHSEP_S);
			strcat(pattern, ASTERISK);
			
			walkdir->handle = FindFirstFileA(pattern, &walkdir->data);
			
			free(pattern);
		#endif
		
		if (walkdir->handle == INVALID_HANDLE_VALUE) {
			return -1;
		}
	#else
		walkdir->dir = opendir(directory);
		
		if (walkdir->dir == NULL) {
			return -1;
		}
		
		#if defined(__HAIKU__)
			walkdir->directory = malloc(strlen(directory) + 1);
			
			if (walkdir->directory == NULL) {
				walkdir_free(walkdir);
				return -1;
			}
			
			strcpy(walkdir->directory, directory);
		#endif
	#endif
	
	return 0;
	
}

const struct WalkDirItem* walkdir_next(struct WalkDir* const walkdir) {
	
	#if defined(_WIN32) && defined(_UNICODE)
		int names = 0;
	#endif
	
	#if !defined(_WIN32)
		const struct dirent* item = NULL;
	#endif
	
	free(walkdir->item.name);
	walkdir->item.name = NULL;
	
	#if defined(_WIN32)
		#if defined(_UNICODE)
			if (walkdir->item.index > 0) {
				if (FindNextFileW(walkdir->handle, &walkdir->data) == 0) {
					return NULL;
				}
			}
			
			names = WideCharToMultiByte(CP_UTF8, 0, walkdir->data.cFileName, -1, NULL, 0, NULL, NULL);
			
			if (names == 0) {
				return NULL;
			}
			
			walkdir->item.name = malloc((size_t) names);
			
			if (walkdir->item.name == NULL) {
				return NULL;
			}
			
			if (WideCharToMultiByte(CP_UTF8, 0, walkdir->data.cFileName, -1, walkdir->item.name, names, NULL, NULL) == 0) {
				free(walkdir->item.name);
				return NULL;
			}
		#else
			if (walkdir->item.index > 0) {
				if (FindNextFileA(walkdir->handle, &walkdir->data) == 0) {
					return NULL;
				}
			}
			
			walkdir->item.name = malloc(strlen(walkdir->data.cFileName) + 1);
			
			if (walkdir->item.name == NULL) {
				return NULL;
			}
			
			strcpy(walkdir->item.name, walkdir->data.cFileName);
		#endif
		
		walkdir->item.type = (walkdir->data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) > 0 ? WALKDIR_ITEM_DIRECTORY : WALKDIR_ITEM_FILE;
	#else
		item = readdir(walkdir->dir);
		
		if (item == NULL) {
			return NULL;
		}
		
		walkdir->item.name = malloc(strlen(item->d_name) + 1);
		
		if (walkdir->item.name == NULL) {
			return NULL;
		}
		
		strcpy(walkdir->item.name, item->d_name);
		
		#if defined(__HAIKU__)
			char* path = malloc(strlen(walkdir->directory) + strlen(PATHSEP_S) + strlen(item->d_name) + 1);
			
			if (path == NULL) {
				return NULL;
			}
			
			strcpy(path, walkdir->directory);
			strcat(path, PATHSEP_S);
			strcat(path, item->d_name);
			
			struct stat st = {0};
			
			if (stat(path, &st) == -1) {
				free(path);
				return NULL;
			}
			
			free(path);
		#endif
		
		#if defined(__HAIKU__)
			switch (st.st_mode & S_IFMT) {
				case S_IFDIR:
				case S_IFBLK:
					walkdir->item.type = WALKDIR_ITEM_DIRECTORY;
					break;
				case S_IFLNK:
				case S_IFIFO:
				case S_IFREG:
				case S_IFSOCK:
				case S_IFCHR:
					walkdir->item.type = WALKDIR_ITEM_FILE;
					break;
				default:
					walkdir->item.type = WALKDIR_ITEM_UNKNOWN;
					break;
			}
		#else
			switch (item->d_type) {
				case DT_DIR:
				case DT_BLK:
					walkdir->item.type = WALKDIR_ITEM_DIRECTORY;
					break;
				case DT_LNK:
				case DT_FIFO:
				case DT_REG:
				case DT_SOCK:
				case DT_CHR:
					walkdir->item.type = WALKDIR_ITEM_FILE;
					break;
				case DT_UNKNOWN:
					walkdir->item.type = WALKDIR_ITEM_UNKNOWN;
					break;
			}
		#endif
	#endif
	
	walkdir->item.index++;
	
	return &walkdir->item;
	
}

void walkdir_free(struct WalkDir* const walkdir) {
	
	#if defined(_WIN32)
		FindClose(walkdir->handle);
	#else
		closedir(walkdir->dir);
	#endif
	
	free(walkdir->item.name);
	walkdir->item.name = NULL;
	
	#if defined(__HAIKU__)
		free(walkdir->directory);
		walkdir->directory = NULL;
	#endif
	
}
