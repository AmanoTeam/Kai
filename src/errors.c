/*
This file is auto-generated. Use the tool at ../tools/errors.h.py to regenerate.
*/

#include "errors.h"

const char* m3u8err_getmessage(const int code) {
	
	switch (code) {
		case M3U8ERR_SUCCESS:
			return "Success";
		case M3U8ERR_ATTRIBUTE_DUPLICATE:
			return "There must not be multiple attributes with the same name within the same tag";
		case M3U8ERR_ATTRIBUTE_EMPTY:
			return "An M3U8 tag must not contain empty attributes";
		case M3U8ERR_ATTRIBUTE_INVALID_BRANGE:
			return "The value of this M3U8 attribute is not a valid byte range";
		case M3U8ERR_ATTRIBUTE_INVALID_DTIME:
			return "The value of this M3U8 attribute is not a valid ISO/IEC 8601:2004 date/time representation";
		case M3U8ERR_ATTRIBUTE_INVALID_ESTRING:
			return "The value of this M3U8 attribute is not a valid enumerated string";
		case M3U8ERR_ATTRIBUTE_INVALID_FLOAT:
			return "The value of this M3U8 attribute is not a valid signed decimal floating point number";
		case M3U8ERR_ATTRIBUTE_INVALID_HEXSEQ:
			return "The value of this M3U8 attribute is not a valid hexadecimal sequence";
		case M3U8ERR_ATTRIBUTE_INVALID_NAME:
			return "The name of this M3U8 attribute is invalid or was not recognized";
		case M3U8ERR_ATTRIBUTE_INVALID_QSTRING:
			return "The value of this M3U8 attribute is not a valid quoted string";
		case M3U8ERR_ATTRIBUTE_INVALID_RESOLUTION:
			return "The value of this M3U8 attribute is not a valid decimal resolution";
		case M3U8ERR_ATTRIBUTE_INVALID_UFLOAT:
			return "The value of this M3U8 attribute is not a valid decimal floating point number";
		case M3U8ERR_ATTRIBUTE_INVALID_UINT:
			return "The value of this M3U8 attribute is not a valid decimal integer";
		case M3U8ERR_ATTRIBUTE_KEYFORMATVERSIONS_INVALID:
			return "The value of this M3U8 attribute is not a valid KEYFORMATVERSIONS";
		case M3U8ERR_ATTRIBUTE_MISSING:
			return "This M3U8 tag is missing a required M3U8 attribute";
		case M3U8ERR_ATTRIBUTE_MISSING_NAME:
			return "An M3U8 attribute requires a key to be supplied";
		case M3U8ERR_ATTRIBUTE_MISSING_VALUE:
			return "An M3U8 attribute requires a value to be supplied";
		case M3U8ERR_ATTRIBUTE_UNEXPECTED:
			return "Got an unexpected M3U8 attribute within this M3U8 tag";
		case M3U8ERR_ATTRIBUTE_VALUE_INVALID:
			return "The value of this M3U8 attribute is invalid or was not recognized";
		case M3U8ERR_ATTRIBUTE_WRONG_END_DATE:
			return "The END-DATE attribute value do not match the value of the DURATION attribute value";
		case M3U8ERR_BUFFER_OVERFLOW:
			return "Writing data to that memory address would cause a buffer overflow";
		case M3U8ERR_CALLBACK_WRITE_FAILURE:
			return "Callback returned error";
		case M3U8ERR_CLI_ARGUMENT_EMPTY:
			return "Got an empty argument while parsing the command-line arguments";
		case M3U8ERR_CLI_ARGUMENT_INVALID:
			return "This argument is invalid or was not recognized";
		case M3U8ERR_CLI_ARGUMENT_VALUE_MISSING:
			return "This keyword argument require a value to be supplied";
		case M3U8ERR_CLI_CANNOT_DETECT_FORMAT:
			return "Could not guess output file format; either the codec is not supported or the media file is corrupt";
		case M3U8ERR_CLI_CONCURRENCY_OUT_RANGE:
			return "The value specified for the keyword argument -c/--concurrency is out of range";
		case M3U8ERR_CLI_DUPLICATE_ARGUMENT:
			return "This argument cannot be specified multiple times";
		case M3U8ERR_CLI_MAX_REDIRS_OUT_RANGE:
			return "The value specified for the keyword argument --max-redirs is out of range";
		case M3U8ERR_CLI_NO_STREAMS_SELECTED:
			return "No streams selected";
		case M3U8ERR_CLI_OUTPUT_MISSING:
			return "No output file specified";
		case M3U8ERR_CLI_OUTPUT_MISSING_FILE_EXTENSION:
			return "The output filename does not contain a file extension";
		case M3U8ERR_CLI_PRIVILEGED_PROCESS_UNALLOWED:
			return "You are not supposed to run this program as a privileged user. Sorry.";
		case M3U8ERR_CLI_RETRY_OUT_RANGE:
			return "The value specified for the keyword argument -r/--retry is out of range";
		case M3U8ERR_CLI_SELECT_MEDIA_MAX_SELECTION_REACHED:
			return "Reached the maximum number of allowed media selection";
		case M3U8ERR_CLI_SELECT_MEDIA_NO_MATCHING_STREAMS:
			return "Could not find any media stream matching the requested index";
		case M3U8ERR_CLI_SELECT_MEDIA_OUT_RANGE:
			return "The value specified for the keyword argument --select-media is out of range";
		case M3U8ERR_CLI_SELECT_STREAM_MAX_SELECTION_REACHED:
			return "Reached the maximum number of allowed stream selection";
		case M3U8ERR_CLI_SELECT_STREAM_NO_AVAILABLE_STREAMS:
			return "This playlist does not contain any Variant Stream";
		case M3U8ERR_CLI_SELECT_STREAM_NO_MATCHING_STREAMS:
			return "Could not find any Variant Stream matching the requested resolution or position";
		case M3U8ERR_CLI_SELECT_STREAM_OUT_RANGE:
			return "The value specified for the keyword argument --select-stream is out of range";
		case M3U8ERR_CLI_SELECT_STREAM_RESOLUTION_INVALID:
			return "The value specified for the keyword argument --select-stream is not a valid resolution or position";
		case M3U8ERR_CLI_SELECT_STREAM_WILDCARD_UNSUPPORTED:
			return "The keyword argument --select-stream does not support wildcard matching";
		case M3U8ERR_CLI_URI_MISSING:
			return "No URI specified";
		case M3U8ERR_CLI_USER_INTERRUPTED:
			return "User interrupted";
		case M3U8ERR_CLI_VALUE_UNEXPECTED:
			return "Got an unexpected value while parsing the command-line arguments";
		case M3U8ERR_CURLE_GET_INFO_FAILURE:
			return "Could not get information about the cURL handler";
		case M3U8ERR_CURLM_ADD_FAILURE:
			return "Could not add the cURL handler to cURL multi";
		case M3U8ERR_CURLM_INIT_FAILURE:
			return "Could not initialize the cURL multi interface";
		case M3U8ERR_CURLM_PERFORM_FAILURE:
			return "Could not perform on cURL multi";
		case M3U8ERR_CURLM_POLL_FAILURE:
			return "Could not poll on cURL multi";
		case M3U8ERR_CURLM_REMOVE_FAILURE:
			return "Could not remove the cURL handler from cURL multi";
		case M3U8ERR_CURLM_SETOPT_FAILURE:
			return "Could not set options on cURL multi";
		case M3U8ERR_CURLSH_INIT_FAILURE:
			return "Could not initialize the cURL Share interface";
		case M3U8ERR_CURLSH_SETOPT_FAILURE:
			return "Could not set options on Share HTTP client";
		case M3U8ERR_CURLU_INIT_FAILURE:
			return "Could not initialize the cURL URL interface";
		case M3U8ERR_CURLU_URL_GET_FAILURE:
			return "Could not get URL from this cURL URL interface";
		case M3U8ERR_CURLU_URL_SET_FAILURE:
			return "Could not set URL for this cURL URL interface";
		case M3U8ERR_CURL_INIT_FAILURE:
			return "Could not initialize the HTTP client due to an unexpected error";
		case M3U8ERR_CURL_REQUEST_FAILURE:
			return "HTTP request failure";
		case M3U8ERR_CURL_SETOPT_FAILURE:
			return "Could not set options on HTTP client";
		case M3U8ERR_CURL_SLIST_FAILURE:
			return "Could not append item to list";
		case M3U8ERR_DOWNLOAD_COULD_NOT_CREATE_TMPDIR:
			return "Could not create the temporary directory";
		case M3U8ERR_DOWNLOAD_COULD_NOT_MOVE_FILE:
			return "Could not move file to specified location";
		case M3U8ERR_DOWNLOAD_LIVESTREAM_UNSUPPORTED:
			return "Downloading media streams from live-streaming playlists is not supported";
		case M3U8ERR_DOWNLOAD_NO_TMPDIR:
			return "Could not find a suitable directory for storing temporary files";
		case M3U8ERR_EXPAND_FILENAME_FAILURE:
			return "Could not resolve filename";
		case M3U8ERR_FFMPEG_COMMAND_NOT_FOUND:
			return "Could locate the FFmpeg executable";
		case M3U8ERR_FFMPEG_MUXING_FAILURE:
			return "Could not mux media streams";
		case M3U8ERR_FSTREAM_LOCK_FAILURE:
			return "Could not lock file";
		case M3U8ERR_FSTREAM_OPEN_FAILURE:
			return "Could not open file";
		case M3U8ERR_FSTREAM_READ_EMPTY_FILE:
			return "Tried to read contents from an empty file";
		case M3U8ERR_FSTREAM_READ_FAILURE:
			return "Could not read data from file";
		case M3U8ERR_FSTREAM_SEEK_FAILURE:
			return "Could not seek file";
		case M3U8ERR_FSTREAM_TELL_FAILURE:
			return "Could not get current file position";
		case M3U8ERR_FSTREAM_WRITE_FAILURE:
			return "Could not write data to file";
		case M3U8ERR_GET_APP_FILENAME_FAILURE:
			return "Could not get app filename";
		case M3U8ERR_ITEM_EMPTY:
			return "This M3U8 tag must not contain empty items";
		case M3U8ERR_ITEM_INVALID_BRANGE:
			return "The value of this M3U8 item is not a valid byte range";
		case M3U8ERR_ITEM_INVALID_DTIME:
			return "The value of this M3U8 item is not a valid ISO/IEC 8601:2004 date/time representation";
		case M3U8ERR_ITEM_INVALID_ESTRING:
			return "The value of this M3U8 item is not a valid enumerated string";
		case M3U8ERR_ITEM_INVALID_UFLOAT:
			return "The value of this M3U8 attribute is not a valid decimal floating point number";
		case M3U8ERR_ITEM_INVALID_UINT:
			return "The value of this M3U8 attribute is not a valid decimal integer";
		case M3U8ERR_ITEM_INVALID_USTRING:
			return "The value of this M3U8 item is not a valid unquoted string";
		case M3U8ERR_ITEM_MISSING:
			return "This M3U8 tag is missing a required M3U8 item";
		case M3U8ERR_ITEM_VALUE_INVALID:
			return "The value of this M3U8 item is invalid or was not recognized";
		case M3U8ERR_ITEM_VALUE_TOO_LONG:
			return "The value of this M3U8 item is too long";
		case M3U8ERR_LOAD_UNSUPPORTED_URI:
			return "Could not load M3U8 playlist from this URI; either this protocol is not supported or it was not recognized";
		case M3U8ERR_MEDIA_NO_MATCHING_AUDIO:
			return "Could not find any audio stream matching this variant stream";
		case M3U8ERR_MEDIA_NO_MATCHING_CLOSED_CAPTIONS:
			return "Could not find any closed-captions stream matching this variant stream";
		case M3U8ERR_MEDIA_NO_MATCHING_SUBTITLES:
			return "Could not find any subtitle stream matching this variant stream";
		case M3U8ERR_MEDIA_NO_MATCHING_VIDEO:
			return "Could not find any video stream matching this variant stream";
		case M3U8ERR_MEDIA_PLAYLIST_NO_SEGMENTS:
			return "This M3U8 playlist has no media segments";
		case M3U8ERR_MEDIA_UNEXPECTED_CC:
			return "This Variant Stream must not have a CLOSED-CAPTIONS attribute whose value is anything other than NONE";
		case M3U8ERR_MEMORY_ALLOCATE_FAILURE:
			return "Could not allocate memory";
		case M3U8ERR_PARSER_INVALID_BRANGE:
			return "Could not parse this string as a byte range";
		case M3U8ERR_PARSER_INVALID_DTIME:
			return "Could not parse this string as an ISO/IEC 8601:2004 date/time representation";
		case M3U8ERR_PARSER_INVALID_ESTRING:
			return "Could not parse this string as an enumerated string";
		case M3U8ERR_PARSER_INVALID_FLOAT:
			return "Could not parse this string as a signed decimal floating point number";
		case M3U8ERR_PARSER_INVALID_HEXSEQ:
			return "Could not parse this string as an hexadecimal sequence";
		case M3U8ERR_PARSER_INVALID_QSTRING:
			return "Could not parse this string as a quoted string";
		case M3U8ERR_PARSER_INVALID_RESOLUTION:
			return "Could not parse this string as a decimal resolution";
		case M3U8ERR_PARSER_INVALID_UFLOAT:
			return "Could not parse this string as a decimal floating point number";
		case M3U8ERR_PARSER_INVALID_UINT:
			return "Could not parse this string as a decimal integer";
		case M3U8ERR_PARSER_INVALID_USTRING:
			return "Could not parse this string as an unquoted string";
		case M3U8ERR_PLAYLIST_LINE_TOO_LONG:
			return "This M3U8 playlist contains a line that is too long";
		case M3U8ERR_PLAYLIST_LINE_UNTERMINATED:
			return "This M3U8 playlist contains a line that was not terminated";
		case M3U8ERR_PLAYLIST_MISSING_TAG:
			return "This M3U8 playlist is missing a required M3U8 tag";
		case M3U8ERR_PLAYLIST_TOO_LARGE:
			return "This M3U8 playlist exceeds the maximum allowed size";
		case M3U8ERR_PLAYLIST_UNEXPECTED_ITEM:
			return "Encountered an unexpected item on the first line of the playlist";
		case M3U8ERR_PLAYLIST_UNEXPECTED_TAG:
			return "Encountered an unexpected playlist tag";
		case M3U8ERR_PLAYLIST_UNEXPECTED_URI:
			return "This M3U8 tag does not expect any URI to be supplied, but a URI was encountered";
		case M3U8ERR_PLAYLIST_UNKNOWN_TYPE:
			return "Could not determine the type of this M3U8 playlist";
		case M3U8ERR_PLAYLIST_WRONG_TAG_POSITION:
			return "This M3U8 playlist contains a M3U8 tag that is not in the expected position";
		case M3U8ERR_PRINTF_WRITE_FAILURE:
			return "Could not format string using snprintf";
		case M3U8ERR_TAG_DUPLICATE:
			return "This M3U8 tag cannot be specified multiple times in the same playlist";
		case M3U8ERR_TAG_MISSING_ATTRIBUTES:
			return "This M3U8 tag requires a list of attributes to be supplied";
		case M3U8ERR_TAG_MISSING_ITEMS:
			return "This M3U8 tag requires a list of items to be supplied";
		case M3U8ERR_TAG_MISSING_VALUE:
			return "This M3U8 tag requires a single-value option to be supplied";
		case M3U8ERR_TAG_NAME_INVALID:
			return "The name of this M3U8 tag is invalid or was not recognized";
		case M3U8ERR_TAG_NON_MATCHING_ATTRIBUTES:
			return "The attributes of this M3U8 tag do not match those of the other M3U8 tag with the same ID";
		case M3U8ERR_TAG_TRAILING_OPTIONS:
			return "This M3U8 tag does not require any value to be supplied, but trailing options were found";
	}
	
	return "Unknown error";
	
}
