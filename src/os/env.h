#if !defined(OS_ENV_H)
#define OS_ENV_H

char* get_configuration_directory(void);
char* get_temporary_directory(void);
char* get_home_directory(void);
char* find_exe(const char* const name);

#endif
