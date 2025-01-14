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
#define M3U8ERR_ATTRIBUTE_NAME_TOO_LONG -17 /* The name of this M3U8 attribute is too long */
#define M3U8ERR_ATTRIBUTE_UNEXPECTED -18 /* Got an unexpected M3U8 attribute within this M3U8 tag */
#define M3U8ERR_ATTRIBUTE_VALUE_INVALID -19 /* The value of this M3U8 attribute is invalid or was not recognized */
#define M3U8ERR_ATTRIBUTE_VALUE_TOO_LONG -20 /* The value of this M3U8 attribute is too long */
#define M3U8ERR_ATTRIBUTE_WRONG_END_DATE -21 /* The END-DATE attribute value do not match the value of the DURATION attribute value */

#define M3U8ERR_BUFFER_OVERFLOW -22 /* Writing data to that memory address would cause a buffer overflow */

#define M3U8ERR_CALLBACK_WRITE_FAILURE -23 /* Callback returned error */

#define M3U8ERR_CLI_ARGUMENT_EMPTY -24 /* Got an empty argument while parsing the command-line arguments */
#define M3U8ERR_CLI_ARGUMENT_INVALID -25 /* This argument is invalid or was not recognized */
#define M3U8ERR_CLI_ARGUMENT_VALUE_MISSING -26 /* This keyword argument require a value to be supplied */
#define M3U8ERR_CLI_CANNOT_DETECT_FORMAT -27 /* Could not guess output file format; either the codec is not supported or the media file is corrupt */
#define M3U8ERR_CLI_CONCURRENCY_OUT_RANGE -28 /* The value specified for the keyword argument -c/--concurrency is out of range */
#define M3U8ERR_CLI_DUPLICATE_ARGUMENT -29 /* This argument cannot be specified multiple times */
#define M3U8ERR_CLI_MAX_REDIRS_OUT_RANGE -30 /* The value specified for the keyword argument --max-redirs is out of range */
#define M3U8ERR_CLI_NO_STREAMS_SELECTED -31 /* No streams selected */
#define M3U8ERR_CLI_OUTPUT_MISSING -32 /* No output file specified */
#define M3U8ERR_CLI_OUTPUT_MISSING_FILE_EXTENSION -33 /* The output filename does not contain a file extension */
#define M3U8ERR_CLI_PRIVILEGED_PROCESS_UNALLOWED -34 /* You are not supposed to run this program as a privileged user. Sorry. */
#define M3U8ERR_CLI_RETRY_OUT_RANGE -35 /* The value specified for the keyword argument -r/--retry is out of range */
#define M3U8ERR_CLI_SELECT_MEDIA_MAX_SELECTION_REACHED -36 /* Reached the maximum number of allowed media selection */
#define M3U8ERR_CLI_SELECT_MEDIA_NO_MATCHING_STREAMS -37 /* Could not find any media stream matching the requested index */
#define M3U8ERR_CLI_SELECT_MEDIA_OUT_RANGE -38 /* The value specified for the keyword argument --select-media is out of range */
#define M3U8ERR_CLI_SELECT_STREAM_MAX_SELECTION_REACHED -39 /* Reached the maximum number of allowed stream selection */
#define M3U8ERR_CLI_SELECT_STREAM_NO_AVAILABLE_STREAMS -40 /* This playlist does not contain any Variant Stream */
#define M3U8ERR_CLI_SELECT_STREAM_NO_MATCHING_STREAMS -41 /* Could not find any Variant Stream matching the requested resolution or position */
#define M3U8ERR_CLI_SELECT_STREAM_OUT_RANGE -42 /* The value specified for the keyword argument --select-stream is out of range */
#define M3U8ERR_CLI_SELECT_STREAM_RESOLUTION_INVALID -43 /* The value specified for the keyword argument --select-stream is not a valid resolution or position */
#define M3U8ERR_CLI_SELECT_STREAM_WILDCARD_UNSUPPORTED -44 /* The keyword argument --select-stream does not support wildcard matching */
#define M3U8ERR_CLI_URI_MISSING -45 /* No URI specified */
#define M3U8ERR_CLI_USER_INTERRUPTED -46 /* User interrupted */
#define M3U8ERR_CLI_VALUE_UNEXPECTED -47 /* Got an unexpected value while parsing the command-line arguments */

#define M3U8ERR_CURLE_GET_INFO_FAILURE -48 /* Could not get information about the cURL handler */

