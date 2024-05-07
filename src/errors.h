#if !defined(ERRORS_H)
#define ERRORS_H

#define M3U8ERR_SUCCESS 0 /* Success */

#define M3U8ERR_ATTRIBUTE_DUPLICATE -1 /* There must not be multiple attributes with the same name within the same tag */
#define M3U8ERR_ATTRIBUTE_EMPTY -2 /* An M3U8 tag must not contain empty attributes */
#define M3U8ERR_ATTRIBUTE_INVALID_BRANGE -3 /* The value of this M3U8 attribute is not a valid byte range */
#define M3U8ERR_ATTRIBUTE_INVALID_DTIME -4 /* The value of this M3U8 attribute is not a valid ISO/IEC 8601:2004 date/time representation */
#define M3U8ERR_ATTRIBUTE_INVALID_ESTRING -5 /* The value of this M3U8 attribute is not a valid enumerated string */
#define M3U8ERR_ATTRIBUTE_INVALID_FLOAT -6 /* The value of this M3U8 attribute is not a valid signed decimal floating point number */
#define M3U8ERR_ATTRIBUTE_INVALID_HEXSEQ -7 /* The value of this M3U8 attribute is not a valid hexadecimal sequence */
#define M3U8ERR_ATTRIBUTE_INVALID_NAME -8 /* The name of this M3U8 attribute is invalid or was not recognized */
#define M3U8ERR_ATTRIBUTE_INVALID_QSTRING -9 /* The value of this M3U8 attribute is not a valid quoted string */
#define M3U8ERR_ATTRIBUTE_INVALID_RESOLUTION -10 /* The value of this M3U8 attribute is not a valid decimal resolution */
#define M3U8ERR_ATTRIBUTE_INVALID_UFLOAT -11 /* The value of this M3U8 attribute is not a valid decimal floating point number */
#define M3U8ERR_ATTRIBUTE_INVALID_UINT -12 /* The value of this M3U8 attribute is not a valid decimal integer */
#define M3U8ERR_ATTRIBUTE_KEYFORMATVERSIONS_INVALID -13 /* The value of this M3U8 attribute is not a valid KEYFORMATVERSIONS */
#define M3U8ERR_ATTRIBUTE_MISSING -14 /* This M3U8 tag is missing a required M3U8 attribute */
#define M3U8ERR_ATTRIBUTE_MISSING_NAME -15 /* An M3U8 attribute requires a key to be supplied */
#define M3U8ERR_ATTRIBUTE_MISSING_VALUE -16 /* An M3U8 attribute requires a value to be supplied */
#define M3U8ERR_ATTRIBUTE_UNEXPECTED -17 /* Got an unexpected M3U8 attribute within this M3U8 tag */
#define M3U8ERR_ATTRIBUTE_VALUE_INVALID -18 /* The value of this M3U8 attribute is invalid or was not recognized */
#define M3U8ERR_ATTRIBUTE_WRONG_END_DATE -19 /* The END-DATE attribute value do not match the value of the DURATION attribute value */

#define M3U8ERR_BUFFER_OVERFLOW -20 /* Writing data to that memory address would cause a buffer overflow */

#define M3U8ERR_CALLBACK_WRITE_FAILURE -21 /* Callback returned error */

