#if !defined(GUESS_URI_TYPE_H)
#define GUESS_URI_TYPE_H

#define GUESS_URI_TYPE_URL 0x01
#define GUESS_URI_TYPE_LOCAL_FILE 0x02
#define GUESS_URI_TYPE_LOCAL_DIRECTORY 0x03
#define GUESS_URI_TYPE_SOMETHING_ELSE 0x04

int uri_guess_type(const char* const something);

#endif