#define M3U8ERR_CURLM_ADD_FAILURE -49 /* Could not add the cURL handler to cURL multi */
#define M3U8ERR_CURLM_INIT_FAILURE -50 /* Could not initialize the cURL multi interface */
#define M3U8ERR_CURLM_PERFORM_FAILURE -51 /* Could not perform on cURL multi */
#define M3U8ERR_CURLM_POLL_FAILURE -52 /* Could not poll on cURL multi */
#define M3U8ERR_CURLM_REMOVE_FAILURE -53 /* Could not remove the cURL handler from cURL multi */
#define M3U8ERR_CURLM_SETOPT_FAILURE -54 /* Could not set options on cURL multi */

#define M3U8ERR_CURLSH_INIT_FAILURE -55 /* Could not initialize the cURL Share interface */
#define M3U8ERR_CURLSH_SETOPT_FAILURE -56 /* Could not set options on Share HTTP client */

#define M3U8ERR_CURLU_INIT_FAILURE -57 /* Could not initialize the cURL URL interface */
#define M3U8ERR_CURLU_URL_GET_FAILURE -58 /* Could not get URL from this cURL URL interface */
#define M3U8ERR_CURLU_URL_SET_FAILURE -59 /* Could not set URL for this cURL URL interface */

#define M3U8ERR_CURL_INIT_FAILURE -60 /* Could not initialize the HTTP client due to an unexpected error */
#define M3U8ERR_CURL_REQUEST_FAILURE -61 /* HTTP request failure */
#define M3U8ERR_CURL_SETOPT_FAILURE -62 /* Could not set options on HTTP client */
#define M3U8ERR_CURL_SLIST_FAILURE -63 /* Could not append item to list */

#define M3U8ERR_DOWNLOAD_COULD_NOT_CREATE_TMPDIR -64 /* Could not create the temporary directory */
#define M3U8ERR_DOWNLOAD_COULD_NOT_MOVE_FILE -65 /* Could not move file to specified location */
#define M3U8ERR_DOWNLOAD_LIVESTREAM_UNSUPPORTED -66 /* Downloading media streams from live-streaming playlists is not supported */
#define M3U8ERR_DOWNLOAD_NO_TMPDIR -67 /* Could not find a suitable directory for storing temporary files */

#define M3U8ERR_EXPAND_FILENAME_FAILURE -68 /* Could not resolve filename */

#define M3U8ERR_FFMPEG_COMMAND_NOT_FOUND -69 /* Could locate the FFmpeg executable */
#define M3U8ERR_FFMPEG_MUXING_FAILURE -70 /* Could not mux media streams */

#define M3U8ERR_FSTREAM_LOCK_FAILURE -71 /* Could not lock file */
#define M3U8ERR_FSTREAM_OPEN_FAILURE -72 /* Could not open file */
#define M3U8ERR_FSTREAM_READ_EMPTY_FILE -73 /* Tried to read contents from an empty file */
#define M3U8ERR_FSTREAM_READ_FAILURE -74 /* Could not read data from file */
#define M3U8ERR_FSTREAM_SEEK_FAILURE -75 /* Could not seek file */
#define M3U8ERR_FSTREAM_TELL_FAILURE -76 /* Could not get current file position */
#define M3U8ERR_FSTREAM_WRITE_FAILURE -77 /* Could not write data to file */

#define M3U8ERR_GET_APP_FILENAME_FAILURE -78 /* Could not get app filename */

#define M3U8ERR_ITEM_EMPTY -79 /* This M3U8 tag must not contain empty items */
#define M3U8ERR_ITEM_INVALID_BRANGE -80 /* The value of this M3U8 item is not a valid byte range */
#define M3U8ERR_ITEM_INVALID_DTIME -81 /* The value of this M3U8 item is not a valid ISO/IEC 8601:2004 date/time representation */
#define M3U8ERR_ITEM_INVALID_ESTRING -82 /* The value of this M3U8 item is not a valid enumerated string */
#define M3U8ERR_ITEM_INVALID_UFLOAT -83 /* The value of this M3U8 attribute is not a valid decimal floating point number */
#define M3U8ERR_ITEM_INVALID_UINT -84 /* The value of this M3U8 attribute is not a valid decimal integer */
#define M3U8ERR_ITEM_INVALID_USTRING -85 /* The value of this M3U8 item is not a valid unquoted string */
#define M3U8ERR_ITEM_MISSING -86 /* This M3U8 tag is missing a required M3U8 item */
#define M3U8ERR_ITEM_VALUE_INVALID -87 /* The value of this M3U8 item is invalid or was not recognized */
#define M3U8ERR_ITEM_VALUE_TOO_LONG -88 /* The value of this M3U8 item is too long */

#define M3U8ERR_LOAD_UNSUPPORTED_URI -89 /* Could not load M3U8 playlist from this URI; either this protocol is not supported or it was not recognized */