#define M3U8ERR_CLI_ARGUMENT_EMPTY -22 /* Got an empty argument while parsing the command-line arguments */
#define M3U8ERR_CLI_ARGUMENT_INVALID -23 /* This argument is invalid or was not recognized */
#define M3U8ERR_CLI_ARGUMENT_VALUE_MISSING -24 /* This keyword argument require a value to be supplied */
#define M3U8ERR_CLI_CANNOT_DETECT_FORMAT -25 /* Could not guess output file format; either the codec is not supported or the media file is corrupt */
#define M3U8ERR_CLI_CONCURRENCY_OUT_RANGE -26 /* The value specified for the keyword argument -c/--concurrency is out of range */
#define M3U8ERR_CLI_DUPLICATE_ARGUMENT -27 /* This argument cannot be specified multiple times */
#define M3U8ERR_CLI_MAX_REDIRS_OUT_RANGE -28 /* The value specified for the keyword argument --max-redirs is out of range */
#define M3U8ERR_CLI_NO_STREAMS_SELECTED -29 /* No streams selected */
#define M3U8ERR_CLI_OUTPUT_MISSING -30 /* No output file specified */
#define M3U8ERR_CLI_OUTPUT_MISSING_FILE_EXTENSION -31 /* The output filename does not contain a file extension */
#define M3U8ERR_CLI_PRIVILEGED_PROCESS_UNALLOWED -32 /* You are not supposed to run this program as a privileged user. Sorry. */
#define M3U8ERR_CLI_RETRY_OUT_RANGE -33 /* The value specified for the keyword argument -r/--retry is out of range */
#define M3U8ERR_CLI_SELECT_MEDIA_MAX_SELECTION_REACHED -34 /* Reached the maximum number of allowed media selection */
#define M3U8ERR_CLI_SELECT_MEDIA_NO_MATCHING_STREAMS -35 /* Could not find any media stream matching the requested index */
#define M3U8ERR_CLI_SELECT_MEDIA_OUT_RANGE -36 /* The value specified for the keyword argument --select-media is out of range */
#define M3U8ERR_CLI_SELECT_STREAM_MAX_SELECTION_REACHED -37 /* Reached the maximum number of allowed stream selection */
#define M3U8ERR_CLI_SELECT_STREAM_NO_AVAILABLE_STREAMS -38 /* This playlist does not contain any Variant Stream */
#define M3U8ERR_CLI_SELECT_STREAM_NO_MATCHING_STREAMS -39 /* Could not find any Variant Stream matching the requested resolution or position */
#define M3U8ERR_CLI_SELECT_STREAM_OUT_RANGE -40 /* The value specified for the keyword argument --select-stream is out of range */
#define M3U8ERR_CLI_SELECT_STREAM_RESOLUTION_INVALID -41 /* The value specified for the keyword argument --select-stream is not a valid resolution or position */
#define M3U8ERR_CLI_SELECT_STREAM_WILDCARD_UNSUPPORTED -42 /* The keyword argument --select-stream does not support wildcard matching */
#define M3U8ERR_CLI_URI_MISSING -43 /* No URI specified */
#define M3U8ERR_CLI_USER_INTERRUPTED -44 /* User interrupted */
#define M3U8ERR_CLI_VALUE_UNEXPECTED -45 /* Got an unexpected value while parsing the command-line arguments */

#define M3U8ERR_CURLE_GET_INFO_FAILURE -46 /* Could not get information about the cURL handler */

#define M3U8ERR_CURLM_ADD_FAILURE -47 /* Could not add the cURL handler to cURL multi */
#define M3U8ERR_CURLM_INIT_FAILURE -48 /* Could not initialize the cURL multi interface */
#define M3U8ERR_CURLM_PERFORM_FAILURE -49 /* Could not perform on cURL multi */
#define M3U8ERR_CURLM_POLL_FAILURE -50 /* Could not poll on cURL multi */
#define M3U8ERR_CURLM_REMOVE_FAILURE -51 /* Could not remove the cURL handler from cURL multi */
#define M3U8ERR_CURLM_SETOPT_FAILURE -52 /* Could not set options on cURL multi */

#define M3U8ERR_CURLSH_INIT_FAILURE -53 /* Could not initialize the cURL Share interface */
#define M3U8ERR_CURLSH_SETOPT_FAILURE -54 /* Could not set options on Share HTTP client */

#define M3U8ERR_CURLU_INIT_FAILURE -55 /* Could not initialize the cURL URL interface */
#define M3U8ERR_CURLU_URL_GET_FAILURE -56 /* Could not get URL from this cURL URL interface */
#define M3U8ERR_CURLU_URL_SET_FAILURE -57 /* Could not set URL for this cURL URL interface */

