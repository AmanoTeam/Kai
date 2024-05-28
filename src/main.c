#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <curl/curl.h>

#include "m3u8stream.h"
#include "m3u8types.h"
#include "m3u8utils.h"
#include "m3u8download.h"
#include "m3u8httpclient.h"
#include "m3u8errors.h"
#include "m3u8.h"

#include "argparser.h"
#include "program_help.h"
#include "kai.h"
#include "os.h"
#include "sutils.h"
#include "ffmpeg_muxer.h"
#include "terminal.h"
#include "pathsep.h"
#include "filesystem.h"
#include "path.h"
#include "showformats.h"
#include "callbacks.h"
#include "biggestint.h"
#include "sslcerts.h"
#include "resources.h"

#if defined(_WIN32) && defined(_UNICODE)
	#include "wio.h"
	#define main wmain
#endif

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

struct M3U8DownloadedStreams {
	size_t offset;
	size_t size;
	char** items;
};

static void m3u8ss_free(struct M3U8SelectedStreams* const streams) {
	
	free(streams->items);
	streams->items = NULL;
	
	streams->offset = 0;
	streams->size = 0;
	
}

static void m3u8sm_free(struct M3U8SelectedMedias* const medias) {
	
	free(medias->items);
	medias->items = NULL;
	
	medias->offset = 0;
	medias->size = 0;
	
}

static void m3u8ds_free(struct M3U8DownloadedStreams* const streams) {
	
	size_t index = 0;
	
	for (index = 0; index < streams->offset; index++) {
		char* const item = streams->items[index];
		free(item);
	}
	
	free(streams->items);
	streams->items = NULL;
	
	streams->offset = 0;
	streams->size = 0;
	
}

const struct M3U8StreamItem* m3u8stream_finditem(const struct M3U8Stream* const root, const struct M3U8Stream* const resource) {
	/*
	Look for a M3U8 item that matches the given M3U8 stream.
	*/
	
	size_t index = 0;
	
	if (root->playlist.type != M3U8_PLAYLIST_TYPE_MASTER) {
		return NULL;
	}
	
	for (index = 0; index < root->offset; index++) {
		const struct M3U8StreamItem* const item = &root->items[index];
		
		switch (item->type) {
			case M3U8_STREAM_VARIANT_STREAM: {
				const struct M3U8VariantStream* const variant_stream = item->item;
				
				if (&variant_stream->stream != resource) {
					break;
				}
				
				return item;
			}
			case M3U8_STREAM_MEDIA: {
				const struct M3U8Media* const media = item->item;
				
				if (&media->stream != resource) {
					break;
				}
				
				return item;
			}
			default: {
				break;
			}
		}
	}
	
	return NULL;
	
}

