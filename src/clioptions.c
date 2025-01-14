#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "errors.h"
#include "os.h"
#include "callbacks.h"
#include "filesystem.h"
#include "kai.h"
#include "m3u8utils.h"
#include "pathsep.h"
#include "argparser.h"
#include "m3u8download.h"
#include "clioptions.h"
#include "guess_uri.h"
#include "sutils.h"

#define CLI_OPTION_CONCURRENCY_MIN 1
#define CLI_OPTION_CONCURRENCY_MAX 128

#define CLI_OPTION_RETRY_MIN 1
#define CLI_OPTION_RETRY_MAX 32

#define CLI_OPTION_OUTPUT_DEFAULT "media.null"

#define CLI_OPTION_MAX_REDIRS_MAX 32

int clioptions_parse(
	struct CLIOptions* const options,
	struct ArgumentParser* const argparser,
	const struct Argument** argument,
	struct HTTPClient* client
) {
	
	int err = M3U8ERR_SUCCESS;
	
	char* temporary_directory = NULL;
	char* directory = NULL;
	
	const struct Argument* arg = NULL;
	
	biguint_t value = 0;
	int exists = 0;
	
	size_t index = 0;
	size_t select_media_index = 0;
	
	struct HTTPClientError* cerror = httpclient_geterror(client);
	
	ssize_t nproc = 0;
	
	temporary_directory = get_temporary_directory();
	
	if (temporary_directory == NULL) {
		err = M3U8ERR_DOWNLOAD_NO_TMPDIR;
		goto end;
	}
	
	directory = malloc(strlen(temporary_directory) + strlen(PATHSEP) + strlen(PROGRAM_NAME) + strlen(PATHSEP) + uintptrlen((uintptr_t) options) + 1);
	
	if (directory == NULL) {
		err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	strcpy(directory, temporary_directory);
	strcat(directory, PATHSEP);
	strcat(directory, PROGRAM_NAME);
	
	free(temporary_directory);
	temporary_directory = directory;
	
	directory = NULL;
	
	options->download_options.concurrency = CLI_OPTION_CONCURRENCY_MIN;
	options->download_options.retry = 0;
	options->download_options.progress_callback = &download_progress_callback;
	options->download_options.temporary_directory = temporary_directory;
	
	nproc = get_nproc();
	
	if (nproc > 0) {
		options->download_options.concurrency = (size_t) nproc;
		
		if (nproc > CLI_OPTION_CONCURRENCY_MAX) {
			options->download_options.concurrency = CLI_OPTION_CONCURRENCY_MAX;
		}
	}
	
	options->selected_medias.size = sizeof(*options->selected_medias.items) * M3U8_MEDIA_MAX_SELECTED;
	options->selected_medias.items = malloc(options->selected_medias.size);
	
	if (options->selected_medias.items == NULL) {
		err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	options->selected_streams.size = sizeof(*options->selected_streams.items) * M3U8_STREAM_MAX_SELECTED;
	options->selected_streams.items = malloc(options->selected_streams.size);
	
	if (options->selected_streams.items == NULL) {
		err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	while (1) {
		arg = argparser_getnext(argparser);
		
		if (arg == NULL) {
			break;
		}
		
		if (strcmp(arg->key, "A") == 0 || strcmp(arg->key, "user-agent") == 0) {
			if (options->user_agent) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (arg->value == NULL) {
				err = M3U8ERR_CLI_ARGUMENT_VALUE_MISSING;
				goto end;
			}
			
			cerror->code = curl_easy_setopt(client->curl, CURLOPT_USERAGENT, arg->value);
			
			if (cerror->code != CURLE_OK) {
				err = M3U8ERR_CURL_SETOPT_FAILURE;
				goto end;
			}
			
			options->user_agent = 1;
		} else if (strcmp(arg->key, "x") == 0 || strcmp(arg->key, "proxy") == 0) {
			if (options->proxy) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (arg->value == NULL) {
				err = M3U8ERR_CLI_ARGUMENT_VALUE_MISSING;
				goto end;
			}
			
			cerror->code = curl_easy_setopt(client->curl, CURLOPT_PROXY, arg->value);
			
			if (cerror->code != CURLE_OK) {
				err = M3U8ERR_CURL_SETOPT_FAILURE;
				goto end;
			}
			
			options->proxy = 1;
		} else if (strcmp(arg->key, "doh-url") == 0) {
			if (options->doh_url) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (arg->value == NULL) {
				err = M3U8ERR_CLI_ARGUMENT_VALUE_MISSING;
				goto end;
			}
			
			cerror->code = curl_easy_setopt(client->curl, CURLOPT_DOH_URL, arg->value);
			
			if (cerror->code != CURLE_OK) {
				err = M3U8ERR_CURL_SETOPT_FAILURE;
				goto end;
			}
			
			options->doh_url = 1;
		} else if (strcmp(arg->key, "e") == 0 || strcmp(arg->key, "referer") == 0) {
			if (options->referer) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (arg->value == NULL) {
				err = M3U8ERR_CLI_ARGUMENT_VALUE_MISSING;
				goto end;
			}
			
			cerror->code = curl_easy_setopt(client->curl, CURLOPT_REFERER, arg->value);
			
			if (cerror->code != CURLE_OK) {
				err = M3U8ERR_CURL_SETOPT_FAILURE;
				goto end;
			}
			
			options->referer = 1;
		} else if (strcmp(arg->key, "k") == 0 || strcmp(arg->key, "insecure") == 0) {
			if (options->insecure) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (arg->value != NULL) {
				err = M3U8ERR_CLI_VALUE_UNEXPECTED;
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
			
			options->insecure = 1;
		} else if (strcmp(arg->key, "S") == 0 || strcmp(arg->key, "show-streams") == 0) {
			if (options->show_formats) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (arg->value != NULL) {
				err = M3U8ERR_CLI_VALUE_UNEXPECTED;
				goto end;
			}
			
			options->show_formats = 1;
		} else if (strcmp(arg->key, "u") == 0 || strcmp(arg->key, "url") == 0) {
			if (options->url != NULL) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (arg->value == NULL) {
				err = M3U8ERR_CLI_ARGUMENT_VALUE_MISSING;
				goto end;
			}
			
			free(options->url);
			options->url = NULL;
			
			options->url = malloc(strlen(arg->value) + 1);
			
			if (options->url == NULL) {
				err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
				goto end;
			}
			
			strcpy(options->url, arg->value);
		} else if (strcmp(arg->key, "base-url") == 0) {
			if (options->base_url != NULL) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (arg->value == NULL) {
				err = M3U8ERR_CLI_ARGUMENT_VALUE_MISSING;
				goto end;
			}
			
			free(options->base_url);
			options->base_url = NULL;
			
			options->base_url = malloc(strlen(arg->value) + 1);
			
			if (options->base_url == NULL) {
				err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
				goto end;
			}
			
			strcpy(options->base_url, arg->value);
		} else if (strcmp(arg->key, "key") == 0) {
			if (options->key != NULL) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (arg->value == NULL) {
				err = M3U8ERR_CLI_ARGUMENT_VALUE_MISSING;
				goto end;
			}
			
			free(options->key);
			options->key = NULL;
			
			options->key = malloc(strlen(arg->value) + 1);
			
			if (options->key == NULL) {
				err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
				goto end;
			}
			
			strcpy(options->key, arg->value);
		} else if (strcmp(arg->key, "h") == 0 || strcmp(arg->key, "help") == 0) {
			if (options->show_help) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (arg->value != NULL) {
				err = M3U8ERR_CLI_VALUE_UNEXPECTED;
				goto end;
			}
			
			options->show_help = 1;
		} else if (strcmp(arg->key, "v") == 0 || strcmp(arg->key, "version") == 0) {
			if (options->show_version) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (arg->value != NULL) {
				err = M3U8ERR_CLI_VALUE_UNEXPECTED;
				goto end;
			}
			
			options->show_version = 1;
		} else if (strcmp(arg->key, "select-media") == 0) {
			if (arg->value == NULL) {
				err = M3U8ERR_CLI_ARGUMENT_VALUE_MISSING;
				goto end;
			}
			
			if (strcmp(arg->value, "*") == 0) {
				options->all_medias_selected = 1;
				continue;
			}
			
			if (options->all_medias_selected) {
				fprintf(stderr, "+ warning: ignoring select media with index '%s' due to previous wildcard match '*'\n", arg->value);
				continue;
			}
			
			if (!isuint(arg->value)) {
				err = M3U8ERR_PARSER_INVALID_UINT;
				goto end;
			}
			
			value = strtobui(arg->value, NULL, 10);
			
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
			
			for (index = 0; index < options->selected_medias.offset; index++) {
				exists = (options->selected_medias.items[index] == select_media_index);
				
				if (exists) {
					break;
				}
			}
			
			if (exists) {
				fprintf(stderr, "+ warning: ignoring duplicate select media with index '%zu'\n", select_media_index);
				continue;
			}
			
			if ((options->selected_medias.offset + 1) > M3U8_MEDIA_MAX_SELECTED) {
				err = M3U8ERR_CLI_SELECT_MEDIA_MAX_SELECTION_REACHED;
				goto end;
			}
			
			options->selected_medias.items[options->selected_medias.offset++] = select_media_index;
		} else if (strcmp(arg->key, "select-stream") == 0) {
			if (options->select_stream) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (arg->value == NULL) {
				err = M3U8ERR_CLI_ARGUMENT_VALUE_MISSING;
				goto end;
			}
			
			if (strcmp(arg->value, "*") == 0) {
				err = M3U8ERR_CLI_SELECT_STREAM_WILDCARD_UNSUPPORTED;
				goto end;
			}
			
			options->selected_stream = 0;
			
			if (strcmp(arg->value, "best") == 0) {
				options->selected_stream = SELECT_STREAM_BEST;
			} else if (strcmp(arg->value, "medium") == 0) {
				options->selected_stream = SELECT_STREAM_MEDIUM;
			} else if (strcmp(arg->value, "worst") == 0) {
				options->selected_stream = SELECT_STREAM_WORST;
			} else {
				const char* const end = strchr(arg->value, '\0') - 1;
				const unsigned char suffix = *end;
				
				if (suffix == 'p' || suffix == 'P') {
					const size_t size = strlen(arg->value);
					char* prefix = NULL;
					
					if (size < 2) {
						err = M3U8ERR_CLI_SELECT_STREAM_RESOLUTION_INVALID;
						goto end;
					}
					
					prefix = malloc(size);
					
					if (prefix == NULL) {
						err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
						goto end;
					}
					
					memcpy(prefix, arg->value, size - 1);
					prefix[size - 1] = '\0';
					
					if (!isuint(prefix)) {
						free(prefix);
						err = M3U8ERR_CLI_SELECT_STREAM_RESOLUTION_INVALID;
						goto end;
					}
					
					value = strtobui(prefix, NULL, 10);
					
					free(prefix);
					
					if (errno == ERANGE) {
						err = M3U8ERR_PARSER_INVALID_UINT;
						goto end;
					}
					
					options->selected_stream = (size_t) (value + SELECT_STREAM_RESOLUTION);
				} else {
					if (!isuint(arg->value)) {
						err = M3U8ERR_PARSER_INVALID_UINT;
						goto end;
					}
					
					value = strtobui(arg->value, NULL, 10);
					
					if (errno == ERANGE) {
						err = M3U8ERR_PARSER_INVALID_UINT;
						goto end;
					}
					
					options->selected_stream = (size_t) value;
				}
			}
			
			options->select_stream = 1;
		} else if (strcmp(arg->key, "c") == 0 || strcmp(arg->key, "concurrency") == 0) {
			if (options->concurrency) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (arg->value == NULL) {
				err = M3U8ERR_CLI_ARGUMENT_VALUE_MISSING;
				goto end;
			}
			
			if (!isuint(arg->value)) {
				err = M3U8ERR_PARSER_INVALID_UINT;
				goto end;
			}
			
			value = strtobui(arg->value, NULL, 10);
			
			if (errno == ERANGE) {
				err = M3U8ERR_PARSER_INVALID_UINT;
				goto end;
			}
			
			if (value < CLI_OPTION_CONCURRENCY_MIN || value > CLI_OPTION_CONCURRENCY_MAX) {
				err = M3U8ERR_CLI_CONCURRENCY_OUT_RANGE;
				goto end;
			}
			
			options->download_options.concurrency = (size_t) value;
			options->concurrency = 1;
		} else if (strcmp(arg->key, "retry") == 0 || strcmp(arg->key, "concurrency") == 0) {
			if (options->retry) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (arg->value == NULL) {
				err = M3U8ERR_CLI_ARGUMENT_VALUE_MISSING;
				goto end;
			}
			
			if (!isuint(arg->value)) {
				err = M3U8ERR_PARSER_INVALID_UINT;
				goto end;
			}
			
			value = strtobui(arg->value, NULL, 10);
			
			if (errno == ERANGE) {
				err = M3U8ERR_PARSER_INVALID_UINT;
				goto end;
			}
			
			if (value < CLI_OPTION_RETRY_MIN || value > CLI_OPTION_RETRY_MAX) {
				err = M3U8ERR_CLI_RETRY_OUT_RANGE;
				goto end;
			}
			
			client->retry = (size_t) value;
			options->download_options.retry = client->retry;
			
			options->retry = 1;
		} else if (strcmp(arg->key, "max-redirs") == 0) {
			if (options->max_redirects) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (arg->value == NULL) {
				err = M3U8ERR_CLI_ARGUMENT_VALUE_MISSING;
				goto end;
			}
			
			if (!isuint(arg->value)) {
				err = M3U8ERR_PARSER_INVALID_UINT;
				goto end;
			}
			
			value = strtobui(arg->value, NULL, 10);
			
			if (errno == ERANGE) {
				err = M3U8ERR_PARSER_INVALID_UINT;
				goto end;
			}
			
			if (value > CLI_OPTION_MAX_REDIRS_MAX) {
				err = M3U8ERR_CLI_MAX_REDIRS_OUT_RANGE;
				goto end;
			}
			
			cerror->code = curl_easy_setopt(client->curl, CURLOPT_FOLLOWLOCATION, (long) value > 0);
			
			if (cerror->code != CURLE_OK) {
				err = M3U8ERR_CURL_SETOPT_FAILURE;
				goto end;
			}
			
			cerror->code = curl_easy_setopt(client->curl, CURLOPT_MAXREDIRS, (long) value);
			
			if (cerror->code != CURLE_OK) {
				err = M3U8ERR_CURL_SETOPT_FAILURE;
				goto end;
			}
			
			options->max_redirects = 1;
		} else if (strcmp(arg->key, "H") == 0 || strcmp(arg->key, "header") == 0) {
			struct curl_slist* tmp = NULL;
			
			if (arg->value == NULL) {
				err = M3U8ERR_CLI_ARGUMENT_VALUE_MISSING;
				goto end;
			}
			
			tmp = curl_slist_append(options->headers, arg->value);
			
			if (tmp == NULL) {
				err = M3U8ERR_CURL_SLIST_FAILURE;
				goto end;
			}
			
			options->headers = tmp;
			
			cerror->code = curl_easy_setopt(client->curl, CURLOPT_HTTPHEADER, options->headers);
			
			if (cerror->code != CURLE_OK) {
				err = M3U8ERR_CURL_SETOPT_FAILURE;
				goto end;
			}
		} else if (strcmp(arg->key, "disable-cookies") == 0) {
			if (options->disable_cookies) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (arg->value != NULL) {
				err = M3U8ERR_CLI_VALUE_UNEXPECTED;
				goto end;
			}
			
			cerror->code = curl_easy_setopt(client->curl, CURLOPT_COOKIEFILE, NULL);
			
			if (cerror->code != CURLE_OK) {
				err = M3U8ERR_CURL_SETOPT_FAILURE;
				goto end;
			}
			
			options->disable_cookies = 1;
		} else if (strcmp(arg->key, "http1.0") == 0) {
			if (options->http10) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (arg->value != NULL) {
				err = M3U8ERR_CLI_VALUE_UNEXPECTED;
				goto end;
			}
			
			cerror->code = curl_easy_setopt(client->curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
			
			if (cerror->code != CURLE_OK) {
				err = M3U8ERR_CURL_SETOPT_FAILURE;
				goto end;
			}
			
			options->http10 = 1;
		} else if (strcmp(arg->key, "http1.1") == 0) {
			if (options->http11) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (arg->value != NULL) {
				err = M3U8ERR_CLI_VALUE_UNEXPECTED;
				goto end;
			}
			
			cerror->code = curl_easy_setopt(client->curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
			
			if (cerror->code != CURLE_OK) {
				err = M3U8ERR_CURL_SETOPT_FAILURE;
				goto end;
			}
			
			options->http11 = 1;
		} else if (strcmp(arg->key, "http2") == 0) {
			if (options->http2) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (arg->value != NULL) {
				err = M3U8ERR_CLI_VALUE_UNEXPECTED;
				goto end;
			}
			
			cerror->code = curl_easy_setopt(client->curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
			
			if (cerror->code != CURLE_OK) {
				err = M3U8ERR_CURL_SETOPT_FAILURE;
				goto end;
			}
			
			options->http2 = 1;
		} else if (strcmp(arg->key, "o") == 0 || strcmp(arg->key, "output") == 0) {
			if (arg->value == NULL) {
				err = M3U8ERR_CLI_ARGUMENT_VALUE_MISSING;
				goto end;
			}
			
			free(options->output);
			options->output = NULL;
			
			options->output = malloc(strlen(arg->value) + 1);
			
			if (options->output == NULL) {
				err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
				goto end;
			}
			
			strcpy(options->output, arg->value);
		} else if (strcmp(arg->key, "randomized-temporary-directory") == 0) {
			int wsize = 0;
			
			if (options->randomized_temporary_directory) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (arg->value != NULL) {
				err = M3U8ERR_CLI_VALUE_UNEXPECTED;
				goto end;
			}
			
			strcat(temporary_directory, PATHSEP);
			
			wsize = snprintf(
				temporary_directory + strlen(temporary_directory),
				uintptrlen((uintptr_t) options) + 1,
				"%"FORMAT_UINT_PTR_T, (uintptr_t) options
			);
			
			if (wsize < 1) {
				err = M3U8ERR_PRINTF_WRITE_FAILURE;
				goto end;
			}
			
			options->randomized_temporary_directory = 1;
		} else if (strcmp(arg->key, "verbose") == 0) {
			if (options->verbose) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (arg->value != NULL) {
				err = M3U8ERR_CLI_VALUE_UNEXPECTED;
				goto end;
			}
			
			cerror->code = curl_easy_setopt(client->curl, CURLOPT_VERBOSE, 1L);
			
			if (cerror->code != CURLE_OK) {
				err = M3U8ERR_CURL_SETOPT_FAILURE;
				goto end;
			}
			
			options->verbose = 1;
		} else if (strcmp(arg->key, "y") == 0 || strcmp(arg->key, "assume-yes") == 0) {
			if (options->assume_yes) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (arg->value != NULL) {
				err = M3U8ERR_CLI_VALUE_UNEXPECTED;
				goto end;
			}
			
			options->assume_yes = 1;
		} else if (strcmp(arg->key, "prefer-ffmpeg-cli") == 0) {
			if (options->prefer_ffmpegc) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (arg->value != NULL) {
				err = M3U8ERR_CLI_VALUE_UNEXPECTED;
				goto end;
			}
			
			options->prefer_ffmpegc = 1;
		} else if (strcmp(arg->key, "dump") == 0) {
			if (options->dump) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (arg->value != NULL) {
				err = M3U8ERR_CLI_VALUE_UNEXPECTED;
				goto end;
			}
			
			options->dump = 1;
		} else if (strcmp(arg->key, "return-error-code") == 0) {
			if (options->return_error_code) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (arg->value != NULL) {
				err = M3U8ERR_CLI_VALUE_UNEXPECTED;
				goto end;
			}
			
			options->return_error_code = 1;
		} else if (strcmp(arg->key, "disable-autoselection") == 0) {
			if (options->disable_autoselection) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (arg->value != NULL) {
				err = M3U8ERR_CLI_VALUE_UNEXPECTED;
				goto end;
			}
			
			options->disable_autoselection = 1;
		} else if (strcmp(arg->key, "disable-progress-meter") == 0) {
			if (options->disable_progress) {
				err = M3U8ERR_CLI_DUPLICATE_ARGUMENT;
				goto end;
			}
			
			if (arg->value != NULL) {
				err = M3U8ERR_CLI_VALUE_UNEXPECTED;
				goto end;
			}
			
			options->download_options.progress_callback = NULL;
			
			options->disable_progress = 1;
		} else {
			err = M3U8ERR_CLI_ARGUMENT_INVALID;
			goto end;
		}
	}
	
	if (options->output == NULL) {
		options->output = malloc(strlen(CLI_OPTION_OUTPUT_DEFAULT) + 1);
		
		if (options->output == NULL) {
			err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
			goto end;
		}
		
		strcpy(options->output, CLI_OPTION_OUTPUT_DEFAULT);
	}
	
	if (create_directory(temporary_directory) != 0) {
		err = M3U8ERR_DOWNLOAD_COULD_NOT_CREATE_TMPDIR;
		goto end;
	}
	
	end:;
	
	if (err != M3U8ERR_SUCCESS) {
		clioptions_free(options);
	}
	
	*argument = arg;
	
	return err;
	
}

void clioptions_free(struct CLIOptions* const options) {
	
	/* Selected streams */
	free(options->selected_streams.items);
	options->selected_streams.items = NULL;
	
	options->selected_streams.offset = 0;
	options->selected_streams.size = 0;
	
	/* Selected medias */
	free(options->selected_medias.items);
	options->selected_medias.items = NULL;
	
	options->selected_medias.offset = 0;
	options->selected_medias.size = 0;
	
	/* cURL headers list */
	curl_slist_free_all(options->headers);
	options->headers = NULL;
	
	/* Options and flags */
	options->show_formats = 0;
	options->show_help = 0;
	options->show_version = 0;
	options->select_stream = 0;
	options->user_agent = 0;
	options->proxy = 0;
	options->doh_url = 0;
	options->referer = 0;
	options->insecure = 0;
	options->disable_cookies = 0;
	options->verbose = 0;
	options->assume_yes = 0;
	options->disable_autoselection = 0;
	options->disable_progress = 0;
	options->prefer_ffmpegc = 0;
	options->dump = 0;
	options->all_medias_selected = 0;
	options->http10 = 0;
	options->http11 = 0;
	options->http2 = 0;
	options->randomized_temporary_directory = 0;
	options->max_redirects = 0;
	options->concurrency = 0;
	options->retry = 0;
	
	free(options->url);
	options->url = NULL;
	
	free(options->base_url);
	options->base_url = NULL;
	
	free(options->key);
	options->key = NULL;
	
	free(options->output);
	options->output = NULL;
	
	/* Download options */
	options->download_options.concurrency = 0;
	options->download_options.retry = 0;
	options->download_options.progress_callback = NULL;
	
	free(options->download_options.temporary_directory);
	options->download_options.temporary_directory = NULL;
	
}