#define M3U8ERR_CURL_INIT_FAILURE -58 /* Could not initialize the HTTP client due to an unexpected error */
#define M3U8ERR_CURL_REQUEST_FAILURE -59 /* HTTP request failure */
#define M3U8ERR_CURL_SETOPT_FAILURE -60 /* Could not set options on HTTP client */
#define M3U8ERR_CURL_SLIST_FAILURE -61 /* Could not append item to list */

#define M3U8ERR_DOWNLOAD_COULD_NOT_CREATE_TMPDIR -62 /* Could not create the temporary directory */
#define M3U8ERR_DOWNLOAD_COULD_NOT_MOVE_FILE -63 /* Could not move file to specified location */
#define M3U8ERR_DOWNLOAD_LIVESTREAM_UNSUPPORTED -64 /* Downloading media streams from live-streaming playlists is not supported */
#define M3U8ERR_DOWNLOAD_NO_TMPDIR -65 /* Could not find a suitable directory for storing temporary files */

#define M3U8ERR_EXPAND_FILENAME_FAILURE -66 /* Could not resolve filename */

#define M3U8ERR_FFMPEG_COMMAND_NOT_FOUND -67 /* Could locate the FFmpeg executable */
#define M3U8ERR_FFMPEG_MUXING_FAILURE -68 /* Could not mux media streams */

#define M3U8ERR_FSTREAM_LOCK_FAILURE -69 /* Could not lock file */
#define M3U8ERR_FSTREAM_OPEN_FAILURE -70 /* Could not open file */
#define M3U8ERR_FSTREAM_READ_EMPTY_FILE -71 /* Tried to read contents from an empty file */
#define M3U8ERR_FSTREAM_READ_FAILURE -72 /* Could not read data from file */
#define M3U8ERR_FSTREAM_SEEK_FAILURE -73 /* Could not seek file */
#define M3U8ERR_FSTREAM_TELL_FAILURE -74 /* Could not get current file position */
#define M3U8ERR_FSTREAM_WRITE_FAILURE -75 /* Could not write data to file */

#define M3U8ERR_GET_APP_FILENAME_FAILURE -76 /* Could not get app filename */

#define M3U8ERR_ITEM_EMPTY -77 /* This M3U8 tag must not contain empty items */
#define M3U8ERR_ITEM_INVALID_BRANGE -78 /* The value of this M3U8 item is not a valid byte range */
#define M3U8ERR_ITEM_INVALID_DTIME -79 /* The value of this M3U8 item is not a valid ISO/IEC 8601:2004 date/time representation */
#define M3U8ERR_ITEM_INVALID_ESTRING -80 /* The value of this M3U8 item is not a valid enumerated string */
#define M3U8ERR_ITEM_INVALID_UFLOAT -81 /* The value of this M3U8 attribute is not a valid decimal floating point number */
#define M3U8ERR_ITEM_INVALID_UINT -82 /* The value of this M3U8 attribute is not a valid decimal integer */
#define M3U8ERR_ITEM_INVALID_USTRING -83 /* The value of this M3U8 item is not a valid unquoted string */
#define M3U8ERR_ITEM_MISSING -84 /* This M3U8 tag is missing a required M3U8 item */
#define M3U8ERR_ITEM_VALUE_INVALID -85 /* The value of this M3U8 item is invalid or was not recognized */
#define M3U8ERR_ITEM_VALUE_TOO_LONG -86 /* The value of this M3U8 item is too long */

#define M3U8ERR_LOAD_UNSUPPORTED_URI -87 /* Could not load M3U8 playlist from this URI; either this protocol is not supported or it was not recognized */

#define M3U8ERR_MEDIA_NO_MATCHING_AUDIO -88 /* Could not find any audio stream matching this variant stream */
#define M3U8ERR_MEDIA_NO_MATCHING_CLOSED_CAPTIONS -89 /* Could not find any closed-captions stream matching this variant stream */
#define M3U8ERR_MEDIA_NO_MATCHING_SUBTITLES -90 /* Could not find any subtitle stream matching this variant stream */
#define M3U8ERR_MEDIA_NO_MATCHING_VIDEO -91 /* Could not find any video stream matching this variant stream */
#define M3U8ERR_MEDIA_PLAYLIST_NO_SEGMENTS -92 /* This M3U8 playlist has no media segments */
#define M3U8ERR_MEDIA_UNEXPECTED_CC -93 /* This Variant Stream must not have a CLOSED-CAPTIONS attribute whose value is anything other than NONE */

