#include <stdlib.h>
#include <string.h>

#include "wcurl.h"
#include "sslcerts.h"

static wcurl_t global_wcurl = {0};
static wcurl_multi_t global_wcurl_multi = {0};

static void wcurl_setcurl(wcurl_t* const wcurl, CURL* const curl) {
	wcurl->curl = curl;
}

CURL* wcurl_getcurl(wcurl_t* const wcurl) {
	return wcurl->curl;
}

wcurl_t* wcurl_getglobal(void) {
	
	int err = WCURL_ERR_SUCCESS;
	
	if (wcurl_getcurl(&global_wcurl) == NULL) {
		err = wcurl_init(&global_wcurl);
		
		if (err != WCURL_ERR_SUCCESS) {
			return NULL;
		}
	}
	
	return &global_wcurl;
	
}

int wcurl_init(wcurl_t* const wcurl) {
	
	int err = WCURL_ERR_SUCCESS;
	CURLcode code = CURLE_OK;
	
	const char* msg = NULL;
	
	wcurl_error_t* werror = wcurl_geterr(wcurl);
	CURL* curl = wcurl_getcurl(wcurl);
	
	if (curl != NULL) {
		return err;
	}
	
	curl = curl_easy_init();
	
	if (curl == NULL) {
		err = WCURL_ERR_INIT_FAILURE;
		goto end;
	}
	
	wcurl_setcurl(wcurl, curl);
	
	code = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	
	if (code != CURLE_OK) {
		err = WCURL_ERR_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 20L);
	
	if (code != CURLE_OK) {
		err = WCURL_ERR_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
	
	if (code != CURLE_OK) {
		err = WCURL_ERR_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
	
	if (code != CURLE_OK) {
		err = WCURL_ERR_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 30L);
	
	if (code != CURLE_OK) {
		err = WCURL_ERR_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, 15L);
	
	if (code != CURLE_OK) {
		err = WCURL_ERR_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
	
	if (code != CURLE_OK) {
		err = WCURL_ERR_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 8L);
	
	if (code != CURLE_OK) {
		err = WCURL_ERR_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(curl, CURLOPT_DNS_CACHE_TIMEOUT, -1L);
	
	if (code != CURLE_OK) {
		err = WCURL_ERR_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(curl, CURLOPT_DNS_SHUFFLE_ADDRESSES, 1L);
	
	if (code != CURLE_OK) {
		err = WCURL_ERR_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1 | CURL_HTTP_VERSION_2_0);
	
	if (code != CURLE_OK) {
		err = WCURL_ERR_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
	
	if (code != CURLE_OK) {
		err = WCURL_ERR_SETOPT_FAILURE;
		goto end;
	}
	
	werror->msg = malloc(CURL_ERROR_SIZE);
	
	if (werror->msg == NULL) {
		err = WCURL_ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	werror->msg[0] = '\0';
	
	code = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, werror->msg);
	
	if (code != CURLE_OK) {
		err = WCURL_ERR_SETOPT_FAILURE;
		goto end;
	}
	
	#if defined(WCURL_DISABLE_SSL_VERIFY)
		code = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		
		if (code != CURLE_OK) {
			err = WCURL_ERR_SETOPT_FAILURE;
			goto end;
		}
		
		code = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		
		if (code != CURLE_OK) {
			err = WCURL_ERR_SETOPT_FAILURE;
			goto end;
		}
	#else
		err = sslcerts_load_certificates(curl);
		
		if (err != SSLCERTS_SUCCESS) {
			err = WCURL_ERR_SSL_CERT_LOAD_FAILURE;
			goto end;
		}
	#endif
	
	end:;
	
	werror->code = code;
	
	if (werror->code != CURLE_OK) {
		msg = curl_easy_strerror(werror->code);
		strcpy(werror->msg, msg);
	}
	
	if (err != WCURL_ERR_SUCCESS) {
		wcurl_free(wcurl);
	}
	
	return err;
	
}

int wcurl_duplicate(wcurl_t* const source, wcurl_t* const destination) {
	
	CURL* old = wcurl_getcurl(source);
	CURL* new = curl_easy_duphandle(old);
	
	if (new == NULL) {
		return WCURL_ERR_INIT_FAILURE;
	}
	
	destination->curl = new;
	
	return WCURL_ERR_SUCCESS;
	
}

void wcurlerr_free(wcurl_error_t* const err) {
	
	if (err == NULL) {
		return;
	}
	
	free(err->msg);
	err->msg = NULL;
	
	err->code = CURLE_OK;
	
}

void wcurl_free(wcurl_t* const wcurl) {
	
	curl_easy_cleanup(wcurl->curl);
	wcurl->curl = NULL;
	
}

void wcurl_global_free(void) {
	
	wcurl_free(&global_wcurl);
	
}

wcurl_error_t* wcurl_geterr(wcurl_t* const wcurl) {
	
	return &wcurl->error;
	
}

int wcurl_retryable(
	CURL* const curl,
	const CURLcode code
) {
	
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

int wcurl_perform(wcurl_t* const wcurl) {
	
	int err = WCURL_ERR_SUCCESS;
	
	int retryable = 0;
	size_t retries = 0;
	
	const char* msg = NULL;
	wcurl_error_t* werror = wcurl_geterr(wcurl);
	
	do {
		werror->code = curl_easy_perform(wcurl->curl);
		
		if (werror->code == CURLE_OK) {
			break;
		}
		
		retryable = wcurl_retryable(wcurl->curl, werror->code);
		
		if (!retryable) {
			break;
		}
	} while (retries++ <= wcurl->retry);
	
	if (werror->msg[0] == '\0') {
		msg = curl_easy_strerror(werror->code);
		strcpy(werror->msg, msg);
	}
	
	if (werror->code != CURLE_OK) {
		err = WCURL_ERR_REQUEST_FAILURE;
	}
	
	return err;
	
}

static void wcurlmlt_setcurl(
	wcurl_multi_t* const wcurl_multi,
	CURLM* curl_multi
) {
	wcurl_multi->curl_multi = curl_multi;
}

CURLM* wcurlmlt_getcurl(wcurl_multi_t* const wcurl_multi) {
	return wcurl_multi->curl_multi;
}

wcurl_multi_t* wcurlmlt_getglobal(const size_t concurrency) {
	
	int err = WCURL_ERR_SUCCESS;
	
	if (wcurlmlt_getcurl(&global_wcurl_multi) == NULL) {
		err = wcurlmlt_init(&global_wcurl_multi, concurrency);
		
		if (err != WCURL_ERR_SUCCESS) {
			return NULL;
		}
	}
	
	return &global_wcurl_multi;
	
}

static void wcurlmlt_setshr(
	wcurl_multi_t* const wcurl_multi,
	CURLSH* const curl_share
) {
	wcurl_multi->curl_share = curl_share;
}

CURLSH* wcurlmlt_getshr(wcurl_multi_t* const wcurl_multi) {
	return wcurl_multi->curl_share;
}

int wcurlmlt_init(
	wcurl_multi_t* const wcurl_multi,
	const size_t concurrency
) {
	
	int err = WCURL_ERR_SUCCESS;
	
	CURLMcode code = CURLM_OK;
	CURLSHcode shcode = CURLSHE_OK;
	
	CURLM* curl_multi = wcurlmlt_getcurl(wcurl_multi);
	CURLSH* curl_share = wcurlmlt_getshr(wcurl_multi);
	
	if (curl_multi != NULL) {
		goto end;
	}
	
	curl_multi = curl_multi_init();
	
	if (curl_multi == NULL) {
		err = WCURLMLT_ERR_INIT_FAILURE;
		goto end;
	}
	
	wcurlmlt_setcurl(wcurl_multi, curl_multi);
	
	code = curl_multi_setopt(curl_multi, CURLMOPT_MAX_HOST_CONNECTIONS, (long) concurrency);
	
	if (code != CURLM_OK) {
		err = WCURLMLT_ERR_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_multi_setopt(curl_multi, CURLMOPT_MAX_TOTAL_CONNECTIONS, (long) concurrency);
	
	if (code != CURLM_OK) {
		err = WCURLMLT_ERR_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_multi_setopt(curl_multi, CURLMOPT_PIPELINING, CURLPIPE_MULTIPLEX);
	
	if (code != CURLM_OK) {
		err = WCURLMLT_ERR_SETOPT_FAILURE;
		goto end;
	}
	
	if (curl_share != NULL) {
		goto end;
	}
	
	curl_share = curl_share_init();
	
	if (curl_share == NULL) {
		err = WCURLSHR_ERR_INIT_FAILURE;
		goto end;
	}
	
	wcurlmlt_setshr(wcurl_multi, curl_share);
	
	shcode = curl_share_setopt(curl_share, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE);
	
	if (shcode != CURLSHE_OK) {
		err = WCURLSHR_ERR_SETOPT_FAILURE;
		goto end;
	}
	
	end:;
	
	if (err != WCURL_ERR_SUCCESS) {
		wcurlmlt_free(wcurl_multi);
	}
	
	return err;
	
}

void wcurlmlt_free(wcurl_multi_t* const wcurl_multi) {
	
	curl_multi_cleanup(wcurl_multi->curl_multi);
	wcurl_multi->curl_multi = NULL;
	
	curl_share_cleanup(wcurl_multi->curl_share);
	wcurl_multi->curl_share = NULL;
	
}

void wcurlmlt_global_free(void) {
	
	wcurlmlt_free(&global_wcurl_multi);
	
}