int main(int argc, argv_t* argv[]) {
	
	int err = M3U8ERR_SUCCESS;
	int fferr = 0;
	
	struct FStream* output_stream = NULL;
	
	struct M3U8HTTPClient* client = NULL;
	struct M3U8HTTPClientError* cerror = NULL;
	
	struct M3U8Stream stream = {0};
	
	struct ArgumentParser argparser = {0};
	const struct Argument* argument = NULL;
	
	int show_formats = 0;
	int show_help = 0;
	int show_version = 0;
	int select_stream = 0;
	int user_agent = 0;
	int proxy = 0;
	int doh_url = 0;
	int referer = 0;
	int insecure = 0;
	int debug = 0;
	int disable_autoselection = 0;
	
	int select_all_medias = 0;
	int exists = 0;
	
	char* url = NULL;
	char* key = NULL;
	char* output = NULL;
	char* temporary_file = NULL;
	char* temporary_directory = NULL;
	char* directory = NULL;
	char* name = NULL;
	
	const char* file_extension = NULL;
	
	size_t select_media_index = 0;
	size_t select_stream_index = 0;
	
	size_t index = 0;
	size_t subindex = 0;
	
	biguint_t value = 0;
	int wsize = 0;
	
	struct M3U8DownloadOptions download_options = {
		.concurrency = 1,
		.retry = 0,
		.progress_callback = &progress_callback
	};
	
	struct M3U8SelectedMedias selected_medias = {0};
	struct M3U8SelectedStreams selected_streams = {0};
	struct M3U8DownloadedStreams downloaded_streams = {0};
	
	#if defined(_WIN32) && defined(_UNICODE)
		wio_set_unicode();
	#endif
	
	if (resources_increase_maxfd() == -1) {
		fprintf(stderr, "+ warning: could not increase max number of open file descriptors\n");
	}
	
	hide_cursor();
	
	temporary_directory = get_temporary_directory();
	
	if (temporary_directory == NULL) {
		err = M3U8ERR_DOWNLOAD_NO_TMPDIR;
		goto end;
	}
	
	directory = malloc(strlen(temporary_directory) + strlen(PATHSEP) + strlen(PROGRAM_NAME) + 1);
	
	if (directory == NULL) {
		err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	strcpy(directory, temporary_directory);
	strcat(directory, PATHSEP);
	strcat(directory, PROGRAM_NAME);
	
	if (create_directory(directory) != 0) {
		err = M3U8ERR_DOWNLOAD_COULD_NOT_CREATE_TMPDIR;
		goto end;
	}
	
	free(temporary_directory);
	temporary_directory = directory;
	
	directory = NULL;
	
	download_options.temporary_directory = temporary_directory;
	
	selected_medias.size = sizeof(*selected_medias.items) * M3U8_MEDIA_MAX_SELECTED;
	selected_medias.items = malloc(selected_medias.size);
	
	if (selected_medias.items == NULL) {
		err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	selected_streams.size = sizeof(*selected_streams.items) * M3U8_STREAM_MAX_SELECTED;
	selected_streams.items = malloc(selected_streams.size);
	
	if (selected_streams.items == NULL) {
		err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	downloaded_streams.size = sizeof(*downloaded_streams.items) * M3U8_STREAM_MAX_SELECTED;
	downloaded_streams.items = malloc(downloaded_streams.size);
	
	if (downloaded_streams.items == NULL) {
		err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	argparser_init(&argparser, argc, argv);
	
	client = &stream.playlist.client;
	
	err =  m3u8httpclient_init(client);
	
	if (err != M3U8ERR_SUCCESS) {
		goto end;
	}
	
	cerror = m3u8httpclient_geterror(client);
	
	while (1) {
		argument = argparser_next(&argparser);
		
		if (argument == NULL) {
			break;
		}
		
		if (strcmp(argument->key, "A") == 0 || strcmp(argument->key, "user-agent") == 0) {
			if (user_agent) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (argument->value == NULL) {
				err = M3U8ERR_CLI_ARGUMENT_VALUE_MISSING;
				goto end;
			}
			
			cerror->code = curl_easy_setopt(client->curl, CURLOPT_USERAGENT, argument->value);
			
			if (cerror->code != CURLE_OK) {
				err = M3U8ERR_CURL_SETOPT_FAILURE;
				goto end;
			}
			
			user_agent = 1;
		} else if (strcmp(argument->key, "x") == 0 || strcmp(argument->key, "proxy") == 0) {
			if (proxy) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (argument->value == NULL) {
				err = M3U8ERR_CLI_ARGUMENT_VALUE_MISSING;
				goto end;
			}
			
			cerror->code = curl_easy_setopt(client->curl, CURLOPT_PROXY, argument->value);
			
			if (cerror->code != CURLE_OK) {
				err = M3U8ERR_CURL_SETOPT_FAILURE;
				goto end;
			}
			
			proxy = 1;
		} else if (strcmp(argument->key, "doh-url") == 0) {
			if (doh_url) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (argument->value == NULL) {
				err = M3U8ERR_CLI_ARGUMENT_VALUE_MISSING;
				goto end;
			}
			
			cerror->code = curl_easy_setopt(client->curl, CURLOPT_DOH_URL, argument->value);
			
			if (cerror->code != CURLE_OK) {
				err = M3U8ERR_CURL_SETOPT_FAILURE;
				goto end;
			}
			
			doh_url = 1;
		} else if (strcmp(argument->key, "e") == 0 || strcmp(argument->key, "referer") == 0) {
			if (referer) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (argument->value == NULL) {
				err = M3U8ERR_CLI_ARGUMENT_VALUE_MISSING;
				goto end;
			}
			
			cerror->code = curl_easy_setopt(client->curl, CURLOPT_REFERER, argument->value);
			
			if (cerror->code != CURLE_OK) {
				err = M3U8ERR_CURL_SETOPT_FAILURE;
				goto end;
			}
			
			referer = 1;
		} else if (strcmp(argument->key, "k") == 0 || strcmp(argument->key, "insecure") == 0) {
			if (insecure) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			cerror->code = curl_easy_setopt(client->curl, CURLOPT_SSL_VERIFYPEER, 0L);
			
			if (cerror->code != CURLE_OK) {
				err = M3U8ERR_CURL_SETOPT_FAILURE;
				goto end;
			}
			
			cerror->code = curl_easy_setopt(client->curl, CURLOPT_DOH_SSL_VERIFYPEER, 0L);
			
			if (cerror->code != CURLE_OK) {
				err = M3U8ERR_CURL_SETOPT_FAILURE;
				goto end;
			}
			
			insecure = 1;
		} else if (strcmp(argument->key, "S") == 0 || strcmp(argument->key, "show-streams") == 0) {
			if (show_formats) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			show_formats = 1;
		} else if (strcmp(argument->key, "u") == 0 || strcmp(argument->key, "url") == 0) {
			if (url != NULL) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (argument->value == NULL) {
				err = M3U8ERR_CLI_ARGUMENT_VALUE_MISSING;
				goto end;
			}
			
			free(url);
			url = NULL;
			
			url = malloc(strlen(argument->value) + 1);
			
			if (url == NULL) {
				err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
				goto end;
			}
			
			strcpy(url, argument->value);
		} else if (strcmp(argument->key, "key") == 0) {
			if (key != NULL) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (argument->value == NULL) {
				err = M3U8ERR_CLI_ARGUMENT_VALUE_MISSING;
				goto end;
			}
			
			free(key);
			key = NULL;
			
			key = malloc(strlen(argument->value) + 1);
			
			if (key == NULL) {
				err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
				goto end;
			}
			
			strcpy(key, argument->value);
		} else if (strcmp(argument->key, "h") == 0 || strcmp(argument->key, "help") == 0) {
			if (show_help) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			show_help = 1;
		} else if (strcmp(argument->key, "v") == 0 || strcmp(argument->key, "version") == 0) {
			if (show_version) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			show_version = 1;
		} else if (strcmp(argument->key, "select-media") == 0) {
			if (argument->value == NULL) {
				err = M3U8ERR_CLI_ARGUMENT_VALUE_MISSING;
				goto end;
			}
			
			if (strcmp(argument->value, "*") == 0) {
				select_all_medias = 1;
				continue;
			}
			
			if (select_all_medias) {
				fprintf(stderr, "+ warning: ignoring select media with index '%s' due to previous wildcard match '*'\n", argument->value);
				continue;
			}
			
			if (!isuint(argument->value)) {
				err = M3U8ERR_PARSER_INVALID_UINT;
				goto end;
			}
			
			value = strtobui(argument->value, NULL, 10);
			
			if (errno == ERANGE) {
				err = M3U8ERR_PARSER_INVALID_UINT;
				goto end;
			}
			
			if (value > 256) {
				err = M3U8ERR_CLI_SELECT_MEDIA_OUT_RANGE;
				goto end;
			}
			
			select_media_index = (size_t) value;
			exists = 0;
			
			for (index = 0; index < selected_medias.offset; index++) {
				exists = (selected_medias.items[index] == select_media_index);
				
				if (exists) {
					break;
				}
			}
			
			if (exists) {
				fprintf(stderr, "+ warning: ignoring duplicate select media with index '%zu'\n", select_media_index);
				continue;
			}
			
			if ((selected_medias.offset + 1) > M3U8_MEDIA_MAX_SELECTED) {
				err = M3U8ERR_CLI_SELECT_MEDIA_MAX_SELECTION_REACHED;
				goto end;
			}
			
			selected_medias.items[selected_medias.offset++] = select_media_index;
		} else if (strcmp(argument->key, "select-stream") == 0) {
			if (select_stream) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (argument->value == NULL) {
				err = M3U8ERR_CLI_ARGUMENT_VALUE_MISSING;
				goto end;
			}
			
			if (strcmp(argument->value, "*") == 0) {
				err = M3U8ERR_CLI_SELECT_STREAM_WILDCARD_UNSUPPORTED;
				goto end;
			}
			
			if (!isuint(argument->value)) {
				err = M3U8ERR_PARSER_INVALID_UINT;
				goto end;
			}
			
			value = strtobui(argument->value, NULL, 10);
			
			if (errno == ERANGE) {
				err = M3U8ERR_PARSER_INVALID_UINT;
				goto end;
			}
			
			select_stream_index = (size_t) value;
			select_stream = 1;
		} else if (strcmp(argument->key, "c") == 0 || strcmp(argument->key, "concurrency") == 0) {
			if (argument->value == NULL) {
				err = M3U8ERR_CLI_ARGUMENT_VALUE_MISSING;
				goto end;
			}
			
			if (!isuint(argument->value)) {
				err = M3U8ERR_PARSER_INVALID_UINT;
				goto end;
			}
			
			value = strtobui(argument->value, NULL, 10);
			
			if (errno == ERANGE) {
				err = M3U8ERR_PARSER_INVALID_UINT;
				goto end;
			}
			
			if (value < 1 || value > 128) {
				err = M3U8ERR_CLI_CONCURRENCY_OUT_RANGE;
				goto end;
			}
			
			download_options.concurrency = (size_t) value;
		} else if (strcmp(argument->key, "retry") == 0 || strcmp(argument->key, "concurrency") == 0) {
			if (argument->value == NULL) {
				err = M3U8ERR_CLI_ARGUMENT_VALUE_MISSING;
				goto end;
			}
			
			if (!isuint(argument->value)) {
				err = M3U8ERR_PARSER_INVALID_UINT;
				goto end;
			}
			
			value = strtobui(argument->value, NULL, 10);
			
			if (errno == ERANGE) {
				err = M3U8ERR_PARSER_INVALID_UINT;
				goto end;
			}
			
			if (value < 1 || value > 128) {
				err = M3U8ERR_CLI_CONCURRENCY_OUT_RANGE;
				goto end;
			}
			
			download_options.retry = (size_t) value;
		} else if (strcmp(argument->key, "o") == 0 || strcmp(argument->key, "output") == 0) {
			if (argument->value == NULL) {
				err = M3U8ERR_CLI_ARGUMENT_VALUE_MISSING;
				goto end;
			}
			
			free(output);
			output = NULL;
			
			output = malloc(strlen(argument->value) + 1);
			
			if (output == NULL) {
				err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
				goto end;
			}
			
			strcpy(output, argument->value);
		} else if (strcmp(argument->key, "debug") == 0) {
			if (debug) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			cerror->code = curl_easy_setopt(client->curl, CURLOPT_VERBOSE, 1L);
			
			if (cerror->code != CURLE_OK) {
				err = M3U8ERR_CURL_SETOPT_FAILURE;
				goto end;
			}
			
			debug = 1;
		} else if (strcmp(argument->key, "disable-autoselection") == 0) {
			if (disable_autoselection) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			disable_autoselection = 1;
		} else {
			err = M3U8ERR_CLI_ARGUMENT_INVALID;
			goto end;
		}
	}
	
	if (output != NULL && show_formats) {
		err = M3U8ERR_CLI_OUTPUT_AND_LIST_STREAMS_USED_TOGETHER;
		goto end;
	}
	
	if (show_help) {
		printf("%s", PROGRAM_HELP);
		goto end;
	}
	
	if (show_version) {
		printf("%s v%s\n", PROGRAM_NAME, PROGRAM_VERSION);
		goto end;
	}
	
	if (url == NULL) {
		err = M3U8ERR_CLI_URI_MISSING;
		goto end;
	}
	
	if (!show_formats && output == NULL) {
		err = M3U8ERR_CLI_OUTPUT_MISSING;
		goto end;
	}
	
	err = m3u8stream_load(&stream, url);
	
	if (err != M3U8ERR_SUCCESS) {
		goto end;
	}
	
	if (show_formats) {
		switch (stream.playlist.type) {
			case M3U8_PLAYLIST_TYPE_MASTER:
				show_master_playlist(&stream);
				break;
			case M3U8_PLAYLIST_TYPE_MEDIA:
				show_media_playlist(&stream);
				break;
		}
		
		goto end;
	}
	
	output_stream = fstream_open(output, FSTREAM_WRITE);
	
	if (output_stream == NULL) {
		err = M3U8ERR_FSTREAM_OPEN_FAILURE;
		goto end;
	}
	
	if (fstream_lock(output_stream) == -1) {
		err = M3U8ERR_FSTREAM_LOCK_FAILURE;
		goto end;
	}
	
	name = expand_filename(output);
	
	if (name == NULL) {
		err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	free(output);
	output = name;
	
	file_extension = get_file_extension(output);
	
	if (file_extension == NULL) {
		err = M3U8ERR_CLI_OUTPUT_MISSING_FILE_EXTENSION;
		goto end;
	}
	
	switch (stream.playlist.type) {
		case M3U8_PLAYLIST_TYPE_MASTER: {
			if (select_stream) {
				size_t stream_index = 0;
				
				if (select_stream_index > stream.offset) {
					err = M3U8ERR_CLI_SELECT_STREAM_OUT_RANGE;
					goto end;
				}
				
				for (index = 0; index < stream.offset; index++) {
					struct M3U8StreamItem* const item = &stream.items[index];
					struct M3U8VariantStream* const variant_stream = item->item;
					
					if (item->type != M3U8_STREAM_VARIANT_STREAM) {
						continue;
					}
					
					if (variant_stream->tag->type != M3U8_TAG_EXT_X_STREAM_INF) {
						continue;
					}
					
					if (stream_index++ != select_stream_index) {
						continue;
					}
					
					selected_streams.items[selected_streams.offset++] = &variant_stream->stream;
					
					break;
				}
				
				if (selected_streams.offset == 0) {
					err = M3U8ERR_CLI_SELECT_STREAM_NO_MATCHING_STREAMS;
					goto end;
				}
			} else {
				if (!disable_autoselection) {
					fprintf(stderr, "+ warning: you did not explicitly select a variant stream; autoselecting the best available\n");
					
					for (index = stream.offset; index-- > 0;) {
						struct M3U8StreamItem* const item = &stream.items[index];
						struct M3U8VariantStream* const variant_stream = item->item;
						
						if (item->type != M3U8_STREAM_VARIANT_STREAM) {
							continue;
						}
						
						if (variant_stream->tag->type != M3U8_TAG_EXT_X_STREAM_INF) {
							continue;
						}
						
						selected_streams.items[selected_streams.offset++] = &variant_stream->stream;
						
						break;
					}
					
					if (selected_streams.offset == 0) {
						err = M3U8ERR_CLI_SELECT_STREAM_NO_AVAILABLE_STREAMS;
						goto end;
					}
				}
			}
			
			if (select_all_medias) {
				for (index = 0; index < stream.offset; index++) {
					struct M3U8StreamItem* const item = &stream.items[index];
					struct M3U8Media* const media = item->item;
					
					if (item->type != M3U8_STREAM_MEDIA) {
						continue;
					}
					
					if (media->type == M3U8_MEDIA_TYPE_CLOSED_CAPTIONS) {
						continue;
					}
					
					if ((selected_streams.offset + 1) > M3U8_STREAM_MAX_SELECTED) {
						err = M3U8ERR_CLI_SELECT_STREAM_MAX_SELECTION_REACHED;
						goto end;
					}
					
					selected_streams.items[selected_streams.offset++] = &media->stream;
				}
			} else {
				for (index = 0; index < selected_medias.offset; index++) {
					const size_t selected_media = selected_medias.items[index];
					size_t media_index = 0;
					
					for (subindex = 0; subindex < stream.offset; subindex++) {
						struct M3U8StreamItem* const item = &stream.items[subindex];
						struct M3U8Media* const media = item->item;
						
						if (item->type != M3U8_STREAM_MEDIA) {
							continue;
						}
						
						if (media_index == selected_media) {
							selected_streams.items[selected_streams.offset++] = &media->stream;
							break;
						}
						
						media_index++;
					}
					
					if (media_index != selected_media) {
						err = M3U8ERR_CLI_SELECT_MEDIA_NO_MATCHING_STREAMS;
						goto end;
					}
				}
				
				/*
				No media have been explicitly selected; let's select the ones attached to
				that variant stream.
				*/
				if (selected_medias.offset == 0 && !disable_autoselection) {
					const size_t offset = selected_streams.offset;
					
					for (index = 0; index < offset; index++) {
						const struct M3U8Stream* const resource = selected_streams.items[index];
						const struct M3U8StreamItem* const item = m3u8stream_finditem(&stream, resource);
						const struct M3U8VariantStream* const variant_stream = item->item;
						
						if (variant_stream->audio != NULL) {
							selected_streams.items[selected_streams.offset++] = &variant_stream->audio->stream;
						}
						
						if (variant_stream->video != NULL) {
							selected_streams.items[selected_streams.offset++] = &variant_stream->video->stream;
						}
						
						if (variant_stream->subtitles != NULL) {
							selected_streams.items[selected_streams.offset++] = &variant_stream->subtitles->stream;
						}
					}
				}
			}
			
			break;
		}
		case M3U8_PLAYLIST_TYPE_MEDIA: {
			if (select_stream || selected_medias.offset > 0) {
				fprintf(stderr, "* warning; this is a media playlist; there is no alternative streams to select\n");
			}
			
			selected_streams.items[selected_streams.offset++] = &stream;
			
			break;
		}
	}
	
	if (key != NULL) {
		for (index = 0; index < stream.offset; index++) {
			struct M3U8StreamItem* const item = &stream.items[index];
			struct M3U8Stream* resource = NULL;
			
			switch (item->type) {
				case M3U8_STREAM_VARIANT_STREAM: {
					resource = &((struct M3U8VariantStream*) item->item)->stream;
					break;
				}
				case M3U8_STREAM_MEDIA: {
					resource = &((struct M3U8Media*) item->item)->stream;
					break;
				}
				default: {
					break;
				}
			}
			
			if (resource == NULL) {
				continue;
			}
			
			for (subindex = 0; subindex < resource->offset; subindex++) {
				struct M3U8StreamItem* const subitem = &resource->items[subindex];
				struct M3U8Segment* const segment = subitem->item;
				struct M3U8Attribute* attribute = NULL;
				
				if (subitem->type != M3U8_STREAM_SEGMENT) {
					continue;
				}
				
				if (segment->key.uri == NULL) {
					continue;
				}
				
				free(segment->key.uri);
				segment->key.uri = NULL;
				
				segment->key.uri = malloc(strlen(key) + 1);
				
				if (segment->key.uri == NULL) {
					err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
					goto end;
				}
				
				strcpy(segment->key.uri, key);
				
				attribute = m3u8tag_igetattr(segment->key.tag, M3U8_ATTRIBUTE_URI);
				attribute->value = segment->key.uri;
				
				break;
			}
		}
	}
	
	for (index = 0; index < selected_streams.offset; index++) {
		const struct M3U8StreamItem* item = NULL;
		
		struct M3U8Stream* const resource = selected_streams.items[index];
		name = malloc(strlen(temporary_directory) + strlen(PATHSEP) + uintlen((biguint_t) resource) + 1 + 4 + 1);
		
		if (name == NULL) {
			err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
			goto end;
		}
		
		strcpy(name, temporary_directory);
		strcat(name, PATHSEP);
		
		wsize = snprintf(name + strlen(name), 4096, "%"FORMAT_BIGGEST_UINT_T, (biguint_t) resource);
		
		if (wsize < 1) {
			err = M3U8ERR_PRINTF_WRITE_FAILURE;
			goto end;
		}
		
		strcat(name, ".m3u8");
		
		printf("- Download Stream #%zu\n", index);
		
		item = m3u8stream_finditem(&stream, resource);
		
		if (item == NULL) {
			show_media_playlist_metadata(resource);
		} else {
			switch (item->type) {
				case M3U8_STREAM_VARIANT_STREAM: {
					show_variant_stream(item);
					break;
				}
				case M3U8_STREAM_MEDIA: {
					show_media(item);
					break;
				}
				default: {
					break;
				}
			}
		}
		
		err = m3u8stream_download(&stream, resource, &download_options);
		
		if (err != M3U8ERR_SUCCESS) {
			goto end;
		}
		
		printf("- Dumping Media Playlist to '%s'\n\n", name);
		
		err = m3u8_dump_file(&resource->playlist, name);
		
		if (err != M3U8ERR_SUCCESS) {
			goto end;
		}
		
		downloaded_streams.items[downloaded_streams.offset++] = name;
		downloaded_streams.items[downloaded_streams.offset] = NULL;
		
		name = NULL;
	}
	
	temporary_file = malloc(strlen(temporary_directory) + strlen(PATHSEP) + uintlen((biguint_t) &stream) + 1 + strlen(file_extension) + 1);
	
	if (temporary_file == NULL) {
		err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	strcpy(temporary_file, temporary_directory);
	strcat(temporary_file, PATHSEP);
	
	wsize = snprintf(temporary_file + strlen(temporary_file), 4096, "%"FORMAT_BIGGEST_UINT_T, (biguint_t) &stream);
	
	if (wsize < 1) {
		err = M3U8ERR_PRINTF_WRITE_FAILURE;
		goto end;
	}
	
	strcat(temporary_file, ".");
	strcat(temporary_file, file_extension);
	
	printf("- Merging media streams into a single file at '%s'\n", temporary_file);
	
	fferr = ffmpeg_mux_streams(downloaded_streams.items, temporary_file);
	
	if (fferr < 0) {
		err = M3U8ERR_FFMPEG_MUXING_FAILURE;
		goto end;
	}
	
	fstream_close(output_stream);
	output_stream = NULL;
	
	printf("- Moving file from '%s' to '%s'\n", temporary_file, output);
	
	if (move_file(temporary_file, output) == -1) {
		err = M3U8ERR_DOWNLOAD_COULD_NOT_MOVE_FILE;
		goto end;
	}
	
	end:;
	
	if (err != M3U8ERR_SUCCESS) {
		const char* const message = m3u8err_getmessage(err);
		fprintf(stderr, "fatal error: (%i) %s", err, message);
		
		switch (err) {
			case M3U8ERR_CURL_REQUEST_FAILURE:
			case M3U8ERR_CURL_SETOPT_FAILURE: {
				fprintf(stderr, ": %s", cerror->message);
				break;
			}
			case M3U8ERR_CLI_ARGUMENT_VALUE_MISSING:
			case M3U8ERR_CLI_DUPLICATE_ARGUMENT: {
				fprintf(stderr, ": %.*s%s", 1 + (strlen(argument->key) > 1), "--", argument->key);
				break;
			}
			case M3U8ERR_CLI_ARGUMENT_INVALID:
				fprintf(stderr, ": %s", argument->key);
				break;
			case M3U8ERR_PARSER_INVALID_UINT: {
				fprintf(stderr, ": %s", argument->value);
				break;
			}
			case M3U8ERR_FFMPEG_MUXING_FAILURE: {
				char* message = ffmpeg_err2str(fferr);
				fprintf(stderr, ": %s", message);
				free(message);
				break;
			}
			case M3U8ERR_CLI_OUTPUT_MISSING_FILE_EXTENSION: {
				fprintf(stderr, ": %s", output);
				break;
			}
		}
		
		fprintf(stderr, "\n");
	}
	
	if (output_stream != NULL) {
		fstream_close(output_stream);
		remove_file(output);
	}
	
	remove_directory(temporary_directory);
	
	m3u8stream_free(&stream);
	m3u8ss_free(&selected_streams);
	m3u8sm_free(&selected_medias);
	m3u8ds_free(&downloaded_streams);
	argparser_free(&argparser);
	m3u8httpclient_errfree(cerror);
	
	free(url);
	free(output);
	free(temporary_directory);
	free(temporary_file);
	free(directory);
	free(name);
	
	sslcerts_unload_certificates();
	
	show_cursor();
	
	return (err == M3U8ERR_SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;
	
}
