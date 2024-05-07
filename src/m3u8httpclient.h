#if !defined(M3U8HTTPCLIENT_H)
#define M3U8HTTPCLIENT_H

#include <curl/curl.h>

struct M3U8HTTPClientError {
	CURLcode code;
	char* message;
};

struct M3U8HTTPClient {
	CURL* curl;
	struct M3U8HTTPClientError error;
	size_t retry;
};

struct M3U8MultiHTTPClient {
	CURLM* curl_multi;
	CURLSH* curl_share;
};

int m3u8httpclient_init(struct M3U8HTTPClient* const client);

void m3u8httpclient_errfree(struct M3U8HTTPClientError* const error);
void m3u8httpclient_free(struct M3U8HTTPClient* const client);

struct M3U8HTTPClientError* m3u8httpclient_geterror(struct M3U8HTTPClient* const client);
CURL* m3u8httpclient_getclient(struct M3U8HTTPClient* const client);

int m3u8httpclient_retryable(CURL* const curl, const CURLcode code);
int m3u8httpclient_perform(struct M3U8HTTPClient* const client);

int m3u8mhttpclient_init(struct M3U8MultiHTTPClient* const client, const size_t concurrency);
void m3u8mhttpclient_free(struct M3U8MultiHTTPClient* const client);

#endif
