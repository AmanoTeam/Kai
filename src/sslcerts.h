#if !defined(SSLCERTS_H)
#define SSLCERTS_H

#include <curl/curl.h>

#define SSLCERTS_SUCCESS 0
#define SSLCERTS_ERROR -1

int sslcerts_load_certificates(CURL* const curl);
void sslcerts_unload_certificates(void);

#endif
