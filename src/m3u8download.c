#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <curl/curl.h>

#include "m3u8.h"
#include "m3u8stream.h"
#include "errors.h"
#include "fstream.h"
#include "m3u8httpclient.h"
#include "pathsep.h"
#include "biggestint.h"
#include "m3u8download.h"
#include "m3u8utils.h"
#include "sutils.h"

struct M3U8Download {
	char* filename;
	struct FStream* stream;
	CURL* curl;
	struct M3U8StreamItem* item;
	size_t retries;
	struct M3U8HTTPClientError error;
	int copy;
};

void m3u8download_free(struct M3U8Download* const download) {
	
	if (!download->copy) {
		free(download->filename);
	}
	
	download->filename = NULL;
	
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
		download->stream = fstream_open(download->filename, FSTREAM_WRITE);
		
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
	size_t index = 0;
	
	CURLcode code = CURLE_OK;
	
	char* resolved_uri = NULL;
	char* range = NULL;
	
	struct M3U8Download source = {0};
	struct M3U8Download* destination = &queue->items[queue->offset];
	
	source.item = item;
	
	err = m3u8uri_resolve(&resource->playlist.uri, uri, &resolved_uri);
	
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
	
	/*
	It may happen that a media playlist has all of its media segments encrypted
	using the same key. In that case, we must compare these URLs to avoid sending
	multiple unnecessary requests.
	*/
	for (index = 0; index < queue->offset; index++) {
		struct M3U8Download* const pending = &queue->items[index];
		char* url = NULL;
		
		CURLcode code = CURLE_OK;
		
		if (pending->curl == NULL) {
			continue;
		}
		
		if (byterange.length != 0) {
			break;
		}
		
		code = curl_easy_getinfo(pending->curl, CURLINFO_EFFECTIVE_URL, &url);
		
		if (code != CURLE_OK) {
			err = M3U8ERR_CURLE_GET_INFO_FAILURE;
			goto end;
		}
		
		if (strcmp(resolved_uri, url) != 0) {
			continue;
		}
		
		source.filename = pending->filename;
		source.copy = 1;
		
		goto end;
	}
	
	source.filename = malloc(strlen(temporary_directory) + strlen(PATHSEP) + uintptrlen((uintptr_t) uri) + 1 + 3 + 1);
	
	if (source.filename == NULL) {
		err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	strcpy(source.filename, temporary_directory);
	strcat(source.filename, PATHSEP);
	
	wsize = snprintf(
		source.filename + strlen(source.filename),
		uintptrlen((uintptr_t) uri) + 1,
		"%"FORMAT_UINT_PTR_T, (uintptr_t) uri
	);
	
	if (wsize < 1) {
		err = M3U8ERR_PRINTF_WRITE_FAILURE;
		goto end;
	}
	
	strcat(source.filename, ".bin");
	
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
	
	code = curl_easy_setopt(source.curl, CURLOPT_ERRORBUFFER, source.error.message);
	
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
		
		const size_t size = uintlen(start) + 1 + uintlen(end) + 1;
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
	
	end:;
	
	memcpy(destination, &source, sizeof(source));
	queue->offset += 1;
	
	if (err != M3U8ERR_SUCCESS) {
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
	size_t total = 0;
	
	int running = 1;
	CURLMsg* msg = NULL;
	int left = 0;
	
	struct M3U8Download* download = NULL;
	
	CURLM* curl_multi = root->playlist.multi_client.curl_multi;
	
	for (index = 0; index < queue->offset; index++) {
		struct M3U8Download* const download = &queue->items[index];
		
		if (download->curl == NULL) {
			continue;
		}
		
		code = curl_multi_add_handle(curl_multi, download->curl);
		
		if (code != CURLM_OK) {
			err = M3U8ERR_CURLM_ADD_FAILURE;
			goto end;
		}
		
		total += 1;
	}
	
	if (options->progress_callback != NULL) {
		(*options->progress_callback)(total, current);
	}
	
	while (running) {
		code = curl_multi_perform(curl_multi, &running);
		
		if (code != CURLM_OK) {
			err = M3U8ERR_CURLM_PERFORM_FAILURE;
			goto end;
		}
		
		if (running) {
			code = curl_multi_poll(curl_multi, NULL, 0, 0, NULL);
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
				const int retryable = m3u8httpclient_retryable(msg->easy_handle, msg->data.result);
				
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
				
				if (download->stream != NULL) {
					const int status = fstream_seek(download->stream, 0, FSTREAM_SEEK_BEGIN);
					
					if (status == -1) {
						err = M3U8ERR_FSTREAM_SEEK_FAILURE;
						goto end;
					}
				}
				
				code = curl_multi_add_handle(curl_multi, msg->easy_handle);
				
				if (code != CURLM_OK) {
					err = M3U8ERR_CURLM_ADD_FAILURE;
					goto end;
				}
			} else {
				current++;
				
				if (options->progress_callback != NULL) {
					(*options->progress_callback)(total, current);
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
	
	const biguint_t segments = m3u8stream_getsegments(resource);
	
	if (segments < 1) {
		err = M3U8ERR_MEDIA_PLAYLIST_NO_SEGMENTS;
		goto end;
	}
	
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
				
				if (segment->key.uri != NULL && segment->key.uri != download->filename) {
					attribute = m3u8tag_igetattr(segment->key.tag, M3U8_ATTRIBUTE_URI);
					
					free(attribute->value);
					attribute->value = malloc(strlen(download->filename) + 1);
					
					if (attribute->value == NULL) {
						err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
						goto end;
					}
					
					strcpy(attribute->value, download->filename);
					
					index++;
					
					download = &queue.items[index];
					item = download->item;
					
					segment = item->item;
				}
				
				free(segment->tag->uri);
				segment->tag->uri = malloc(strlen(download->filename) + 1);
				
				if (segment->tag->uri == NULL) {
					err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
					goto end;
				}
				
				strcpy(segment->tag->uri, download->filename);
				
				break;
			}
			case M3U8_STREAM_MAP: {
				struct M3U8Map* const map = ((struct M3U8Map*) item->item);
				attribute = m3u8tag_igetattr(map->tag, M3U8_ATTRIBUTE_URI);
				
				free(attribute->value);
				attribute->value = malloc(strlen(download->filename) + 1);
				
				if (attribute->value == NULL) {
					err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
					goto end;
				}
				
				strcpy(attribute->value, download->filename);
				
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
	
	/* Delete the BYTERANGE attribute from the #EXT-X-BYTERANGE tag */
	tag = m3u8playlist_igettag(&resource->playlist, M3U8_TAG_EXT_X_MAP);
	
	if (tag != NULL) {
		m3u8tag_idelattr(tag, M3U8_ATTRIBUTE_BYTERANGE);
	}
	
	end:;
	
	m3u8dq_free(&queue);
	
	return err;
	
}
