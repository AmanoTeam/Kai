#include <stdlib.h>
#include <string.h>

#include "m3u8errors.h"
#include "m3u8httpclient.h"
#include "sslcerts.h"

int m3u8httpclient_init(struct M3U8HTTPClient* const client) {
	
	int err = M3U8ERR_SUCCESS;
	CURLcode code = CURLE_OK;
	
	if (client->curl != NULL) {
		return err;
	}
	
	client->curl = curl_easy_init();
	
	if (client->curl == NULL) {
		err = M3U8ERR_CURL_INIT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(client->curl, CURLOPT_FAILONERROR, 1L);
	
	if (code != CURLE_OK) {
		err = M3U8ERR_CURL_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(client->curl, CURLOPT_TCP_KEEPALIVE, 1L);
	
	if (code != CURLE_OK) {
		err = M3U8ERR_CURL_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(client->curl, CURLOPT_TCP_KEEPIDLE, 30L);
	
	if (code != CURLE_OK) {
		err = M3U8ERR_CURL_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(client->curl, CURLOPT_TCP_KEEPINTVL, 15L);
	
	if (code != CURLE_OK) {
		err = M3U8ERR_CURL_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(client->curl, CURLOPT_VERBOSE, 0L);
	
	if (code != CURLE_OK) {
		err = M3U8ERR_CURL_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(client->curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Android 10; Mobile; rv:109.0) Gecko/115.0 Firefox/115.0");
	
	if (code != CURLE_OK) {
		err = M3U8ERR_CURL_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(client->curl, CURLOPT_DNS_CACHE_TIMEOUT, -1L);
	
	if (code != CURLE_OK) {
		err = M3U8ERR_CURL_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(client->curl, CURLOPT_DNS_SHUFFLE_ADDRESSES, 1L);
	
	if (code != CURLE_OK) {
		err = M3U8ERR_CURL_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(client->curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
	
	if (code != CURLE_OK) {
		err = M3U8ERR_CURL_SETOPT_FAILURE;
		goto end;
	}
	
	client->error.message = malloc(CURL_ERROR_SIZE);
	
	if (client->error.message == NULL) {
		err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	client->error.message[0] = '\0';
	
	code = curl_easy_setopt(client->curl, CURLOPT_ERRORBUFFER, client->error.message);
	
	if (code != CURLE_OK) {
		err = M3U8ERR_CURL_SETOPT_FAILURE;
		goto end;
	}
	
	err = sslcerts_load_certificates(client->curl);
	
	end:;
	
	client->error.code = code;
	
	if (client->error.code != CURLE_OK) {
		const char* const message = curl_easy_strerror(client->error.code);
		strcpy(client->error.message, message);
	}
	
	if (err != M3U8ERR_SUCCESS) {
		m3u8httpclient_free(client);
	}
	
	return err;
	
}

void m3u8httpclient_errfree(struct M3U8HTTPClientError* const error) {
	
	free(error->message);
	error->message = NULL;
	
	error->code = CURLE_OK;
	
}

void m3u8httpclient_free(struct M3U8HTTPClient* const client) {
	
	curl_easy_cleanup(client->curl);
	client->curl = NULL;
	
}

struct M3U8HTTPClientError* m3u8httpclient_geterror(struct M3U8HTTPClient* const client) {
	
	return &client->error;
	
}

CURL* m3u8httpclient_getclient(struct M3U8HTTPClient* const client) {
	
	return client->curl;
	
}

int m3u8httpclient_perform(struct M3U8HTTPClient* const client) {
	
	int err = M3U8ERR_SUCCESS;
	
	client->error.code = curl_easy_perform(client->curl);
	
	if (client->error.message[0] == '\0') {
		const char* const message = curl_easy_strerror(client->error.code);
		strcpy(client->error.message, message);
	}
	
	if (client->error.code != CURLE_OK) {
		err = M3U8ERR_CURL_REQUEST_FAILURE;
	}
	
	return err;
	
}

int m3u8mhttpclient_init(struct M3U8MultiHTTPClient* const client, const size_t concurrency) {
	
	int err = M3U8ERR_SUCCESS;
	CURLMcode code = CURLM_OK;
	
	if (client->curl_multi != NULL) {
		goto end;
	}
	
	client->curl_multi = curl_multi_init();
	
	if (client->curl_multi == NULL) {
		err = M3U8ERR_CURLM_INIT_FAILURE;
		goto end;
	}
	
	code = curl_multi_setopt(client->curl_multi, CURLMOPT_MAX_HOST_CONNECTIONS, (long) concurrency);
	
	if (code != CURLM_OK) {
		err = M3U8ERR_CURLM_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_multi_setopt(client->curl_multi, CURLMOPT_MAX_TOTAL_CONNECTIONS, (long) concurrency);
	
	if (code != CURLM_OK) {
		err = M3U8ERR_CURLM_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_multi_setopt(client->curl_multi, CURLMOPT_PIPELINING, CURLPIPE_MULTIPLEX);
	
	if (code != CURLM_OK) {
		err = M3U8ERR_CURLM_SETOPT_FAILURE;
		goto end;
	}
	
	end:;
	
	if (err != M3U8ERR_SUCCESS) {
		m3u8mhttpclient_free(client);
	}
	
	return err;
	
}

void m3u8mhttpclient_free(struct M3U8MultiHTTPClient* const client) {
	
	curl_multi_cleanup(client->curl_multi);
	client->curl_multi = NULL;
	
}