#define M3U8ERR_MEDIA_NO_MATCHING_AUDIO -90 /* Could not find any audio stream matching this variant stream */
#define M3U8ERR_MEDIA_NO_MATCHING_CLOSED_CAPTIONS -91 /* Could not find any closed-captions stream matching this variant stream */
#define M3U8ERR_MEDIA_NO_MATCHING_SUBTITLES -92 /* Could not find any subtitle stream matching this variant stream */
#define M3U8ERR_MEDIA_NO_MATCHING_VIDEO -93 /* Could not find any video stream matching this variant stream */
#define M3U8ERR_MEDIA_PLAYLIST_NO_SEGMENTS -94 /* This M3U8 playlist has no media segments */
#define M3U8ERR_MEDIA_UNEXPECTED_CC -95 /* This Variant Stream must not have a CLOSED-CAPTIONS attribute whose value is anything other than NONE */

#define M3U8ERR_MEMORY_ALLOCATE_FAILURE -96 /* Could not allocate memory */

#define M3U8ERR_PARSER_INVALID_BRANGE -97 /* Could not parse this string as a byte range */
#define M3U8ERR_PARSER_INVALID_DTIME -98 /* Could not parse this string as an ISO/IEC 8601:2004 date/time representation */
#define M3U8ERR_PARSER_INVALID_ESTRING -99 /* Could not parse this string as an enumerated string */
#define M3U8ERR_PARSER_INVALID_FLOAT -100 /* Could not parse this string as a signed decimal floating point number */
#define M3U8ERR_PARSER_INVALID_HEXSEQ -101 /* Could not parse this string as an hexadecimal sequence */
#define M3U8ERR_PARSER_INVALID_QSTRING -102 /* Could not parse this string as a quoted string */
#define M3U8ERR_PARSER_INVALID_RESOLUTION -103 /* Could not parse this string as a decimal resolution */
#define M3U8ERR_PARSER_INVALID_UFLOAT -104 /* Could not parse this string as a decimal floating point number */
#define M3U8ERR_PARSER_INVALID_UINT -105 /* Could not parse this string as a decimal integer */
#define M3U8ERR_PARSER_INVALID_USTRING -106 /* Could not parse this string as an unquoted string */

#define M3U8ERR_PLAYLIST_LINE_TOO_LONG -107 /* This M3U8 playlist contains a line that is too long */
#define M3U8ERR_PLAYLIST_LINE_UNTERMINATED -108 /* This M3U8 playlist contains a line that was not terminated */
#define M3U8ERR_PLAYLIST_MISSING_TAG -109 /* This M3U8 playlist is missing a required M3U8 tag */
#define M3U8ERR_PLAYLIST_TOO_LARGE -110 /* This M3U8 playlist exceeds the maximum allowed size */
#define M3U8ERR_PLAYLIST_UNEXPECTED_ITEM -111 /* Encountered an unexpected item on the first line of the playlist */
#define M3U8ERR_PLAYLIST_UNEXPECTED_TAG -112 /* Encountered an unexpected playlist tag */
#define M3U8ERR_PLAYLIST_UNEXPECTED_URI -113 /* This M3U8 tag does not expect any URI to be supplied, but a URI was encountered */
#define M3U8ERR_PLAYLIST_UNKNOWN_TYPE -114 /* Could not determine the type of this M3U8 playlist */
#define M3U8ERR_PLAYLIST_WRONG_TAG_POSITION -115 /* This M3U8 playlist contains a M3U8 tag that is not in the expected position */

#define M3U8ERR_PRINTF_WRITE_FAILURE -116 /* Could not format string using snprintf */

#define M3U8ERR_TAG_DUPLICATE -117 /* This M3U8 tag cannot be specified multiple times in the same playlist */
#define M3U8ERR_TAG_MISSING_ATTRIBUTES -118 /* This M3U8 tag requires a list of attributes to be supplied */
#define M3U8ERR_TAG_MISSING_ITEMS -119 /* This M3U8 tag requires a list of items to be supplied */
#define M3U8ERR_TAG_MISSING_VALUE -120 /* This M3U8 tag requires a single-value option to be supplied */
#define M3U8ERR_TAG_NAME_INVALID -121 /* The name of this M3U8 tag is invalid or was not recognized */
#define M3U8ERR_TAG_NON_MATCHING_ATTRIBUTES -122 /* The attributes of this M3U8 tag do not match those of the other M3U8 tag with the same ID */
#define M3U8ERR_TAG_TRAILING_OPTIONS -123 /* This M3U8 tag does not require any value to be supplied, but trailing options were found */

const char* m3u8err_getmessage(const int code);

#endif
