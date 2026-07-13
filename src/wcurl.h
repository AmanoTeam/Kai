#if !defined(WCURL_H)
#define WCURL_H

#include <curl/curl.h>

#define WCURL_ERR_SUCCESS 0 /* Success */

#define WCURL_ERR_MEM_ALLOC_FAILURE -1 /* Could not allocate memory */

#define WCURLMLT_ERR_ADD_FAILURE -49 /* Could not add the cURL handler to cURL multi */
#define WCURLMLT_ERR_INIT_FAILURE -50 /* Could not initialize the cURL multi interface */
#define WCURLMLT_ERR_PERFORM_FAILURE -51 /* Could not perform on cURL multi */
#define WCURLMLT_ERR_POLL_FAILURE -52 /* Could not poll on cURL multi */
#define WCURLMLT_ERR_REMOVE_FAILURE -53 /* Could not remove the cURL handler from cURL multi */
#define WCURLMLT_ERR_SETOPT_FAILURE -54 /* Could not set options on cURL multi */

#define WCURLSHR_ERR_INIT_FAILURE -55 /* Could not initialize the cURL Share interface */
#define WCURLSHR_ERR_SETOPT_FAILURE -56 /* Could not set options on Share HTTP client */

#define WCURLU_ERR_INIT_FAILURE -57 /* Could not initialize the cURL URL interface */
#define WCURLU_ERR_URL_GET_FAILURE -58 /* Could not get URL from this cURL URL interface */
#define WCURLU_ERR_URL_SET_FAILURE -59 /* Could not set URL for this cURL URL interface */

#define WCURL_ERR_INIT_FAILURE -60 /* Could not initialize the HTTP client due to an unexpected error */
#define WCURL_ERR_REQUEST_FAILURE -61 /* HTTP request failure */
#define WCURL_ERR_SETOPT_FAILURE -62 /* Could not set options on HTTP client */
#define WCURL_ERR_SLIST_FAILURE -63 /* Could not append item to list */

#define WCURL_ERR_SSL_CERT_LOAD_FAILURE -63 /* Could not load SSL certificates */

#define WCURL_ERR_BUFFER_OVERFLOW -22 /* Writing data to that memory address would cause a buffer overflow */

struct wcurl_error {
	CURLcode code;
	char* msg;
};

struct wcurl {
	CURL* curl;
	struct wcurl_error error;
	size_t retry;
};

struct wcurl_multi {
	CURLM* curl_multi;
	CURLSH* curl_share;
};

typedef struct wcurl_error wcurl_error_t;
typedef struct wcurl wcurl_t;
typedef struct wcurl_multi wcurl_multi_t;

int wcurl_init(wcurl_t* const wcurl);
int wcurl_duplicate(wcurl_t* const source, wcurl_t* const destination);
void wcurl_free(wcurl_t* const wcurl);
void wcurl_global_free(void);

CURL* wcurl_getcurl(wcurl_t* const wcurl);
wcurl_t* wcurl_getglobal(void);

wcurl_error_t* wcurl_geterr(wcurl_t* const wcurl);
void wcurlerr_free(wcurl_error_t* const err);

int wcurl_perform(wcurl_t* const wcurl);

int wcurl_retryable(
	CURL* const curl,
	const CURLcode code
);

int wcurlmlt_init(
	wcurl_multi_t* const wcurl_multi,
	const size_t concurrency
);
void wcurlmlt_free(wcurl_multi_t* const wcurl_multi);
void wcurlmlt_global_free(void);

CURLM* wcurlmlt_getcurl(wcurl_multi_t* const wcurl_multi);
wcurl_multi_t* wcurlmlt_getglobal(const size_t concurrency);
CURLSH* wcurlmlt_getshr(wcurl_multi_t* const wcurl_multi);

#endif
