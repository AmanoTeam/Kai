#if !defined(M3U8DOWNLOAD_H)
#define M3U8DOWNLOAD_H

#include <stdlib.h>

#include <curl/curl.h>

#include "httpclient.h"
#include "m3u8stream.h"
#include "fstream.h"

struct M3U8Download {
	CURL* curl;
	char* filename;
	struct FStream* stream;
	struct M3U8StreamItem* item;
	struct HTTPClientError error;
	size_t retries;
	int copy;
};

struct M3U8DownloadOptions {
	size_t concurrency;
	size_t retry;
	char* temporary_directory;
	void (*progress_callback)(const size_t total, const size_t current);
};

int m3u8stream_download(
	struct M3U8Stream* const root,
	struct M3U8Stream* const resource,
	const struct M3U8DownloadOptions* const options
);

#endif
