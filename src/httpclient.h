#if !defined(HTTPCLIENT_H)
#define HTTPCLIENT_H

#include <curl/curl.h>

struct HTTPClientError {
	CURLcode code;
	char* message;
};

struct HTTPClient {
	CURL* curl;
	struct HTTPClientError error;
	size_t retry;
};

struct MultiHTTPClient {
	CURLM* curl_multi;
	CURLSH* curl_share;
};

int httpclient_init(struct HTTPClient* const client);

void httpclient_error_free(struct HTTPClientError* const error);
void httpclient_free(struct HTTPClient* const client);

struct HTTPClientError* httpclient_geterror(struct HTTPClient* const client);
CURL* httpclient_getclient(struct HTTPClient* const client);

int httpclient_retryable(CURL* const curl, const CURLcode code);
int httpclient_perform(struct HTTPClient* const client);

int multihttpclient_init(struct MultiHTTPClient* const client, const size_t concurrency);
void multihttpclient_free(struct MultiHTTPClient* const client);

#endif
