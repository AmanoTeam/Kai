#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>

#include <curl/curl.h>
#include <libavutil/error.h>

#include "m3u8stream.h"
#include "m3u8types.h"
#include "m3u8utils.h"
#include "m3u8download.h"
#include "httpclient.h"
#include "errors.h"
#include "m3u8.h"
#include "pathsep.h"
#include "sutils.h"
#include "biggestint.h"
#include "path.h"
#include "filesystem.h"
#include "kai.h"
#include "showformats.h"
#include "callbacks.h"
#include "argparser.h"
#include "program_help.h"
#include "ffmpeg_muxer.h"
#include "ffmpegc_muxer.h"
#include "terminal.h"
#include "sslcerts.h"
#include "resources.h"
#include "signals.h"
#include "clioptions.h"
#include "os.h"
#include "cir.h"
#include "threads.h"
#include "guess_uri.h"
#include "dump.h"

#if defined(_WIN32) && defined(_UNICODE)
	#include "wio.h"
	#define main wmain
#endif

#define M3U8_MEDIA_MAX_SELECTED (100)
#define M3U8_STREAM_MAX_SELECTED (M3U8_MEDIA_MAX_SELECTED + 1)

struct M3U8DownloadedStreams {
	size_t offset;
	size_t size;
	char** items;
};

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
	
	int guess_type = 0;
	
	int exit_code = EXIT_SUCCESS;
	
	thread_t thread = {0};
	int status = 1;
	
	struct HTTPClient* client = NULL;
	struct HTTPClientError* cerror = NULL;
	
	struct M3U8Stream stream = {0};
	
	struct ArgumentParser argparser = {0};
	const struct Argument* argument = NULL;
	
	struct CLIOptions options = {0};
	
	struct CIR cir = {0};
	
	char* temporary_file = NULL;
	char* name = NULL;
	
	const char* file_extension = NULL;
	
	size_t index = 0;
	size_t subindex = 0;
	
	int wsize = 0;
	
	struct M3U8DownloadedStreams downloaded_streams = {0};
	
	#if defined(_WIN32) && defined(_UNICODE)
		wio_setunicode();
	#endif
	
	#if !defined(__HAIKU__)
		status = (
		#if defined(_WIN32)
			!is_wine() &&
		#endif
			is_administrator()
		); 
		
		if (status) {
			err = M3U8ERR_CLI_PRIVILEGED_PROCESS_UNALLOWED;
			goto end;
		}
	#endif
	
	if (resources_increase_maxfd() == -1) {
		fprintf(stderr, "+ warning: could not increase max number of open file descriptors\n");
	}
	
	signal_sethandler(SIGINT, &sigint_handler);
	
	downloaded_streams.size = sizeof(*downloaded_streams.items) * M3U8_STREAM_MAX_SELECTED;
	downloaded_streams.items = malloc(downloaded_streams.size);
	
	if (downloaded_streams.items == NULL) {
		err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	err = argparser_init(&argparser, argc, argv);
	
	if (err != M3U8ERR_SUCCESS) {
		goto end;
	}
	
	client = &stream.playlist.client;
	
	err =  httpclient_init(client);
	
	if (err != M3U8ERR_SUCCESS) {
		goto end;
	}
	
	cerror = httpclient_geterror(client);
	
	err = clioptions_parse(&options, &argparser, &argument, client);
	
	if (err != M3U8ERR_SUCCESS) {
		goto end;
	}
	
	if (options.show_help) {
		printf("%s", PROGRAM_HELP);
		goto end;
	}
	
	if (options.show_version) {
		printf("%s v%s\n", PROGRAM_NAME, PROGRAM_VERSION);
		goto end;
	}
	
	if (options.url == NULL) {
		err = M3U8ERR_CLI_URI_MISSING;
		goto end;
	}
	
	if (!options.disable_cookies) {
		err = multihttpclient_init(&stream.playlist.multi_client, options.download_options.concurrency);
		
		if (err != M3U8ERR_SUCCESS) {
			goto end;
		}
		
		cerror->code = curl_easy_setopt(client->curl, CURLOPT_SHARE, stream.playlist.multi_client.curl_share);
		
		if (cerror->code != CURLE_OK) {
			err = M3U8ERR_CURL_SETOPT_FAILURE;
			goto end;
		}
	}
	
	guess_type = uri_guess_type(options.url);
	
	if (!(guess_type == GUESS_URI_TYPE_URL || guess_type == GUESS_URI_TYPE_LOCAL_FILE)) {
		err = M3U8ERR_LOAD_UNSUPPORTED_URI;
		goto end;
	}
	
	status = !(options.verbose || options.disable_progress || !is_atty(stdout));
	
	if (status) {
		hide_cursor();
		thread_create(&thread, loading_progress_callback, (void*) &status);
	}
	
	err = m3u8stream_load(&stream, options.url, options.base_url);
	
	if (status) {
		status = 0;
		thread_wait(&thread);
		show_cursor();
	}
	
	if (err != M3U8ERR_SUCCESS) {
		goto end;
	}
	
	if (options.dump) {
		dump_playlist(&stream);
		goto end;
	}
	
	if (options.show_formats) {
		show_formats(&stream);
		goto end;
	}
	
	name = expand_filename(options.output);
	
	if (name == NULL) {
		err = M3U8ERR_EXPAND_FILENAME_FAILURE;
		goto end;
	}
	
	free(options.output);
	options.output = name;
	
	name = NULL;
	
	file_extension = get_file_extension(options.output);
	
	if (file_extension == NULL) {
		err = M3U8ERR_CLI_OUTPUT_MISSING_FILE_EXTENSION;
		goto end;
	}
	
	switch (stream.playlist.type) {
		case M3U8_PLAYLIST_TYPE_MASTER: {
			struct M3U8VariantStream* variant_stream = NULL;
			
			if (options.select_stream) {
				switch (options.selected_stream) {
					case SELECT_STREAM_BEST: {
						variant_stream = m3u8stream_getvariant(
							&stream,
							M3U8_SELECT_STREAM_BY_POSITION,
							-1
						);
						
						break;
					}
					case SELECT_STREAM_MEDIUM: {
						const size_t size = m3u8stream_getvarsize(&stream);
						const ssize_t position = size / 2;
						
						variant_stream = m3u8stream_getvariant(
							&stream,
							M3U8_SELECT_STREAM_BY_POSITION,
							position
						);
						
						break;
					}
					case SELECT_STREAM_WORST: {
						variant_stream = m3u8stream_getvariant(
							&stream,
							M3U8_SELECT_STREAM_BY_POSITION,
							0
						);
						
						break;
					}
					default: {
						if (options.selected_stream > SELECT_STREAM_RESOLUTION) {
							const size_t resolution = options.selected_stream - SELECT_STREAM_RESOLUTION;
							
							variant_stream = m3u8stream_getvariant(
								&stream,
								M3U8_SELECT_STREAM_BY_RESOLUTION,
								resolution
							);
						} else {
							const size_t size = m3u8stream_getvarsize(&stream);
							
							if (options.selected_stream > size) {
								err = M3U8ERR_CLI_SELECT_STREAM_OUT_RANGE;
								goto end;
							}
							
							variant_stream = m3u8stream_getvariant(
								&stream,
								M3U8_SELECT_STREAM_BY_POSITION,
								options.selected_stream
							);
						}
						
						break;
					}
				}
				
				if (variant_stream != NULL) {
					options.selected_streams.items[options.selected_streams.offset++] = &variant_stream->stream;
				}
				
				if (options.selected_streams.offset == 0) {
					err = M3U8ERR_CLI_SELECT_STREAM_NO_MATCHING_STREAMS;
					goto end;
				}
			} else {
				if (!options.disable_autoselection) {
					variant_stream = m3u8stream_getvariant(
						&stream,
						M3U8_SELECT_STREAM_BY_POSITION,
						-1
					);
					
					if (variant_stream == NULL) {
						err = M3U8ERR_CLI_SELECT_STREAM_NO_AVAILABLE_STREAMS;
						goto end;
					}
					
					options.selected_streams.items[options.selected_streams.offset++] = &variant_stream->stream;
				}
			}
			
			if (options.all_medias_selected) {
				for (index = 0; index < stream.offset; index++) {
					struct M3U8StreamItem* const item = &stream.items[index];
					struct M3U8Media* const media = item->item;
					
					if (item->type != M3U8_STREAM_MEDIA) {
						continue;
					}
					
					if (media->type == M3U8_MEDIA_TYPE_CLOSED_CAPTIONS) {
						continue;
					}
					
					if ((options.selected_streams.offset + 1) > M3U8_STREAM_MAX_SELECTED) {
						err = M3U8ERR_CLI_SELECT_STREAM_MAX_SELECTION_REACHED;
						goto end;
					}
					
					options.selected_streams.items[options.selected_streams.offset++] = &media->stream;
				}
			} else {
				for (index = 0; index < options.selected_medias.offset; index++) {
					const size_t selected_media = options.selected_medias.items[index];
					size_t media_index = 0;
					
					for (subindex = 0; subindex < stream.offset; subindex++) {
						struct M3U8StreamItem* const item = &stream.items[subindex];
						struct M3U8Media* const media = item->item;
						
						if (item->type != M3U8_STREAM_MEDIA) {
							continue;
						}
						
						if (media_index == selected_media) {
							options.selected_streams.items[options.selected_streams.offset++] = &media->stream;
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
				if (options.selected_medias.offset == 0 && !options.disable_autoselection) {
					const size_t offset = options.selected_streams.offset;
					
					for (index = 0; index < offset; index++) {
						const struct M3U8Stream* const resource = options.selected_streams.items[index];
						const struct M3U8StreamItem* const item = m3u8stream_finditem(&stream, resource);
						const struct M3U8VariantStream* const variant_stream = item->item;
						
						if (variant_stream->audio != NULL) {
							options.selected_streams.items[options.selected_streams.offset++] = &variant_stream->audio->stream;
						}
						
						if (variant_stream->video != NULL) {
							options.selected_streams.items[options.selected_streams.offset++] = &variant_stream->video->stream;
						}
						
						if (variant_stream->subtitles != NULL) {
							options.selected_streams.items[options.selected_streams.offset++] = &variant_stream->subtitles->stream;
						}
					}
				}
			}
			
			break;
		}
		case M3U8_PLAYLIST_TYPE_MEDIA: {
			if (options.select_stream || options.selected_medias.offset > 0) {
				fprintf(stderr, "* warning; this is a media playlist; there is no alternative streams to select\n");
			}
			
			options.selected_streams.items[options.selected_streams.offset++] = &stream;
			
			break;
		}
	}
	
	if (options.key != NULL) {
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
				
				segment->key.uri = malloc(strlen(options.key) + 1);
				
				if (segment->key.uri == NULL) {
					err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
					goto end;
				}
				
				strcpy(segment->key.uri, options.key);
				
				attribute = m3u8tag_igetattr(segment->key.tag, M3U8_ATTRIBUTE_URI);
				attribute->value = segment->key.uri;
				
				break;
			}
		}
	}
	
	if (options.selected_streams.offset == 0) {
		err = M3U8ERR_CLI_NO_STREAMS_SELECTED;
		goto end;
	}
	
	for (index = 0; index < options.selected_streams.offset; index++) {
		const struct M3U8StreamItem* item = NULL;
		struct M3U8Stream* const resource = options.selected_streams.items[index];
		
		if (resource->livestream) {
			err = M3U8ERR_DOWNLOAD_LIVESTREAM_UNSUPPORTED;
			goto end;
		}
		
		name = malloc(strlen(options.download_options.temporary_directory) + strlen(PATHSEP_S) + uintptrlen((uintptr_t) resource) + 1 + 4 + 1);
		
		if (name == NULL) {
			err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
			goto end;
		}
		
		strcpy(name, options.download_options.temporary_directory);
		strcat(name, PATHSEP_S);
		
		wsize = snprintf(name + strlen(name), 4096, "%"FORMAT_UINT_PTR_T, (uintptr_t) resource);
		
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
		
		hide_cursor();
		
		err = m3u8stream_download(&stream, resource, &options.download_options);
		
		show_cursor();
		
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
	
	if (strcmp(file_extension, "null") == 0) {
		file_extension = ffmpeg_guess_extension(downloaded_streams.items);
		
		if (file_extension == NULL) {
			err = M3U8ERR_CLI_CANNOT_DETECT_FORMAT;
			goto end;
		}
		
		remove_file_extension(options.output);
		
		strcat(options.output, ".");
		strcat(options.output, file_extension);
	}
	
	temporary_file = malloc(strlen(options.download_options.temporary_directory) + strlen(PATHSEP_S) + uintptrlen((uintptr_t) &stream) + 1 + strlen(file_extension) + 1);
	
	if (temporary_file == NULL) {
		err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	strcpy(temporary_file, options.download_options.temporary_directory);
	strcat(temporary_file, PATHSEP_S);
	
	wsize = snprintf(
		temporary_file + strlen(temporary_file),
		uintptrlen((uintptr_t) &stream) + 1,
		"%"FORMAT_UINT_PTR_T, (uintptr_t) &stream
	);
	
	if (wsize < 1) {
		err = M3U8ERR_PRINTF_WRITE_FAILURE;
		goto end;
	}
	
	strcat(temporary_file, ".");
	strcat(temporary_file, file_extension);
	
	printf("- Merging media streams into a single file at '%s'\n", temporary_file);
	
	if (options.prefer_ffmpegc) {
		err = ffmpegc_mux_streams(downloaded_streams.items, temporary_file, options.verbose);
		
		if (err != M3U8ERR_SUCCESS) {
			fferr = AVERROR_INVALIDDATA;
			goto end;
		}
	} else {
		fferr = ffmpeg_mux_streams(downloaded_streams.items, temporary_file, options.verbose);
		
		if (fferr < 0) {
			err = M3U8ERR_FFMPEG_MUXING_FAILURE;
			goto end;
		}
	}
	
	if (!options.assume_yes && file_exists(options.output)) {
		cir_init(&cir);
		
		printf("+ Output file already exists. Overwrite? (Y/n) ");
		fflush(stdout);
		
		while (1) {
			const struct CIKey* const key = cir_get(&cir);
			
			switch (key->type) {
				case KEY_CTRL_C:
				case KEY_CTRL_D:
				case KEY_CTRL_BACKSLASH: {
					err = M3U8ERR_CLI_USER_INTERRUPTED;
					goto answer;
				}
				case KEY_Y: {
					printf("%s", key->name);
					goto answer;
				}
				case KEY_N: {
					printf("%s", key->name);
					err = M3U8ERR_CLI_USER_INTERRUPTED;
					goto answer;
				}
				case KEY_ENTER: {
					printf("%s", "y");
					goto answer;
				}
				default: {
					break;
				}
			}
		}
		
		answer:;
		
		fflush(stdout);
		printf("%s", "\r\n");
		
		cir_free(&cir);
		
		if (err == M3U8ERR_CLI_USER_INTERRUPTED) {
			goto end;
		}
	}
	
	printf("- Moving file from '%s' to '%s'\n", temporary_file, options.output);
	
	if (move_file(temporary_file, options.output) == -1) {
		err = M3U8ERR_DOWNLOAD_COULD_NOT_MOVE_FILE;
		goto end;
	}
	
	end:;
	
	if (err != M3U8ERR_SUCCESS && err != M3U8ERR_CLI_USER_INTERRUPTED) {
		const char* const message = m3u8err_getmessage(err);
		fprintf(stderr, "fatal error: (%i) %s", -err, message);
		
		switch (err) {
			case M3U8ERR_CURL_REQUEST_FAILURE:
			case M3U8ERR_CURL_SETOPT_FAILURE: {
				fprintf(stderr, ": %s", cerror->message);
				break;
			}
			case M3U8ERR_CLI_ARGUMENT_VALUE_MISSING:
			case M3U8ERR_CLI_DUPLICATE_ARGUMENT:
			case M3U8ERR_CLI_ARGUMENT_INVALID: {
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
				fprintf(stderr, ": %s", options.output);
				break;
			}
		}
		
		fprintf(stderr, "\n");
	}
	
	if (options.download_options.temporary_directory != NULL) {
		remove_directory(options.download_options.temporary_directory);
	}
	
	m3u8stream_free(&stream);
	m3u8ds_free(&downloaded_streams);
	argparser_free(&argparser);
	httpclient_error_free(cerror);
	
	free(name);
	free(temporary_file);
	
	sslcerts_unload_certificates();
	
	show_cursor();
	cir_free(&cir);
	
	if (err != M3U8ERR_SUCCESS) {
		exit_code = options.return_error_code ? -err : EXIT_FAILURE;
	}
	
	clioptions_free(&options);
	
	return exit_code;
	
}
