#if !defined(OS_H)
#define OS_H

#if !defined(_WIN32)
	#include <sys/types.h>
#endif

#if !defined(__HAIKU__)
	int is_administrator(void);
#endif

#if defined(_WIN32)
	int is_wine(void);
#endif

int execute_shell_command(const char* const command);
char* get_configuration_directory(void);
char* get_temporary_directory(void);
char* get_home_directory(void);
char* find_exe(const char* const name);
ssize_t get_nproc(void);

#endif
