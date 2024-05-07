#include <stdlib.h>
#include <string.h>

#include "errors.h"
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
	
	code = curl_easy_setopt(client->curl, CURLOPT_FOLLOWLOCATION, 1L);
	
	if (code != CURLE_OK) {
		err = M3U8ERR_CURL_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(client->curl, CURLOPT_MAXREDIRS, 20L);
	
	if (code != CURLE_OK) {
		err = M3U8ERR_CURL_SETOPT_FAILURE;
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
	
	code = curl_easy_setopt(client->curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1 | CURL_HTTP_VERSION_2_0);
	
	if (code != CURLE_OK) {
		err = M3U8ERR_CURL_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(client->curl, CURLOPT_COOKIEFILE, "");
	
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
	
	if (error == NULL) {
		return;
	}
	
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

int m3u8httpclient_retryable(CURL* const curl, const CURLcode code) {
	
	switch (code) {
		case CURLE_HTTP_RETURNED_ERROR: {
			long status_code = 0;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
			
			if (status_code == 408 || status_code == 429 || status_code == 500 ||
				status_code == 502 || status_code == 503 || status_code == 504) {
				return 1;
			}
			
			break;
		}
		case CURLE_SEND_ERROR:
		case CURLE_OPERATION_TIMEDOUT:
		case CURLE_PARTIAL_FILE:
		case CURLE_RECV_ERROR:
			return 1;
		default:
			break;
	}
	
	return 0;
	
}

int m3u8httpclient_perform(struct M3U8HTTPClient* const client) {
	
	int err = M3U8ERR_SUCCESS;
	int retryable = 0;
	
	size_t retries = 0;
	
	do {
		client->error.code = curl_easy_perform(client->curl);
		
		if (client->error.code == CURLE_OK) {
			break;
		}
		
		retryable = m3u8httpclient_retryable(client->curl, client->error.code);
		
		if (!retryable) {
			break;
		}
	} while (retries++ <= client->retry);
	
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
	CURLSHcode shcode = CURLSHE_OK;
	
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
	
	if (client->curl_share != NULL) {
		goto end;
	}
	
	client->curl_share = curl_share_init();
	
	if (client->curl_share == NULL) {
		err = M3U8ERR_CURLSH_INIT_FAILURE;
		goto end;
	}
	
	shcode = curl_share_setopt(client->curl_share, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE);
	
	if (shcode != CURLSHE_OK) {
		err = M3U8ERR_CURLSH_SETOPT_FAILURE;
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
	
	curl_share_cleanup(client->curl_share);
	client->curl_share = NULL;
	
}