#define M3U8ERR_MEMORY_ALLOCATE_FAILURE -94 /* Could not allocate memory */

#define M3U8ERR_PARSER_INVALID_BRANGE -95 /* Could not parse this string as a byte range */
#define M3U8ERR_PARSER_INVALID_DTIME -96 /* Could not parse this string as an ISO/IEC 8601:2004 date/time representation */
#define M3U8ERR_PARSER_INVALID_ESTRING -97 /* Could not parse this string as an enumerated string */
#define M3U8ERR_PARSER_INVALID_FLOAT -98 /* Could not parse this string as a signed decimal floating point number */
#define M3U8ERR_PARSER_INVALID_HEXSEQ -99 /* Could not parse this string as an hexadecimal sequence */
#define M3U8ERR_PARSER_INVALID_QSTRING -100 /* Could not parse this string as a quoted string */
#define M3U8ERR_PARSER_INVALID_RESOLUTION -101 /* Could not parse this string as a decimal resolution */
#define M3U8ERR_PARSER_INVALID_UFLOAT -102 /* Could not parse this string as a decimal floating point number */
#define M3U8ERR_PARSER_INVALID_UINT -103 /* Could not parse this string as a decimal integer */
#define M3U8ERR_PARSER_INVALID_USTRING -104 /* Could not parse this string as an unquoted string */

#define M3U8ERR_PLAYLIST_LINE_TOO_LONG -105 /* This M3U8 playlist contains a line that is too long */
#define M3U8ERR_PLAYLIST_LINE_UNTERMINATED -106 /* This M3U8 playlist contains a line that was not terminated */
#define M3U8ERR_PLAYLIST_MISSING_TAG -107 /* This M3U8 playlist is missing a required M3U8 tag */
#define M3U8ERR_PLAYLIST_TOO_LARGE -108 /* This M3U8 playlist exceeds the maximum allowed size */
#define M3U8ERR_PLAYLIST_UNEXPECTED_ITEM -109 /* Encountered an unexpected item on the first line of the playlist */
#define M3U8ERR_PLAYLIST_UNEXPECTED_TAG -110 /* Encountered an unexpected playlist tag */
#define M3U8ERR_PLAYLIST_UNEXPECTED_URI -111 /* This M3U8 tag does not expect any URI to be supplied, but a URI was encountered */
#define M3U8ERR_PLAYLIST_UNKNOWN_TYPE -112 /* Could not determine the type of this M3U8 playlist */
#define M3U8ERR_PLAYLIST_WRONG_TAG_POSITION -113 /* This M3U8 playlist contains a M3U8 tag that is not in the expected position */

#define M3U8ERR_PRINTF_WRITE_FAILURE -114 /* Could not format string using snprintf */

#define M3U8ERR_TAG_DUPLICATE -115 /* This M3U8 tag cannot be specified multiple times in the same playlist */
#define M3U8ERR_TAG_MISSING_ATTRIBUTES -116 /* This M3U8 tag requires a list of attributes to be supplied */
#define M3U8ERR_TAG_MISSING_ITEMS -117 /* This M3U8 tag requires a list of items to be supplied */
#define M3U8ERR_TAG_MISSING_VALUE -118 /* This M3U8 tag requires a single-value option to be supplied */
#define M3U8ERR_TAG_NAME_INVALID -119 /* The name of this M3U8 tag is invalid or was not recognized */
#define M3U8ERR_TAG_NON_MATCHING_ATTRIBUTES -120 /* The attributes of this M3U8 tag do not match those of the other M3U8 tag with the same ID */
#define M3U8ERR_TAG_TRAILING_OPTIONS -121 /* This M3U8 tag does not require any value to be supplied, but trailing options were found */

const char* m3u8err_getmessage(const int code);

#endif
