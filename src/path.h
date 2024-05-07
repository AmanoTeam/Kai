#if !defined(PATH_H)
#define PATH_H

const char* strip_separator(char* const s);
int isabsolute(const char* const path);
int isrelative(const char* const path);
char* dirname(const char* const path);
char* basename(const char* const path);
char* get_file_extension(const char* const filename);
char* remove_file_extension(char* const filename);
char* normalize_filename(char* const filename);
char* normalize_directory(char* directory);
size_t get_parent_directory(const char* const source, char* const destination, const size_t depth);

#endif
