#include <stdlib.h>

#if defined(__linux__)
	#include <features.h>
	
	#if !defined(__USE_GNU) && !defined(__BIONIC__)
		#define LOOKS_LIKE_MUSL 1
	#endif
#endif

#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__) || \
	defined(__OpenBSD__) || defined(__HAIKU__) || defined(__APPLE__) || \
	(defined(__linux__) && (defined(__GLIBC__) || defined(LOOKS_LIKE_MUSL)))
	#include <dirent.h>
	
	#if defined(__GLIBC__) && defined(_LARGEFILE64_SOURCE)
		typedef struct dirent64 directory_entry_t;
	#else
		typedef struct dirent directory_entry_t;
	#endif
	
	#define HAVE_GETDIRENTRIES 1
#endif

#if defined(__linux__) && !(defined(__GLIBC__) || defined(LOOKS_LIKE_MUSL))
	#include <sys/syscall.h>
	#include <sys/types.h>
	
	#if defined(SYS_getdents64)
		struct linux_dirent64 {
			ino64_t d_ino;
			off64_t d_off;
			unsigned short d_reclen;
			unsigned char  d_type;
			char d_name[];
		};
		
		typedef struct linux_dirent64 directory_entry_t;
	#else
		struct linux_dirent {
			unsigned long d_ino;
			off_t d_off;
			unsigned short d_reclen;
			char d_name[];
		};
		
		typedef struct linux_dirent directory_entry_t;
	#endif
	
	#define HAVE_GETDIRENTRIES 1
#endif

#if !defined(HAVE_GETDIRENTRIES)
	#error "This platform lacks a functioning getdents/getdirentries implementation"
#endif

int open_dir(const char* const directory);
ssize_t get_directory_entries(int fd, char* const buffer, const size_t buffer_size);
size_t directory_entry_size(const directory_entry_t* const entry);
int close_dir(int fd);

#pragma once
