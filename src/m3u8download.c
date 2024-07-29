#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <curl/curl.h>

#include "m3u8.h"
#include "m3u8stream.h"
#include "m3u8errors.h"
#include "fstream.h"
#include "m3u8httpclient.h"
#include "pathsep.h"
#include "biggestint.h"
#include "m3u8download.h"
#include "m3u8utils.h"
#include "sutils.h"

struct M3U8Download {
	char* destination;
	struct FStream* stream;
	CURL* curl;
	struct M3U8StreamItem* item;
	size_t retries;
	struct M3U8HTTPClientError error;
};

int m3u8download_retryable(CURL* const curl, const CURLcode code) {
	
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

void m3u8download_free(struct M3U8Download* const download) {
	
	download->destination = NULL;
	
	curl_easy_cleanup(download->curl);
	download->curl = NULL;
	
	fstream_close(download->stream);
	download->stream = NULL;
	
	m3u8httpclient_errfree(&download->error);
	
}

struct M3U8DownloadQueue {
	size_t offset;
	size_t size;
	struct M3U8Download* items;
};

void m3u8dq_free(struct M3U8DownloadQueue* const queue) {
	
	size_t index = 0;
	
	for (index = 0; index < queue->offset; index++) {
		struct M3U8Download* const download = &queue->items[index];
		m3u8download_free(download);
	}
	
	queue->offset = 0;
	queue->size = 0;
	
	free(queue->items);
	queue->items = NULL;
	
}

size_t curl_write_file_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
	
	struct M3U8Download* download = (struct M3U8Download*) userdata;
	const size_t wsize = size * nmemb;
	
	if (download->stream == NULL) {
		download->stream = fstream_open(download->destination, FSTREAM_WRITE);
		
		if (download->stream == NULL) {
			return 0;
		}
		
		if (fstream_lock(download->stream) == -1) {
			return 0;
		}
	}
	
	if (fstream_write(download->stream, ptr, wsize) == -1) {
		return 0;
	}
	
	return wsize;
	
}

