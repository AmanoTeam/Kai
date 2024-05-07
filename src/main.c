#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <curl/curl.h>

#include "m3u8stream.h"
#include "m3u8types.h"
#include "m3u8utils.h"
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
#include "biggestint.h"

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

void progress_callback(const size_t total, const size_t current) {
	
	erase_line();
	printf("+ Fetching media segments: %zu out of %zu\r", current, total);
	fflush(stdout);
	
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

static void show_media_playlist_metadata(const struct M3U8Stream* const stream) {
	
	const bigfloat_t duration = m3u8stream_getduration(stream);
	const biguint_t segments = m3u8stream_getsegments(stream);
	const bigfloat_t average_duration = duration / (segments > 0 ? segments : 1);
	
	printf("  Metadata:");
	
	printf(
		"\n    Duration: %02"FORMAT_BIGGEST_UINT_T":%02"FORMAT_BIGGEST_UINT_T":%02"FORMAT_BIGGEST_UINT_T,
		((biguint_t) duration / 3600),
		(((biguint_t) duration % 3600) / 60),
		(((biguint_t) duration % 3600) % 60)
	);
	
	printf("\n    Segments: %"FORMAT_BIGGEST_UINT_T" (~%"FORMAT_BIGGEST_UINT_T" seconds)", segments, (biguint_t) average_duration);
	
	printf("\n\n");
	
}

static void show_media_playlist(const struct M3U8Stream* const stream) {
	
	printf("Playlist #0\n  Metadata:\n    Type: Media Playlist\n\n");
	
	printf("Media #0\n");
	
	show_media_playlist_metadata(stream);
	
}

static void show_media(const struct M3U8StreamItem* const item) {
	
	const struct M3U8Media* const media = (struct M3U8Media*) item->item;
	
	const char* media_type = NULL;
	
	printf("  Metadata:");
	
	switch (media->type) {
		case M3U8_MEDIA_TYPE_AUDIO:
			media_type = "Audio";
			break;
		case M3U8_MEDIA_TYPE_VIDEO:
			media_type = "Video";
			break;
		case M3U8_MEDIA_TYPE_SUBTITLES:
			media_type = "Subtitles";
			break;
		case M3U8_MEDIA_TYPE_CLOSED_CAPTIONS:
			media_type = "Closed captions";
			break;
	}
	
	printf("\n    Type: %s", media_type);
	printf("\n    Identification: %s", media->group_id);
	printf("\n    Name: %s", media->name);
	
	if (media->language) {
		printf("\n    Language: %s", media->language);
	}
	
	if (media->assoc_language) {
		printf("\n    Associated language: %s", media->assoc_language);
	}
	
	printf(
		"\n    Options: (default = %s, autoselect = %s, forced = %s)",
		(media->default_ ? "yes" : "no"),
		(media->autoselect ? "yes" : "no"),
		(media->forced ? "yes" : "no")
	);
	
	if (media->duration) {
		printf(
			"\n    Duration: %02"FORMAT_BIGGEST_UINT_T":%02"FORMAT_BIGGEST_UINT_T":%02"FORMAT_BIGGEST_UINT_T,
			((biguint_t) media->duration / 3600),
			(((biguint_t) media->duration % 3600) / 60),
			(((biguint_t) media->duration % 3600) % 60)
		);
	}
	
	if (media->characteristics) {
		printf("\n    Characteristics: %s", media->characteristics);
	}
	
	if (media->channels) {
		printf("\n    Channels: %s", media->channels);
	}
	
	if (media->segments) {
		printf("\n    Segments: %"FORMAT_BIGGEST_UINT_T" (~%"FORMAT_BIGGEST_UINT_T" seconds)", media->segments, (biguint_t) media->average_duration);
	}
	
	printf("\n\n");
	
}

static void show_variant_stream(const struct M3U8StreamItem* const item) {
	
	const struct M3U8VariantStream* const variant_stream = (struct M3U8VariantStream*) item->item;
	
	printf("  Metadata:");
	
	printf("\n    Bandwidth: %"FORMAT_BIGGEST_UINT_T, variant_stream->bandwidth);
	
	if (variant_stream->average_bandwidth) {
		printf(" (~%"FORMAT_BIGGEST_UINT_T")", variant_stream->average_bandwidth);
	}
	
	if (variant_stream->codecs) {
		printf("\n    Codecs: %s", variant_stream->codecs);
	}
	
	if (variant_stream->resolution.width && variant_stream->resolution.height) { 
		printf("\n    Resolution: %"FORMAT_BIGGEST_UINT_T"x%"FORMAT_BIGGEST_UINT_T, variant_stream->resolution.width, variant_stream->resolution.height);
	}
	
	if (variant_stream->frame_rate) {
		printf("\n    Frame rate: %"FORMAT_BIGGEST_UINT_T, (biguint_t) variant_stream->frame_rate);
	}
	
	if (variant_stream->duration) {
		printf(
			"\n    Duration: %02"FORMAT_BIGGEST_UINT_T":%02"FORMAT_BIGGEST_UINT_T":%02"FORMAT_BIGGEST_UINT_T,
			((biguint_t) variant_stream->duration / 3600),
			(((biguint_t) variant_stream->duration % 3600) / 60),
			(((biguint_t) variant_stream->duration % 3600) % 60)
		);
	}
	
	if (variant_stream->audio) {
		printf("\n    Audio: %s (ID = \"%s\")", variant_stream->audio->name, variant_stream->audio->group_id);
	}
	
	if (variant_stream->video) {
		printf("\n    Video: %s (ID = \"%s\")", variant_stream->video->name, variant_stream->video->group_id);
	}
	
	if (variant_stream->subtitles) {
		printf("\n    Subtitles: %s (ID = \"%s\")", variant_stream->subtitles->name, variant_stream->subtitles->group_id);
	}
	
	if (variant_stream->closed_captions) {
		printf("\n    Closed captions: %s (ID = \"%s\")", variant_stream->closed_captions->name, variant_stream->closed_captions->group_id);
	}
	
	if (variant_stream->segments) {
		printf("\n    Segments: %"FORMAT_BIGGEST_UINT_T" (~%"FORMAT_BIGGEST_UINT_T" seconds)", variant_stream->segments, (biguint_t) variant_stream->average_duration);
	}
	
	if (variant_stream->size) {
		char format[BTOS_MAX_SIZE];
		btos(variant_stream->size, format);
		
		printf("\n    Size: ~%s", format);
	}
	
	printf("\n\n");
	
}

static void show_master_playlist(const struct M3U8Stream* const stream) {
	
	size_t index = 0;
	 
	size_t variant_stream_index = 0;
	size_t media_index = 0;
	
	printf("Playlist #0\n  Metadata:\n    Type: Master Playlist\n\n");
	
	for (index = 0; index < stream->offset; index++) {
		const struct M3U8StreamItem* const item = &stream->items[index];
		
		switch (item->type) {
			case M3U8_STREAM_MEDIA: {
				printf("Media #%zu\n", media_index++);
				
				show_media(item);
				
				break;
			}
			case M3U8_STREAM_VARIANT_STREAM: {
				const struct M3U8VariantStream* const variant_stream = (struct M3U8VariantStream*) item->item;
				
				if (variant_stream->tag->type != M3U8_TAG_EXT_X_STREAM_INF) {
					break;
				}
				
				printf("Variant Stream #%zu\n", variant_stream_index++);
				
				show_variant_stream(item);
				
				break;
			}
			default: {
				break;
			}
		}
	}
	
}

int main(int argc, argv_t* argv[]) {
	
	int err = M3U8ERR_SUCCESS;
	int fferr = 0;
	
	struct M3U8HTTPClient* client = NULL;
	
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
	
	int select_all_medias = 0;
	int exists = 0;
	
	char* url = NULL;
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
		.progress_callback = &progress_callback
	};
	
	struct M3U8SelectedMedias selected_medias = {0};
	struct M3U8SelectedStreams selected_streams = {0};
	struct M3U8DownloadedStreams downloaded_streams = {0};
	
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
			
			client->curl_code = curl_easy_setopt(client->curl, CURLOPT_USERAGENT, argument->value);
			
			if (client->curl_code != CURLE_OK) {
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
			
			client->curl_code = curl_easy_setopt(client->curl, CURLOPT_PROXY, argument->value);
			
			if (client->curl_code != CURLE_OK) {
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
			
			client->curl_code = curl_easy_setopt(client->curl, CURLOPT_DOH_URL, argument->value);
			
			if (client->curl_code != CURLE_OK) {
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
			
			client->curl_code = curl_easy_setopt(client->curl, CURLOPT_REFERER, argument->value);
			
			if (client->curl_code != CURLE_OK) {
				err = M3U8ERR_CURL_SETOPT_FAILURE;
				goto end;
			}
			
			referer = 1;
		} else if (strcmp(argument->key, "k") == 0 || strcmp(argument->key, "insecure") == 0) {
			if (insecure) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			client->curl_code = curl_easy_setopt(client->curl, CURLOPT_SSL_VERIFYPEER, 0L);
			
			if (client->curl_code != CURLE_OK) {
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
	
	if (output == NULL) {
		err = M3U8ERR_CLI_OUTPUT_MISSING;
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
				fprintf(stderr, "+ warning: you did not explicitly select a variant stream; auto selecting the best available\n");
				
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
		
	for (index = 0; index < selected_streams.offset; index++) {
		struct M3U8Stream* const substream = selected_streams.items[index];
		name = malloc(strlen(temporary_directory) + strlen(PATHSEP) + uintlen((biguint_t) substream) + 1 + 4 + 1);
		
		if (name == NULL) {
			err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
			goto end;
		}
		
		strcpy(name, temporary_directory);
		strcat(name, PATHSEP);
		
		wsize = snprintf(name + strlen(name), 4096, "%"FORMAT_BIGGEST_UINT_T, (biguint_t) substream);
		
		if (wsize < 1) {
			err = M3U8ERR_PRINTF_WRITE_FAILURE;
			goto end;
		}
		
		strcat(name, ".m3u8");
		
		printf("- Download Stream #%zu\n", index);
		
		switch (stream.playlist.type) {
			case M3U8_PLAYLIST_TYPE_MASTER: {
				
				for (subindex = 0; subindex < stream.offset; subindex++) {
					const struct M3U8StreamItem* const item = &stream.items[subindex];
					
					switch (item->type) {
						case M3U8_STREAM_VARIANT_STREAM: {
							const struct M3U8VariantStream* const variant_stream = item->item;
							
							if (&variant_stream->stream != substream) {
								break;
							}
							
							show_variant_stream(item);
							
							break;
						}
						case M3U8_STREAM_MEDIA: {
							const struct M3U8Media* const media = item->item;
							
							if (&media->stream != substream) {
								break;
							}
							
							show_media(item);
							
							break;
						}
						default: {
							break;
						}
					}
				}
				
				break;
			}
			case M3U8_PLAYLIST_TYPE_MEDIA: {
				show_media_playlist_metadata(substream);
				break;
			}
		}
		
		err = m3u8stream_download(&stream, substream, &download_options);
		
		if (err != M3U8ERR_SUCCESS) {
			goto end;
		}
		
		printf("- Dumping Media Playlist to '%s'\n\n", name);
		
		err = m3u8_dump_file(&substream->playlist, name);
		
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
				fprintf(stderr, ": %s", client->curl_error_message);
				break;
			}
			case M3U8ERR_CLI_ARGUMENT_VALUE_MISSING:
			case M3U8ERR_CLI_DUPLICATE_ARGUMENT: {
				fprintf(stderr, ": %.*s%s", 1 + (strlen(argument->key) > 1), "--", argument->key);
				break;
			}
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
	
	remove_directory(temporary_directory);
	
	m3u8stream_free(&stream);
	m3u8ss_free(&selected_streams);
	m3u8sm_free(&selected_medias);
	m3u8ds_free(&downloaded_streams);
	argparser_free(&argparser);
	
	free(url);
	free(output);
	free(temporary_directory);
	free(temporary_file);
	free(directory);
	free(name);
	
	show_cursor();
	
}
	