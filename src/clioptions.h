#if !defined(CLIOPTIONS_H)
#define CLIOPTIONS_H

#include <stdlib.h>

#include "argparse.h"

#define M3U8_MEDIA_MAX_SELECTED (100)
#define M3U8_STREAM_MAX_SELECTED (M3U8_MEDIA_MAX_SELECTED + 1)

#define SELECT_STREAM_BEST (0x4000)
#define SELECT_STREAM_MEDIUM (0x4001)
#define SELECT_STREAM_WORST (0x4002)

#define SELECT_STREAM_RESOLUTION (0x5000)

struct M3U8SelectedStreams {
	size_t offset;
	size_t size;
	struct M3U8Stream** items;
};

struct M3U8SelectedMedias {
	size_t offset;
	size_t size;
	size_t* items;
};

struct CLIOptions {
	int show_formats;
	int show_help;
	int show_version;
	int select_stream;
	int user_agent;
	int proxy;
	int doh_url;
	int referer;
	int insecure;
	int disable_cookies;
	int verbose;
	int assume_yes;
	int disable_autoselection;
	int disable_progress;
	int prefer_ffmpegc;
	int dump;
	int return_error_code;
	int all_medias_selected;
	size_t selected_stream;
	int http10;
	int http11;
	int http2;
	int randomized_temporary_directory;
	int max_redirects;
	int concurrency;
	int retry;
	char* url;
	char* base_url;
	char* key;
	char* output;
	struct curl_slist* headers;
	struct M3U8SelectedMedias selected_medias;
	struct M3U8SelectedStreams selected_streams;
	struct M3U8DownloadOptions download_options;
};

int clioptions_parse(
	struct CLIOptions* const options,
	struct ArgumentParser* const argparser,
	const struct Argument** argument,
	struct HTTPClient* client
);

void clioptions_free(struct CLIOptions* const options);

#endif
