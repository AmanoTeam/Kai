#if !defined(CLIOPTIONS_H)
#define CLIOPTIONS_H

#include <stdlib.h>
#include <errno.h>

#include "argparser.h"

#define M3U8_MEDIA_MAX_SELECTED (100)
#define M3U8_STREAM_MAX_SELECTED (M3U8_MEDIA_MAX_SELECTED + 1)

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
	int debug;
	int disable_autoselection;
	int disable_progress;
	int prefer_ffmpegc;
	int all_medias_selected;
	size_t selected_stream;
	char* url;
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
	struct M3U8HTTPClient* client
);

void clioptions_free(struct CLIOptions* const options);

#endif
