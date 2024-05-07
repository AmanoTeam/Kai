#if !defined(M3U8HTTPCLIENT_H)
#define M3U8HTTPCLIENT_H

#include <curl/curl.h>

struct M3U8HTTPClient {
	CURL* curl;
	CURLcode curl_code;
	char* curl_error_message;
};

struct M3U8MultiHTTPClient {
	CURLM* curl_multi;
};

int m3u8httpclient_init(struct M3U8HTTPClient* const client);
void m3u8httpclient_free(struct M3U8HTTPClient* const client);
CURL* m3u8httpclient_getclient(struct M3U8HTTPClient* const client);

int m3u8mhttpclient_init(struct M3U8MultiHTTPClient* const client, const size_t concurrency);
void m3u8mhttpclient_free(struct M3U8MultiHTTPClient* const client);

#endif