static int m3u8download_addqueue(
	struct M3U8DownloadQueue* const queue,
	struct M3U8Stream* const root,
	struct M3U8Stream* const resource,
	const char* const temporary_directory,
	const char* const uri,
	const struct M3U8ByteRange byterange,
	struct M3U8StreamItem* const item
) {
	
	int err = 0;
	int wsize = 0;
	size_t size = 0;
	
	CURLcode code = CURLE_OK;
	
	char* resolved_uri = NULL;
	char* range = NULL;
	
	struct M3U8Download source = {0};
	struct M3U8Download* destination = &queue->items[queue->offset];
	
	source.item = item;
	
	size = (
		strlen(temporary_directory) + strlen(PATHSEP) + uintlen(ptobiguint(uri)) + 1 + 3 + 1
	);
	
	source.destination = malloc(size);
	
	if (source.destination == NULL) {
		err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	strcpy(source.destination, temporary_directory);
	strcat(source.destination, PATHSEP);
	
	wsize = snprintf(source.destination + strlen(source.destination), 4096, "%"FORMAT_BIGGEST_INT_T, ptobiguint(uri));
	
	if (wsize < 1) {
		err = M3U8ERR_PRINTF_WRITE_FAILURE;
		goto end;
	}
	
	strcat(source.destination, ".bin");
	
	switch (resource->playlist.uri.type) {
		case M3U8_BASE_URI_TYPE_URL:
			err = m3u8uri_resolve_url(resource->playlist.uri.uri, uri, &resolved_uri);
			break;
		case M3U8_BASE_URI_TYPE_LOCAL_FILE:
			err = m3u8uri_resolve_path(resource->playlist.uri.uri, uri, &resolved_uri);
			break;
		default:
			err = M3U8ERR_LOAD_UNSUPPORTED_URI;
			break;
	}
	
	if (err != M3U8ERR_SUCCESS) {
		goto end;
	}
	
	/*
	Prepend the scheme "file://" for local files, otherwise curl will fail to parse it.
	*/
	if (resource->playlist.uri.type == M3U8_BASE_URI_TYPE_LOCAL_FILE) {
		char* new_resolved_uri = malloc(7 + strlen(resolved_uri) + 1);
		
		if (new_resolved_uri == NULL) {
			err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
			goto end;
		}
		
		strcpy(new_resolved_uri, "file://");
		strcat(new_resolved_uri, resolved_uri);
		
		free(resolved_uri);
		resolved_uri = new_resolved_uri;
	}
	
	source.curl = curl_easy_duphandle(root->playlist.client.curl);
	
	if (source.curl == NULL) {
		err = M3U8ERR_CURL_INIT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(source.curl, CURLOPT_VERBOSE, 0L);
	
	if (code != CURLE_OK) {
		err = M3U8ERR_CURL_SETOPT_FAILURE;
		goto end;
	}
	
	source.error.message = malloc(CURL_ERROR_SIZE);
	
	if (source.error.message == NULL) {
		err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(source.curl, CURLOPT_ERRORBUFFER, download.error.message);
	
	if (code != CURLE_OK) {
		err = M3U8ERR_CURL_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(source.curl, CURLOPT_REFERER, resource->playlist.uri.uri);
	
	if (code != CURLE_OK) {
		err = M3U8ERR_CURL_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(source.curl, CURLOPT_URL, resolved_uri);
	
	if (code != CURLE_OK) {
		err = M3U8ERR_CURL_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(source.curl, CURLOPT_WRITEFUNCTION, curl_write_file_cb);
	
	if (code != CURLE_OK) {
		err = M3U8ERR_CURL_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(source.curl, CURLOPT_WRITEDATA, destination);
	
	if (code != CURLE_OK) {
		err = M3U8ERR_CURL_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(source.curl, CURLOPT_SHARE, root->playlist.multi_client.curl_share);
	
	if (code != CURLE_OK) {
		err = M3U8ERR_CURL_SETOPT_FAILURE;
		goto end;
	}
	
	if (byterange.length > 0) {
		const biguint_t start = byterange.offset;
		const biguint_t end = (byterange.length + start) - 1;
		
		size = uintlen(start) + 1 + uintlen(end) + 1;
		range = malloc(size);
		
		if (range == NULL) {
			err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
			goto end;
		}
		
		wsize = snprintf(
			range,
			size,
			"%"FORMAT_BIGGEST_UINT_T"-%"FORMAT_BIGGEST_UINT_T,
			start,
			end
		);
		
		if (wsize < 1) {
			err = M3U8ERR_PRINTF_WRITE_FAILURE;
			goto end;
		}
		
		code = curl_easy_setopt(source.curl, CURLOPT_RANGE, range);
		
		if (code != CURLE_OK) {
			err = M3U8ERR_CURL_SETOPT_FAILURE;
			goto end;
		}
	}
	
	memcpy(destination, &source, sizeof(source));
	queue->offset += 1;
	
	end:;
	
	if (err != M3U8ERR_SUCCESS) {
		free(source.destination);
		m3u8download_free(&source);
	}
	
	free(range);
	free(resolved_uri);
	
	return err;
	
}

static int m3u8download_pollqueue(
	struct M3U8DownloadQueue* const queue,
	struct M3U8Stream* const root,
	const struct M3U8DownloadOptions* const options
) {
	
	int err = M3U8ERR_SUCCESS;
	CURLMcode code = CURLM_OK;
	
	size_t index = 0;
	size_t current = 0;
	
	int running = 1;
	CURLMsg* msg = NULL;
	int left = 0;
	
	struct M3U8Download* download = NULL;
	
	CURLM* curl_multi = root->playlist.multi_client.curl_multi;
	
	for (index = 0; index < queue->offset; index++) {
		struct M3U8Download* const download = &queue->items[index];
		code = curl_multi_add_handle(curl_multi, download->curl);
		
		if (code != CURLM_OK) {
			err = M3U8ERR_CURLM_ADD_FAILURE;
			goto end;
		}
	}
	
	if (options->progress_callback != NULL) {
		(*options->progress_callback)(queue->offset, current);
	}
	
	while (running) {
		code = curl_multi_perform(curl_multi, &running);
		
		if (code != CURLM_OK) {
			err = M3U8ERR_CURLM_PERFORM_FAILURE;
			goto end;
		}
		
		if (running) {
			code = curl_multi_poll(curl_multi, NULL, 0, 5000, NULL);
		}
		
		if (code != CURLM_OK) {
			err = M3U8ERR_CURLM_POLL_FAILURE;
			goto end;
		}
		
		while ((msg = curl_multi_info_read(curl_multi, &left))) {
			if (msg->msg != CURLMSG_DONE) {
				continue;
			}
			
			download = NULL;
			
			for (index = 0; index < queue->offset; index++) {
				struct M3U8Download* const subdownload = &queue->items[index];
				
				if (subdownload->curl == msg->easy_handle) {
					download = subdownload;
					break;
				}
			}
			
			code = curl_multi_remove_handle(curl_multi, msg->easy_handle);
			
			if (code != CURLM_OK) {
				err = M3U8ERR_CURLM_REMOVE_FAILURE;
				goto end;
			}
			
			if (msg->data.result != CURLE_OK) {
				const int status = fstream_seek(download->stream, 0, FSTREAM_SEEK_BEGIN);
				const int retryable = m3u8download_retryable(msg->easy_handle, msg->data.result);
				
				if (status == -1) {
					err = M3U8ERR_FSTREAM_SEEK_FAILURE;
					goto end;
				}
				
				if (download->retries++ > options->retry || !retryable) {
					struct M3U8HTTPClientError* const error = m3u8httpclient_geterror(&root->playlist.client);
					
					strcpy(error->message, download->error.message);
					error->code = msg->data.result;
					
					if (root->playlist.client.error.message[0] == '\0') {
						const char* const message = curl_easy_strerror(error->code);
						strcpy(error->message, message);
					}
					
					err = M3U8ERR_CURL_REQUEST_FAILURE;
					goto end;
				}
				
				code = curl_multi_add_handle(curl_multi, msg->easy_handle);
				
				if (code != CURLM_OK) {
					err = M3U8ERR_CURLM_ADD_FAILURE;
					goto end;
				}
			} else {
				current++;
				
				if (options->progress_callback != NULL) {
					(*options->progress_callback)(queue->offset, current);
				}
				
				curl_easy_cleanup(download->curl);
				download->curl = NULL;
				
				fstream_close(download->stream);
				download->stream = NULL;
			}
		}
	}
	
	end:;
	
	return err;
	
}
	

int m3u8stream_download(
	struct M3U8Stream* const root,
	struct M3U8Stream* const resource,
	const struct M3U8DownloadOptions* const options
) {
	
	size_t index = 0;
	int status = 0;
	
	int err = M3U8ERR_SUCCESS;
	
	struct M3U8Tag* tag = NULL;
	
	struct M3U8DownloadQueue queue = {0};
	
	err = m3u8mhttpclient_init(&root->playlist.multi_client, options->concurrency);
	
	if (err != M3U8ERR_SUCCESS) {
		goto end;
	}
	
	queue.size = sizeof(*queue.items) * (resource->offset * 2);
	queue.items = malloc(queue.size);
	
	if (queue.items == NULL) {
		err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	for (index = 0; index < resource->offset; index++) {
		struct M3U8StreamItem* const item = &resource->items[index];
		
		switch (item->type) {
			case M3U8_STREAM_SEGMENT: {
				struct M3U8Segment* const segment = ((struct M3U8Segment*) item->item);
				
				if (segment->key.uri != NULL) {
					err = m3u8download_addqueue(
						&queue,
						root,
						resource,
						options->temporary_directory,
						segment->key.uri,
						(struct M3U8ByteRange) {},
						item
					);
					
					if (err != M3U8ERR_SUCCESS) {
						goto end;
					}
				}
				
				err = m3u8download_addqueue(
					&queue,
					root,
					resource,
					options->temporary_directory,
					segment->uri,
					segment->byterange,
					item
				);
				
				if (err != M3U8ERR_SUCCESS) {
					goto end;
				}
				
				break;
			}
			case M3U8_STREAM_MAP: {
				struct M3U8Map* const map = ((struct M3U8Map*) item->item);
				
				err = m3u8download_addqueue(
					&queue,
					root,
					resource,
					options->temporary_directory,
					map->uri,
					map->byterange,
					item
				);
				
				if (err != M3U8ERR_SUCCESS) {
					goto end;
				}
				
				break;
			}
			default: {
				break;
			}
		}
	}
	
	err = m3u8download_pollqueue(&queue, root, options);
	
	if (err != M3U8ERR_SUCCESS) {
		goto end;
	}
	
	for (index = 0; index < queue.offset; index++) {
		struct M3U8Download* download = &queue.items[index];
		struct M3U8StreamItem* item = download->item;
		struct M3U8Attribute* attribute = NULL;
		
		switch (item->type) {
			case M3U8_STREAM_SEGMENT: {
				struct M3U8Segment* segment = item->item;
				
				if (segment->key.uri != NULL && segment->key.uri != download->destination) {
					attribute = m3u8tag_igetattr(segment->key.tag, M3U8_ATTRIBUTE_URI);
					
					free(attribute->value);
					attribute->value = download->destination;
					
					index++;
					
					download = &queue.items[index];
					item = download->item;
					
					segment = item->item;
					
					free(segment->tag->uri);
					segment->tag->uri = download->destination;
				} else {
					free(segment->tag->uri);
					segment->tag->uri = download->destination;
				}
				
				break;
			}
			case M3U8_STREAM_MAP: {
				struct M3U8Map* const map = ((struct M3U8Map*) item->item);
				attribute = m3u8tag_igetattr(map->tag, M3U8_ATTRIBUTE_URI);
				
				free(attribute->value);
				attribute->value = download->destination;
				
				break;
			}
			default: {
				break;
			}
		}
	}
	
	status = 0;
	
	/* Delete all the #EXT-X-BYTERANGE tags within the playlist */
	do {
		status = m3u8playlist_ideltag(&resource->playlist, M3U8_TAG_EXT_X_BYTERANGE);
	} while (status);
	
	tag = m3u8playlist_igettag(&resource->playlist, M3U8_TAG_EXT_X_MAP);
	
	if (tag != NULL) {
		m3u8tag_idelattr(tag, M3U8_ATTRIBUTE_BYTERANGE);
	}
	
	end:;
	
	if (err != M3U8ERR_SUCCESS) {
		for (index = 0; index < queue.offset; index++) {
			struct M3U8Download* const download = &queue.items[index];
			free(download->destination);
		}
	}
	
	m3u8dq_free(&queue);
	
	return err;
	
}
