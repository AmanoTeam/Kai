#if !defined(SSLCERTS_H)
#define SSLCERTS_H

#include <curl/curl.h>

int sslcerts_load_certificates(CURL* const curl);
void sslcerts_unload_certificates(void);

#endif
