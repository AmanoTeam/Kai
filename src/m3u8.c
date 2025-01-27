#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "m3u8.h"
#include "m3u8utils.h"
#include "biggestint.h"
#include "errors.h"
#include "m3u8parser.h"
#include "m3u8sizeof.h"
#include "fstream.h"
#include "filesystem.h"
#include "strsplit.h"
#include "httpclient.h"
#include "hex.h"
#include "sutils.h"
#include "guess_uri.h"

static const enum M3U8VAttrType M3U8_VATTR_TYPES[] = {
	M3U8_VATTR_TYPE_UINT,
	M3U8_VATTR_TYPE_HEXSEQ,
	M3U8_VATTR_TYPE_UFLOAT,
	M3U8_VATTR_TYPE_FLOAT,
	M3U8_VATTR_TYPE_QSTRING,
	M3U8_VATTR_TYPE_ESTRING,
	M3U8_VATTR_TYPE_RESOLUTION
};

static const enum M3U8VItemType M3U8_VITEM_TYPES[] = {
	M3U8_VITEM_TYPE_UINT,
	M3U8_VITEM_TYPE_UFLOAT,
	M3U8_VITEM_TYPE_BRANGE,
	M3U8_VITEM_TYPE_DTIME,
	M3U8_VITEM_TYPE_ESTRING,
	M3U8_VITEM_TYPE_USTRING
};

#define M3U8TAG_SETVAL_URI 0
#define M3U8TAG_SETVAL_VALUE 1
#define M3U8TAG_SETVAL_ATTRIBUTES 2
#define M3U8TAG_SETVAL_ITEMS 3

/*
Max limit for line lengths.
*/
#define M3U8_MAX_LINE_LEN (1024 * 10) /* 10 KiB */

/*
Max length limit for attribute values.
*/
#define M3U8_MAX_ATTR_VALUE_LEN (1024 * 5) /* 5 KiB */

/*
Max length limit for attribute names.
*/
#define M3U8_MAX_ATTR_NAME_LEN (50 * 1) /* 50 */

/*
Max length limit for tag names.
*/
#define M3U8_MAX_TAG_NAME_LEN (28 + 1) /* 28 */

/*
Max length limit for item values.
*/
#define M3U8_MAX_ITEM_VALUE_LEN (128 * 5)

/*
Maximum length limit for M3U8 playlists.
*/
#define M3U8_MAX_PLAYLIST_LEN ((1024 * 1024 * 16) + 1) /* 16 MiB */

#define M3U8_PLAYLIST_TAG 1
#define M3U8_PLAYLIST_COMMENT 2
#define M3U8_PLAYLIST_URI 3

/* M3U8 tags. */
static const char M3U8K_EXTM3U[] = "EXTM3U";
static const char M3U8K_EXT_X_VERSION[] = "EXT-X-VERSION";
static const char M3U8K_EXTINF[] = "EXTINF";
static const char M3U8K_EXT_X_BYTERANGE[] = "EXT-X-BYTERANGE";
static const char M3U8K_EXT_X_DISCONTINUITY[] = "EXT-X-DISCONTINUITY";
static const char M3U8K_EXT_X_KEY[] = "EXT-X-KEY";
static const char M3U8K_EXT_X_MAP[] = "EXT-X-MAP";
static const char M3U8K_EXT_X_PROGRAM_DATE_TIME[] = "EXT-X-PROGRAM-DATE-TIME";
static const char M3U8K_EXT_X_ALLOW_CACHE[] = "EXT-X-ALLOW-CACHE";
static const char M3U8K_EXT_X_DATERANGE[] = "EXT-X-DATERANGE";
static const char M3U8K_EXT_X_TARGETDURATION[] = "EXT-X-TARGETDURATION";
static const char M3U8K_EXT_X_MEDIA_SEQUENCE[] = "EXT-X-MEDIA-SEQUENCE";
static const char M3U8K_EXT_X_DISCONTINUITY_SEQUENCE[] = "EXT-X-DISCONTINUITY-SEQUENCE";
static const char M3U8K_EXT_X_ENDLIST[] = "EXT-X-ENDLIST";
static const char M3U8K_EXT_X_PLAYLIST_TYPE[] = "EXT-X-PLAYLIST-TYPE";
static const char M3U8K_EXT_X_I_FRAMES_ONLY[] = "EXT-X-I-FRAMES-ONLY";
static const char M3U8K_EXT_X_MEDIA[] = "EXT-X-MEDIA";
static const char M3U8K_EXT_X_STREAM_INF[] = "EXT-X-STREAM-INF";
static const char M3U8K_EXT_X_I_FRAME_STREAM_INF[] = "EXT-X-I-FRAME-STREAM-INF";
static const char M3U8K_EXT_X_SESSION_DATA[] = "EXT-X-SESSION-DATA";
static const char M3U8K_EXT_X_SESSION_KEY[] = "EXT-X-SESSION-KEY";
static const char M3U8K_EXT_X_INDEPENDENT_SEGMENTS[] = "EXT-X-INDEPENDENT-SEGMENTS";
static const char M3U8K_EXT_X_START[] = "EXT-X-START";
static const char M3U8K_EXT_X_DEFINE[] = "EXT-X-DEFINE";
static const char M3U8K_EXT_X_PART_INF[] = "EXT-X-PART-INF";
static const char M3U8K_EXT_X_SERVER_CONTROL[] = "EXT-X-SERVER-CONTROL";
static const char M3U8K_EXT_X_GAP[] = "EXT-X-GAP";
static const char M3U8K_EXT_X_BITRATE[] = "EXT-X-BITRATE";
static const char M3U8K_EXT_X_PART[] = "EXT-X-PART";
static const char M3U8K_EXT_X_SKIP[] = "EXT-X-SKIP";
static const char M3U8K_EXT_X_PRELOAD_HINT[] = "EXT-X-PRELOAD-HINT";
static const char M3U8K_EXT_X_RENDITION_REPORT[] = "EXT-X-RENDITION-REPORT";

/* M3U8 attributes. */
static const char M3U8K_METHOD[] = "METHOD";
static const char M3U8K_URI[] = "URI";
static const char M3U8K_IV[] = "IV";
static const char M3U8K_KEYFORMAT[] = "KEYFORMAT";
static const char M3U8K_KEYFORMATVERSIONS[] = "KEYFORMATVERSIONS";
static const char M3U8K_BYTERANGE[] = "BYTERANGE";
static const char M3U8K_ID[] = "ID";
static const char M3U8K_CLASS[] = "CLASS";
static const char M3U8K_START_DATE[] = "START-DATE";
static const char M3U8K_END_DATE[] = "END-DATE";
static const char M3U8K_DURATION[] = "DURATION";
static const char M3U8K_PLANNED_DURATION[] = "PLANNED-DURATION";
static const char M3U8K_END_ON_NEXT[] = "END-ON-NEXT";
static const char M3U8K_TYPE[] = "TYPE";
static const char M3U8K_GROUP_ID[] = "GROUP-ID";
static const char M3U8K_LANGUAGE[] = "LANGUAGE";
static const char M3U8K_ASSOC_LANGUAGE[] = "ASSOC-LANGUAGE";
static const char M3U8K_NAME[] = "NAME";
static const char M3U8K_DEFAULT[] = "DEFAULT";
static const char M3U8K_AUTOSELECT[] = "AUTOSELECT";
static const char M3U8K_FORCED[] = "FORCED";
static const char M3U8K_INSTREAM_ID[] = "INSTREAM-ID";
static const char M3U8K_CHARACTERISTICS[] = "CHARACTERISTICS";
static const char M3U8K_CHANNELS[] = "CHANNELS";
static const char M3U8K_BANDWIDTH[] = "BANDWIDTH";
static const char M3U8K_PROGRAM_ID[] = "PROGRAM-ID";
static const char M3U8K_AVERAGE_BANDWIDTH[] = "AVERAGE-BANDWIDTH";
static const char M3U8K_CODECS[] = "CODECS";
static const char M3U8K_RESOLUTION[] = "RESOLUTION";
static const char M3U8K_FRAME_8RATE[] = "FRAME-RATE";
static const char M3U8K_HDCP_LEVEL[] = "HDCP-LEVEL";
static const char M3U8K_AUDIO[] = "AUDIO";
static const char M3U8K_VIDEO[] = "VIDEO";
static const char M3U8K_SUBTITLES[] = "SUBTITLES";
static const char M3U8K_CLOSED_CAPTIONS[] = "CLOSED-CAPTIONS";
static const char M3U8K_DATA_ID[] = "DATA-ID";
static const char M3U8K_VALUE[] = "VALUE";
static const char M3U8K_TIME_OFFSET[] = "TIME-OFFSET";
static const char M3U8K_PRECISE[] = "PRECISE";
static const char M3U8K_IMPORT[] = "IMPORT";
static const char M3U8K_QUERYPARAM[] = "QUERYPARAM";
static const char M3U8K_PART_TARGET[] = "PART-TARGET";
static const char M3U8K_CAN_SKIP_UNTIL[] = "CAN-SKIP-UNTIL";
static const char M3U8K_CAN_SKIP_DATERANGES[] = "CAN-SKIP-DATERANGES";
static const char M3U8K_HOLD_BACK[] = "HOLD-BACK";
static const char M3U8K_PART_HOLD_BACK[] = "PART-HOLD-BACK";
static const char M3U8K_CAN_BLOCK_RELOAD[] = "CAN-BLOCK-RELOAD";
static const char M3U8K_INDEPENDENT[] = "INDEPENDENT";
static const char M3U8K_GAP[] = "GAP";
static const char M3U8K_CUE[] = "CUE";
static const char M3U8K_SKIPPED_SEGMENTS[] = "SKIPPED-SEGMENTS";
static const char M3U8K_RECENTLY_REMOVED_DATERANGES[] = "RECENTLY-REMOVED-DATERANGES";
static const char M3U8K_BYTERANGE_START[] = "BYTERANGE-START";
static const char M3U8K_BYTERANGE_LENGTH[] = "BYTERANGE-LENGTH";
static const char M3U8K_LAST_MSN[] = "LAST-MSN";
static const char M3U8K_LAST_PART[] = "LAST-PART";
static const char M3U8K_STABLE_RENDITION_ID[] = "STABLE-RENDITION-ID";
static const char M3U8K_BIT_DEPTH[] = "BIT-DEPTH";
static const char M3U8K_SAMPLE_RATE[] = "SAMPLE-RATE";
static const char M3U8K_SCORE[] = "SCORE";
static const char M3U8K_SUPPLEMENTAL_CODECS[] = "SUPPLEMENTAL-CODECS";
static const char M3U8K_ALLOWED_CPC[] = "ALLOWED-CPC";
static const char M3U8K_VIDEO_RANGE[] = "VIDEO-RANGE";
static const char M3U8K_REQ_VIDEO_LAYOUT[] = "REQ-VIDEO-LAYOUT";
static const char M3U8K_STABLE_VARIANT_ID[] = "STABLE-VARIANT-ID";
static const char M3U8K_PATHWAY_ID[] = "PATHWAY-ID";

static const char* m3u8tag_stringify(const enum M3U8TagType type) {
	/*
	Get the string representation of this M3U8 tag type.
	*/
	
	switch (type) {
		case M3U8_TAG_EXTM3U:
			return M3U8K_EXTM3U;
		case M3U8_TAG_EXT_X_VERSION:
			return M3U8K_EXT_X_VERSION;
		case M3U8_TAG_EXTINF:
			return M3U8K_EXTINF;
		case M3U8_TAG_EXT_X_BYTERANGE:
			return M3U8K_EXT_X_BYTERANGE;
		case M3U8_TAG_EXT_X_DISCONTINUITY:
			return M3U8K_EXT_X_DISCONTINUITY;
		case M3U8_TAG_EXT_X_KEY:
			return M3U8K_EXT_X_KEY;
		case M3U8_TAG_EXT_X_MAP:
			return M3U8K_EXT_X_MAP;
		case M3U8_TAG_EXT_X_PROGRAM_DATE_TIME:
			return M3U8K_EXT_X_PROGRAM_DATE_TIME;
		case M3U8_TAG_EXT_X_ALLOW_CACHE:
			return M3U8K_EXT_X_ALLOW_CACHE;
		case M3U8_TAG_EXT_X_DATERANGE:
			return M3U8K_EXT_X_DATERANGE;
		case M3U8_TAG_EXT_X_TARGETDURATION:
			return M3U8K_EXT_X_TARGETDURATION;
		case M3U8_TAG_EXT_X_MEDIA_SEQUENCE:
			return M3U8K_EXT_X_MEDIA_SEQUENCE;
		case M3U8_TAG_EXT_X_DISCONTINUITY_SEQUENCE:
			return M3U8K_EXT_X_DISCONTINUITY_SEQUENCE;
		case M3U8_TAG_EXT_X_ENDLIST:
			return M3U8K_EXT_X_ENDLIST;
		case M3U8_TAG_EXT_X_PLAYLIST_TYPE:
			return M3U8K_EXT_X_PLAYLIST_TYPE;
		case M3U8_TAG_EXT_X_I_FRAMES_ONLY:
			return M3U8K_EXT_X_I_FRAMES_ONLY;
		case M3U8_TAG_EXT_X_MEDIA:
			return M3U8K_EXT_X_MEDIA;
		case M3U8_TAG_EXT_X_STREAM_INF:
			return M3U8K_EXT_X_STREAM_INF;
		case M3U8_TAG_EXT_X_I_FRAME_STREAM_INF:
			return M3U8K_EXT_X_I_FRAME_STREAM_INF;
		case M3U8_TAG_EXT_X_SESSION_DATA:
			return M3U8K_EXT_X_SESSION_DATA;
		case M3U8_TAG_EXT_X_SESSION_KEY:
			return M3U8K_EXT_X_SESSION_KEY;
		case M3U8_TAG_EXT_X_INDEPENDENT_SEGMENTS:
			return M3U8K_EXT_X_INDEPENDENT_SEGMENTS;
		case M3U8_TAG_EXT_X_START:
			return M3U8K_EXT_X_START;
		case M3U8_TAG_EXT_X_DEFINE:
			return M3U8K_EXT_X_DEFINE;
		case M3U8_TAG_EXT_X_PART_INF:
			return M3U8K_EXT_X_PART_INF;
		case M3U8_TAG_EXT_X_SERVER_CONTROL:
			return M3U8K_EXT_X_SERVER_CONTROL;
		case M3U8_TAG_EXT_X_GAP:
			return M3U8K_EXT_X_GAP;
		case M3U8_TAG_EXT_X_BITRATE:
			return M3U8K_EXT_X_BITRATE;
		case M3U8_TAG_EXT_X_PART:
			return M3U8K_EXT_X_PART;
		case M3U8_TAG_EXT_X_SKIP:
			return M3U8K_EXT_X_SKIP;
		case M3U8_TAG_EXT_X_PRELOAD_HINT:
			return M3U8K_EXT_X_PRELOAD_HINT;
		case M3U8_TAG_EXT_X_RENDITION_REPORT:
			return M3U8K_EXT_X_RENDITION_REPORT;
	}
	
	return NULL;
	
}

static enum M3U8TagType m3u8tag_unstringify(const char* const name) {
	
	if (strcmp(name, M3U8K_EXTM3U) == 0) {
		return M3U8_TAG_EXTM3U;
	}
	
	if (strcmp(name, M3U8K_EXT_X_VERSION) == 0) {
		return M3U8_TAG_EXT_X_VERSION;
	}
	
	if (strcmp(name, M3U8K_EXTINF) == 0) {
		return M3U8_TAG_EXTINF;
	}
	
	if (strcmp(name, M3U8K_EXT_X_BYTERANGE) == 0) {
		return M3U8_TAG_EXT_X_BYTERANGE;
	}
	
	if (strcmp(name, M3U8K_EXT_X_DISCONTINUITY) == 0) {
		return M3U8_TAG_EXT_X_DISCONTINUITY;
	}
	
	if (strcmp(name, M3U8K_EXT_X_KEY) == 0) {
		return M3U8_TAG_EXT_X_KEY;
	}
	
	if (strcmp(name, M3U8K_EXT_X_MAP) == 0) {
		return M3U8_TAG_EXT_X_MAP;
	}
	
	if (strcmp(name, M3U8K_EXT_X_PROGRAM_DATE_TIME) == 0) {
		return M3U8_TAG_EXT_X_PROGRAM_DATE_TIME;
	}
	
	if (strcmp(name, M3U8K_EXT_X_ALLOW_CACHE) == 0) {
		return M3U8_TAG_EXT_X_ALLOW_CACHE;
	}
	
	if (strcmp(name, M3U8K_EXT_X_DATERANGE) == 0) {
		return M3U8_TAG_EXT_X_DATERANGE;
	}
	
	if (strcmp(name, M3U8K_EXT_X_TARGETDURATION) == 0) {
		return M3U8_TAG_EXT_X_TARGETDURATION;
	}
	
	if (strcmp(name, M3U8K_EXT_X_MEDIA_SEQUENCE) == 0) {
		return M3U8_TAG_EXT_X_MEDIA_SEQUENCE;
	}
	
	if (strcmp(name, M3U8K_EXT_X_DISCONTINUITY_SEQUENCE) == 0) {
		return M3U8_TAG_EXT_X_DISCONTINUITY_SEQUENCE;
	}
	
	if (strcmp(name, M3U8K_EXT_X_ENDLIST) == 0) {
		return M3U8_TAG_EXT_X_ENDLIST;
	}
	
	if (strcmp(name, M3U8K_EXT_X_PLAYLIST_TYPE) == 0) {
		return M3U8_TAG_EXT_X_PLAYLIST_TYPE;
	}
	
	if (strcmp(name, M3U8K_EXT_X_I_FRAMES_ONLY) == 0) {
		return M3U8_TAG_EXT_X_I_FRAMES_ONLY;
	}
	
	if (strcmp(name, M3U8K_EXT_X_MEDIA) == 0) {
		return M3U8_TAG_EXT_X_MEDIA;
	}
	
	if (strcmp(name, M3U8K_EXT_X_STREAM_INF) == 0) {
		return M3U8_TAG_EXT_X_STREAM_INF;
	}
	
	if (strcmp(name, M3U8K_EXT_X_I_FRAME_STREAM_INF) == 0) {
		return M3U8_TAG_EXT_X_I_FRAME_STREAM_INF;
	}
	
	if (strcmp(name, M3U8K_EXT_X_SESSION_DATA) == 0) {
		return M3U8_TAG_EXT_X_SESSION_DATA;
	}
	
	if (strcmp(name, M3U8K_EXT_X_SESSION_KEY) == 0) {
		return M3U8_TAG_EXT_X_SESSION_KEY;
	}
	
	if (strcmp(name, M3U8K_EXT_X_INDEPENDENT_SEGMENTS) == 0) {
		return M3U8_TAG_EXT_X_INDEPENDENT_SEGMENTS;
	}
	
	if (strcmp(name, M3U8K_EXT_X_START) == 0) {
		return M3U8_TAG_EXT_X_START;
	}
	
	if (strcmp(name, M3U8K_EXT_X_DEFINE) == 0) {
		return M3U8_TAG_EXT_X_DEFINE;
	}
	
	if (strcmp(name, M3U8K_EXT_X_PART_INF) == 0) {
		return M3U8_TAG_EXT_X_PART_INF;
	}
	
	if (strcmp(name, M3U8K_EXT_X_SERVER_CONTROL) == 0) {
		return M3U8_TAG_EXT_X_SERVER_CONTROL;
	}
	
	if (strcmp(name, M3U8K_EXT_X_GAP) == 0) {
		return M3U8_TAG_EXT_X_GAP;
	}
	
	if (strcmp(name, M3U8K_EXT_X_BITRATE) == 0) {
		return M3U8_TAG_EXT_X_BITRATE;
	}
	
	if (strcmp(name, M3U8K_EXT_X_PART) == 0) {
		return M3U8_TAG_EXT_X_PART;
	}
	
	if (strcmp(name, M3U8K_EXT_X_SKIP) == 0) {
		return M3U8_TAG_EXT_X_SKIP;
	}
	
	if (strcmp(name, M3U8K_EXT_X_PRELOAD_HINT) == 0) {
		return M3U8_TAG_EXT_X_PRELOAD_HINT;
	}
	
	if (strcmp(name, M3U8K_EXT_X_RENDITION_REPORT) == 0) {
		return M3U8_TAG_EXT_X_RENDITION_REPORT;
	}
	
	return (enum M3U8TagType) 0;
	
}

static const char* m3u8attribute_stringify(const enum M3U8AttributeType type) {
	/*
	Get the string representation of this M3U8 attribute type.
	*/
	
	switch (type) {
		case M3U8_ATTRIBUTE_METHOD:
			return M3U8K_METHOD;
		case M3U8_ATTRIBUTE_URI:
			return M3U8K_URI;
		case M3U8_ATTRIBUTE_IV:
			return M3U8K_IV;
		case M3U8_ATTRIBUTE_KEYFORMAT:
			return M3U8K_KEYFORMAT;
		case M3U8_ATTRIBUTE_KEYFORMATVERSIONS:
			return M3U8K_KEYFORMATVERSIONS;
		case M3U8_ATTRIBUTE_BYTERANGE:
			return M3U8K_BYTERANGE;
		case M3U8_ATTRIBUTE_ID:
			return M3U8K_ID;
		case M3U8_ATTRIBUTE_CLASS:
			return M3U8K_CLASS;
		case M3U8_ATTRIBUTE_START_DATE:
			return M3U8K_START_DATE;
		case M3U8_ATTRIBUTE_END_DATE:
			return M3U8K_END_DATE;
		case M3U8_ATTRIBUTE_DURATION:
			return M3U8K_DURATION;
		case M3U8_ATTRIBUTE_PLANNED_DURATION:
			return M3U8K_PLANNED_DURATION;
		case M3U8_ATTRIBUTE_END_ON_NEXT:
			return M3U8K_END_ON_NEXT;
		case M3U8_ATTRIBUTE_TYPE:
			return M3U8K_TYPE;
		case M3U8_ATTRIBUTE_GROUP_ID:
			return M3U8K_GROUP_ID;
		case M3U8_ATTRIBUTE_LANGUAGE:
			return M3U8K_LANGUAGE;
		case M3U8_ATTRIBUTE_ASSOC_LANGUAGE:
			return M3U8K_ASSOC_LANGUAGE;
		case M3U8_ATTRIBUTE_NAME:
			return M3U8K_NAME;
		case M3U8_ATTRIBUTE_DEFAULT:
			return M3U8K_DEFAULT;
		case M3U8_ATTRIBUTE_AUTOSELECT:
			return M3U8K_AUTOSELECT;
		case M3U8_ATTRIBUTE_FORCED:
			return M3U8K_FORCED;
		case M3U8_ATTRIBUTE_INSTREAM_ID:
			return M3U8K_INSTREAM_ID;
		case M3U8_ATTRIBUTE_CHARACTERISTICS:
			return M3U8K_CHARACTERISTICS;
		case M3U8_ATTRIBUTE_CHANNELS:
			return M3U8K_CHANNELS;
		case M3U8_ATTRIBUTE_BANDWIDTH:
			return M3U8K_BANDWIDTH;
		case M3U8_ATTRIBUTE_PROGRAM_ID:
			return M3U8K_PROGRAM_ID;
		case M3U8_ATTRIBUTE_AVERAGE_BANDWIDTH:
			return M3U8K_AVERAGE_BANDWIDTH;
		case M3U8_ATTRIBUTE_CODECS:
			return M3U8K_CODECS;
		case M3U8_ATTRIBUTE_RESOLUTION:
			return M3U8K_RESOLUTION;
		case M3U8_ATTRIBUTE_FRAME_RATE:
			return M3U8K_FRAME_8RATE;
		case M3U8_ATTRIBUTE_HDCP_LEVEL:
			return M3U8K_HDCP_LEVEL;
		case M3U8_ATTRIBUTE_AUDIO:
			return M3U8K_AUDIO;
		case M3U8_ATTRIBUTE_VIDEO:
			return M3U8K_VIDEO;
		case M3U8_ATTRIBUTE_SUBTITLES:
			return M3U8K_SUBTITLES;
		case M3U8_ATTRIBUTE_CLOSED_CAPTIONS:
			return M3U8K_CLOSED_CAPTIONS;
		case M3U8_ATTRIBUTE_DATA_ID:
			return M3U8K_DATA_ID;
		case M3U8_ATTRIBUTE_VALUE:
			return M3U8K_VALUE;
		case M3U8_ATTRIBUTE_TIME_OFFSET:
			return M3U8K_TIME_OFFSET;
		case M3U8_ATTRIBUTE_PRECISE:
			return M3U8K_PRECISE;
		case M3U8_ATTRIBUTE_IMPORT:
			return M3U8K_IMPORT;
		case M3U8_ATTRIBUTE_QUERYPARAM:
			return M3U8K_QUERYPARAM;
		case M3U8_ATTRIBUTE_PART_TARGET:
			return M3U8K_PART_TARGET;
		case M3U8_ATTRIBUTE_CAN_SKIP_UNTIL:
			return M3U8K_CAN_SKIP_UNTIL;
		case M3U8_ATTRIBUTE_CAN_SKIP_DATERANGES:
			return M3U8K_CAN_SKIP_DATERANGES;
		case M3U8_ATTRIBUTE_HOLD_BACK:
			return M3U8K_HOLD_BACK;
		case M3U8_ATTRIBUTE_PART_HOLD_BACK:
			return M3U8K_PART_HOLD_BACK;
		case M3U8_ATTRIBUTE_CAN_BLOCK_RELOAD:
			return M3U8K_CAN_BLOCK_RELOAD;
		case M3U8_ATTRIBUTE_INDEPENDENT:
			return M3U8K_INDEPENDENT;
		case M3U8_ATTRIBUTE_GAP:
			return M3U8K_GAP;
		case M3U8_ATTRIBUTE_CUE:
			return M3U8K_CUE;
		case M3U8_ATTRIBUTE_SKIPPED_SEGMENTS:
			return M3U8K_SKIPPED_SEGMENTS;
		case M3U8_ATTRIBUTE_RECENTLY_REMOVED_DATERANGES:
			return M3U8K_RECENTLY_REMOVED_DATERANGES;
		case M3U8_ATTRIBUTE_BYTERANGE_START:
			return M3U8K_BYTERANGE_START;
		case M3U8_ATTRIBUTE_BYTERANGE_LENGTH:
			return M3U8K_BYTERANGE_LENGTH;
		case M3U8_ATTRIBUTE_LAST_MSN:
			return M3U8K_LAST_MSN;
		case M3U8_ATTRIBUTE_LAST_PART:
			return M3U8K_LAST_PART;
		case M3U8_ATTRIBUTE_STABLE_RENDITION_ID:
			return M3U8K_STABLE_RENDITION_ID;
		case M3U8_ATTRIBUTE_BIT_DEPTH:
			return M3U8K_BIT_DEPTH;
		case M3U8_ATTRIBUTE_SAMPLE_RATE:
			return M3U8K_SAMPLE_RATE;
		case M3U8_ATTRIBUTE_SCORE:
			return M3U8K_SCORE;
		case M3U8_ATTRIBUTE_SUPPLEMENTAL_CODECS:
			return M3U8K_SUPPLEMENTAL_CODECS;
		case M3U8_ATTRIBUTE_ALLOWED_CPC:
			return M3U8K_ALLOWED_CPC;
		case M3U8_ATTRIBUTE_VIDEO_RANGE:
			return M3U8K_VIDEO_RANGE;
		case M3U8_ATTRIBUTE_STABLE_VARIANT_ID:
			return M3U8K_STABLE_VARIANT_ID;
		case M3U8_ATTRIBUTE_PATHWAY_ID:
			return M3U8K_PATHWAY_ID;
		case M3U8_ATTRIBUTE_REQ_VIDEO_LAYOUT:
			return M3U8K_REQ_VIDEO_LAYOUT;
		case M3U8_ATTRIBUTE_X_CUSTOM:
			return NULL;
	}
	
	return NULL;
	
}

static enum M3U8AttributeType m3u8attribute_unstringify(const char* const name) {
	/*
	Get the equivalent M3U8 attribute type for this string.
	*/
	
	if (strcmp(name, M3U8K_METHOD) == 0) {
		return M3U8_ATTRIBUTE_METHOD;
	}
	
	if (strcmp(name, M3U8K_URI) == 0) {
		return M3U8_ATTRIBUTE_URI;
	}
	
	if (strcmp(name, M3U8K_IV) == 0) {
		return M3U8_ATTRIBUTE_IV;
	}
	
	if (strcmp(name, M3U8K_KEYFORMAT) == 0) {
		return M3U8_ATTRIBUTE_KEYFORMAT;
	}
	
	if (strcmp(name, M3U8K_KEYFORMATVERSIONS) == 0) {
		return M3U8_ATTRIBUTE_KEYFORMATVERSIONS;
	}
	
	if (strcmp(name, M3U8K_BYTERANGE) == 0) {
		return M3U8_ATTRIBUTE_BYTERANGE;
	}
	
	if (strcmp(name, M3U8K_ID) == 0) {
		return M3U8_ATTRIBUTE_ID;
	}
	
	if (strcmp(name, M3U8K_CLASS) == 0) {
		return M3U8_ATTRIBUTE_CLASS;
	}
	
	if (strcmp(name, M3U8K_START_DATE) == 0) {
		return M3U8_ATTRIBUTE_START_DATE;
	}
	
	if (strcmp(name, M3U8K_END_DATE) == 0) {
		return M3U8_ATTRIBUTE_END_DATE;
	}
	
	if (strcmp(name, M3U8K_DURATION) == 0) {
		return M3U8_ATTRIBUTE_DURATION;
	}
	
	if (strcmp(name, M3U8K_PLANNED_DURATION) == 0) {
		return M3U8_ATTRIBUTE_PLANNED_DURATION;
	}
	
	if (strcmp(name, M3U8K_END_ON_NEXT) == 0) {
		return M3U8_ATTRIBUTE_END_ON_NEXT;
	}
	
	if (strcmp(name, M3U8K_TYPE) == 0) {
		return M3U8_ATTRIBUTE_TYPE;
	}
	
	if (strcmp(name, M3U8K_GROUP_ID) == 0) {
		return M3U8_ATTRIBUTE_GROUP_ID;
	}
	
	if (strcmp(name, M3U8K_LANGUAGE) == 0) {
		return M3U8_ATTRIBUTE_LANGUAGE;
	}
	
	if (strcmp(name, M3U8K_ASSOC_LANGUAGE) == 0) {
		return M3U8_ATTRIBUTE_ASSOC_LANGUAGE;
	}
	
	if (strcmp(name, M3U8K_NAME) == 0) {
		return M3U8_ATTRIBUTE_NAME;
	}
	
	if (strcmp(name, M3U8K_DEFAULT) == 0) {
		return M3U8_ATTRIBUTE_DEFAULT;
	}
	
	if (strcmp(name, M3U8K_AUTOSELECT) == 0) {
		return M3U8_ATTRIBUTE_AUTOSELECT;
	}
	
	if (strcmp(name, M3U8K_FORCED) == 0) {
		return M3U8_ATTRIBUTE_FORCED;
	}
	
	if (strcmp(name, M3U8K_INSTREAM_ID) == 0) {
		return M3U8_ATTRIBUTE_INSTREAM_ID;
	}
	
	if (strcmp(name, M3U8K_CHARACTERISTICS) == 0) {
		return M3U8_ATTRIBUTE_CHARACTERISTICS;
	}
	
	if (strcmp(name, M3U8K_CHANNELS) == 0) {
		return M3U8_ATTRIBUTE_CHANNELS;
	}
	
	if (strcmp(name, M3U8K_BANDWIDTH) == 0) {
		return M3U8_ATTRIBUTE_BANDWIDTH;
	}
	
	if (strcmp(name, M3U8K_PROGRAM_ID) == 0) {
		return M3U8_ATTRIBUTE_PROGRAM_ID;
	}
	
	if (strcmp(name, M3U8K_AVERAGE_BANDWIDTH) == 0) {
		return M3U8_ATTRIBUTE_AVERAGE_BANDWIDTH;
	}
	
	if (strcmp(name, M3U8K_CODECS) == 0) {
		return M3U8_ATTRIBUTE_CODECS;
	}
	
	if (strcmp(name, M3U8K_RESOLUTION) == 0) {
		return M3U8_ATTRIBUTE_RESOLUTION;
	}
	
	if (strcmp(name, M3U8K_FRAME_8RATE) == 0) {
		return M3U8_ATTRIBUTE_FRAME_RATE;
	}
	
	if (strcmp(name, M3U8K_HDCP_LEVEL) == 0) {
		return M3U8_ATTRIBUTE_HDCP_LEVEL;
	}
	
	if (strcmp(name, M3U8K_AUDIO) == 0) {
		return M3U8_ATTRIBUTE_AUDIO;
	}
	
	if (strcmp(name, M3U8K_VIDEO) == 0) {
		return M3U8_ATTRIBUTE_VIDEO;
	}
	
	if (strcmp(name, M3U8K_SUBTITLES) == 0) {
		return M3U8_ATTRIBUTE_SUBTITLES;
	}
	
	if (strcmp(name, M3U8K_CLOSED_CAPTIONS) == 0) {
		return M3U8_ATTRIBUTE_CLOSED_CAPTIONS;
	}
	
	if (strcmp(name, M3U8K_DATA_ID) == 0) {
		return M3U8_ATTRIBUTE_DATA_ID;
	}
	
	if (strcmp(name, M3U8K_VALUE) == 0) {
		return M3U8_ATTRIBUTE_VALUE;
	}
	
	if (strcmp(name, M3U8K_TIME_OFFSET) == 0) {
		return M3U8_ATTRIBUTE_TIME_OFFSET;
	}
	
	if (strcmp(name, M3U8K_PRECISE) == 0) {
		return M3U8_ATTRIBUTE_PRECISE;
	}
	
	if (strcmp(name, M3U8K_IMPORT) == 0) {
		return M3U8_ATTRIBUTE_IMPORT;
	}
	
	if (strcmp(name, M3U8K_QUERYPARAM) == 0) {
		return M3U8_ATTRIBUTE_QUERYPARAM;
	}
	
	if (strcmp(name, M3U8K_PART_TARGET) == 0) {
		return M3U8_ATTRIBUTE_PART_TARGET;
	}
	
	if (strcmp(name, M3U8K_CAN_SKIP_UNTIL) == 0) {
		return M3U8_ATTRIBUTE_CAN_SKIP_UNTIL;
	}
	
	if (strcmp(name, M3U8K_CAN_SKIP_DATERANGES) == 0) {
		return M3U8_ATTRIBUTE_CAN_SKIP_DATERANGES;
	}
	
	if (strcmp(name, M3U8K_HOLD_BACK) == 0) {
		return M3U8_ATTRIBUTE_HOLD_BACK;
	}
	
	if (strcmp(name, M3U8K_PART_HOLD_BACK) == 0) {
		return M3U8_ATTRIBUTE_PART_HOLD_BACK;
	}
	
	if (strcmp(name, M3U8K_CAN_BLOCK_RELOAD) == 0) {
		return M3U8_ATTRIBUTE_CAN_BLOCK_RELOAD;
	}
	
	if (strcmp(name, M3U8K_INDEPENDENT) == 0) {
		return M3U8_ATTRIBUTE_INDEPENDENT;
	}
	
	if (strcmp(name, M3U8K_GAP) == 0) {
		return M3U8_ATTRIBUTE_GAP;
	}
	
	if (strcmp(name, M3U8K_CUE) == 0) {
		return M3U8_ATTRIBUTE_CUE;
	}
	
	if (strcmp(name, M3U8K_SKIPPED_SEGMENTS) == 0) {
		return M3U8_ATTRIBUTE_SKIPPED_SEGMENTS;
	}
	
	if (strcmp(name, M3U8K_RECENTLY_REMOVED_DATERANGES) == 0) {
		return M3U8_ATTRIBUTE_RECENTLY_REMOVED_DATERANGES;
	}
	
	if (strcmp(name, M3U8K_BYTERANGE_START) == 0) {
		return M3U8_ATTRIBUTE_BYTERANGE_START;
	}
	
	if (strcmp(name, M3U8K_BYTERANGE_LENGTH) == 0) {
		return M3U8_ATTRIBUTE_BYTERANGE_LENGTH;
	}
	
	if (strcmp(name, M3U8K_LAST_MSN) == 0) {
		return M3U8_ATTRIBUTE_LAST_MSN;
	}
	if (strcmp(name, M3U8K_LAST_PART) == 0) {
		return M3U8_ATTRIBUTE_LAST_PART;
	}
	
	if (strcmp(name, M3U8K_STABLE_RENDITION_ID) == 0) {
		return M3U8_ATTRIBUTE_STABLE_RENDITION_ID;
	}
	
	if (strcmp(name, M3U8K_BIT_DEPTH) == 0) {
		return M3U8_ATTRIBUTE_BIT_DEPTH;
	}
	
	if (strcmp(name, M3U8K_SAMPLE_RATE) == 0) {
		return M3U8_ATTRIBUTE_SAMPLE_RATE;
	}
	
	if (strcmp(name, M3U8K_SCORE) == 0) {
		return M3U8_ATTRIBUTE_SCORE;
	}
	
	if (strcmp(name, M3U8K_SUPPLEMENTAL_CODECS) == 0) {
		return M3U8_ATTRIBUTE_SUPPLEMENTAL_CODECS;
	}
	
	if (strcmp(name, M3U8K_ALLOWED_CPC) == 0) {
		return M3U8_ATTRIBUTE_ALLOWED_CPC;
	}
	
	if (strcmp(name, M3U8K_REQ_VIDEO_LAYOUT) == 0) {
		return M3U8_ATTRIBUTE_REQ_VIDEO_LAYOUT;
	}
	
	if (strcmp(name, M3U8K_VIDEO_RANGE) == 0) {
		return M3U8_ATTRIBUTE_VIDEO_RANGE;
	}
	
	if (strcmp(name, M3U8K_STABLE_VARIANT_ID) == 0) {
		return M3U8_ATTRIBUTE_STABLE_VARIANT_ID;
	}
	
	if (strcmp(name, M3U8K_PATHWAY_ID) == 0) {
		return M3U8_ATTRIBUTE_PATHWAY_ID;
	}
	
	if (strncmp(name, "X-", 2) == 0) {
		return M3U8_ATTRIBUTE_X_CUSTOM;
	}
	
	return 0;
	
}

static enum M3U8VTagType m3u8tag_getvtype(const struct M3U8Tag tag) {
	/*
	Get the type of the value for this M3U8 tag.
	*/
	
	switch (tag.type) {
		case M3U8_TAG_EXTM3U:
		case M3U8_TAG_EXT_X_DISCONTINUITY:
		case M3U8_TAG_EXT_X_ENDLIST:
		case M3U8_TAG_EXT_X_I_FRAMES_ONLY:
		case M3U8_TAG_EXT_X_INDEPENDENT_SEGMENTS:
		case M3U8_TAG_EXT_X_GAP:
			return M3U8_VTAG_NONE;
		case M3U8_TAG_EXT_X_VERSION:
		case M3U8_TAG_EXT_X_BYTERANGE:
		case M3U8_TAG_EXT_X_PROGRAM_DATE_TIME:
		case M3U8_TAG_EXT_X_ALLOW_CACHE:
		case M3U8_TAG_EXT_X_TARGETDURATION:
		case M3U8_TAG_EXT_X_MEDIA_SEQUENCE:
		case M3U8_TAG_EXT_X_DISCONTINUITY_SEQUENCE:
		case M3U8_TAG_EXT_X_PLAYLIST_TYPE:
		case M3U8_TAG_EXT_X_BITRATE:
			return M3U8_VTAG_SINGLE_VALUE;
		case M3U8_TAG_EXT_X_KEY:
		case M3U8_TAG_EXT_X_MAP:
		case M3U8_TAG_EXT_X_DATERANGE:
		case M3U8_TAG_EXT_X_MEDIA:
		case M3U8_TAG_EXT_X_STREAM_INF:
		case M3U8_TAG_EXT_X_I_FRAME_STREAM_INF:
		case M3U8_TAG_EXT_X_SESSION_DATA:
		case M3U8_TAG_EXT_X_SESSION_KEY:
		case M3U8_TAG_EXT_X_START:
		case M3U8_TAG_EXT_X_DEFINE:
		case M3U8_TAG_EXT_X_PART_INF:
		case M3U8_TAG_EXT_X_SERVER_CONTROL:
		case M3U8_TAG_EXT_X_PART:
		case M3U8_TAG_EXT_X_SKIP:
		case M3U8_TAG_EXT_X_PRELOAD_HINT:
		case M3U8_TAG_EXT_X_RENDITION_REPORT:
			return M3U8_VTAG_ATTRIBUTE_LIST;
		case M3U8_TAG_EXTINF:
			return M3U8_VTAG_LIST;
	}
	
	return M3U8_VTAG_TYPE_UNKNOWN;
	
}

static ssize_t m3u8tag_getmaxoffset(const struct M3U8Tag* const tag) {
	
	switch (tag->type) {
		case M3U8_TAG_EXT_X_VERSION:
		case M3U8_TAG_EXT_X_BYTERANGE:
		case M3U8_TAG_EXT_X_PROGRAM_DATE_TIME:
		case M3U8_TAG_EXT_X_ALLOW_CACHE:
		case M3U8_TAG_EXT_X_PLAYLIST_TYPE:
		case M3U8_TAG_EXTM3U:
		case M3U8_TAG_EXT_X_TARGETDURATION:
		case M3U8_TAG_EXT_X_MEDIA_SEQUENCE:
		case M3U8_TAG_EXT_X_DISCONTINUITY_SEQUENCE:
		case M3U8_TAG_EXT_X_BITRATE:
			return 0;
		case M3U8_TAG_EXTINF:
			return 1;
		default: {
			break;
		}
	}
	
	return -1;
	
}

static enum M3U8VItemType m3u8tag_getvitem(const struct M3U8Tag* const tag, const size_t index) {
	/*
	Get the type of the value for this M3U8 item.
	
	Most M3U8 items accepts only one type of value, while
	some others support multiple types.
	*/
	
	switch (tag->type) {
		case M3U8_TAG_EXT_X_VERSION: {
			switch (index) {
				case 0: {
					return M3U8_VITEM_TYPE_UINT;
				}
			}
			
			break;
		}
		case M3U8_TAG_EXT_X_BYTERANGE: {
			switch (index) {
				case 0: {
					return M3U8_VITEM_TYPE_BRANGE;
				}
			}
			
			break;
		}
		case M3U8_TAG_EXT_X_PROGRAM_DATE_TIME: {
			switch (index) {
				case 0: {
					return M3U8_VITEM_TYPE_DTIME;
				}
			}
			
			break;
		}
		case M3U8_TAG_EXT_X_ALLOW_CACHE:
		case M3U8_TAG_EXT_X_PLAYLIST_TYPE: {
			switch (index) {
				case 0: {
					return M3U8_VITEM_TYPE_ESTRING;
				}
			}
			
			break;
		}
		case M3U8_TAG_EXTM3U:
		case M3U8_TAG_EXT_X_TARGETDURATION:
		case M3U8_TAG_EXT_X_MEDIA_SEQUENCE:
		case M3U8_TAG_EXT_X_DISCONTINUITY_SEQUENCE:
		case M3U8_TAG_EXT_X_BITRATE: {
			switch (index) {
				case 0: {
					return M3U8_VITEM_TYPE_UINT;
				}
			}
			
			break;
		}
		case M3U8_TAG_EXTINF: {
			switch (index) {
				case 0: {
					return (enum M3U8VItemType) (M3U8_VITEM_TYPE_UINT | M3U8_VITEM_TYPE_UFLOAT);
				}
				case 1: {
					return M3U8_VITEM_TYPE_USTRING;
				}
			}
			
			break;
		}
		default: {
			break;
		}
	}
	
	return M3U8_VITEM_TYPE_UNKNOWN;
	
}

static enum M3U8VAttrType m3u8tag_getvattr(const struct M3U8Tag* const tag, const struct M3U8Attribute* const attribute) {
	/*
	Get the type of the value for this M3U8 attribute.
	
	Most M3U8 attributes accepts only one type of value, while
	some others support multiple types.
	*/
	
	switch (tag->type) {
		case M3U8_TAG_EXTM3U: {
			break;
		}
		case M3U8_TAG_EXT_X_VERSION: {
			break;
		}
		case M3U8_TAG_EXTINF: {
			break;
		}
		case M3U8_TAG_EXT_X_BYTERANGE: {
			break;
		}
		case M3U8_TAG_EXT_X_DISCONTINUITY: {
			break;
		}
		case M3U8_TAG_EXT_X_KEY: {
			switch (attribute->type) {
				case M3U8_ATTRIBUTE_METHOD: {
					/*
					The value is an enumerated-string that specifies the encryption
					method. This attribute is REQUIRED.
					*/
					return M3U8_VATTR_TYPE_ESTRING;
				}
				case M3U8_ATTRIBUTE_URI: {
					/*
					The value is a quoted-string containing a URI that specifies how
					to obtain the key. This attribute is REQUIRED unless the METHOD
					is NONE.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_IV: {
					/*
					The value is a hexadecimal-sequence that specifies a 128-bit
					unsigned integer Initialization Vector to be used with the key.
					*/
					return M3U8_VATTR_TYPE_HEXSEQ;
				}
				case M3U8_ATTRIBUTE_KEYFORMAT: {
					/*
					The value is a quoted-string that specifies how the key is
					represented in the resource identified by the URI. This attribute
					is OPTIONAL; its absence indicates an implicit value of "identity".
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_KEYFORMATVERSIONS: {
					/*
					The value is a quoted-string containing one or more positive
					integers separated by the "/" character (for example, "1", "1/2",
					or "1/2/5"). This attribute is OPTIONAL; if it is not present, its
					value is considered to be "1".
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				default: {
					break;
				}
			}
			
			break;
		}
		case M3U8_TAG_EXT_X_MAP: {
			switch (attribute->type) {
				case M3U8_ATTRIBUTE_URI: {
					/*
					The value is a quoted-string containing a URI that identifies a
					resource that contains the Media Initialization Section. This
					attribute is REQUIRED.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_BYTERANGE: {
					/*
					The value is a quoted-string specifying a byte range into the
					resource identified by the URI attribute. This attribute is
					OPTIONAL; if it is not present, the byte range is the entire
					resource indicated by the URI.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				default: {
					break;
				}
			}
			
			break;
		}
		case M3U8_TAG_EXT_X_PROGRAM_DATE_TIME: {
			break;
		}
		case M3U8_TAG_EXT_X_ALLOW_CACHE: {
			break;
		}
		case M3U8_TAG_EXT_X_DATERANGE: {
			switch (attribute->type) {
				case M3U8_ATTRIBUTE_ID: {
					/*
					A quoted-string that uniquely identifies a Date Range in the
					Playlist. This attribute is REQUIRED.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_CLASS: {
					/*
					A client-defined quoted-string that specifies some set of
					attributes and their associated value semantics. All Date Ranges
					with the same CLASS attribute value MUST adhere to these
					semantics. This attribute is OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_START_DATE: {
					/*
					A quoted-string containing the ISO-8601 date at which the Date
					Range begins. This attribute is REQUIRED.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_CUE: {
					/*
					An enumerated-string-list of Trigger Identifiers.  The list
					collectively indicates when to trigger an action associated with
					the Date Range. This attribute is OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_END_DATE: {
					/*
					A quoted-string containing the ISO-8601 date at which the Date
					Range ends. It MUST be equal to or later than the value of the
					START-DATE attribute. This attribute is OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_DURATION: {
					/*
					The duration of the Date Range expressed as a decimal-floating-
					point number of seconds. It MUST NOT be negative. A single
					instant in time (e.g., crossing a finish line) SHOULD be
					represented with a duration of 0. This attribute is OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_UFLOAT;
				}
				case M3U8_ATTRIBUTE_PLANNED_DURATION: {
					/*
					The expected duration of the Date Range expressed as a decimal-
					floating-point number of seconds. It MUST NOT be negative. This
					attribute SHOULD be used to indicate the expected duration of a
					Date Range whose actual duration is not yet known. It is OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_UFLOAT;
				}
				case M3U8_ATTRIBUTE_X_CUSTOM: {
					/*
					The "X-" prefix defines a namespace reserved for client-defined
					attributes. The attribute value MUST be a quoted-string,
					a hexadecimal-sequence, or a decimal-floating-point.
					These attributes are OPTIONAL.
					*/
					return (enum M3U8VAttrType) (M3U8_VATTR_TYPE_QSTRING | M3U8_VATTR_TYPE_HEXSEQ | M3U8_VATTR_TYPE_UFLOAT);
				}
				case M3U8_ATTRIBUTE_END_ON_NEXT: {
					/*
					An enumerated-string whose value MUST be YES. This attribute
					indicates that the end of the range containing it is equal to the
					START-DATE of its Following Range. The Following Range is the
					Date Range of the same CLASS that has the earliest START-DATE
					after the START-DATE of the range in question. This attribute is
					OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_ESTRING;
				}
				default: {
					break;
				}
			}
			
			break;
		}
		case M3U8_TAG_EXT_X_TARGETDURATION: {
			break;
		}
		case M3U8_TAG_EXT_X_MEDIA_SEQUENCE: {
			break;
		}
		case M3U8_TAG_EXT_X_DISCONTINUITY_SEQUENCE: {
			break;
		}
		case M3U8_TAG_EXT_X_ENDLIST: {
			break;
		}
		case M3U8_TAG_EXT_X_PLAYLIST_TYPE: {
			break;
		}
		case M3U8_TAG_EXT_X_I_FRAMES_ONLY: {
			break;
		}
		case M3U8_TAG_EXT_X_MEDIA: {
			switch (attribute->type) {
				case M3U8_ATTRIBUTE_TYPE: {
					/*
					The value is an enumerated-string; valid strings are AUDIO, VIDEO,
					SUBTITLES, and CLOSED-CAPTIONS. This attribute is REQUIRED.
					*/
					return M3U8_VATTR_TYPE_ESTRING;
				}
				case M3U8_ATTRIBUTE_URI: {
					/*
					The value is a quoted-string containing a URI that identifies the
					Media Playlist file. This attribute is OPTIONAL.
					If the TYPE is CLOSED-CAPTIONS, the URI attribute MUST NOT be present.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_GROUP_ID: {
					/*
					The value is a quoted-string that specifies the group to which the
					Rendition belongs. This attribute is REQUIRED.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_LANGUAGE: {
					/*
					The value is a quoted-string containing one of the standard Tags
					for Identifying Languages [RFC5646], which identifies the primary
					language used in the Rendition. This attribute is OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_ASSOC_LANGUAGE: {
					/*
					The value is a quoted-string containing a language tag [RFC5646]
					that identifies a language that is associated with the Rendition.
					This attribute is OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_NAME: {
					/*
					The value is a quoted-string containing a human-readable
					description of the Rendition. This attribute is REQUIRED.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_STABLE_RENDITION_ID: {
					/*
					The value is a quoted-string which is a stable identifier for the
					URI within the Multivariant Playlist. This attribute is OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_DEFAULT: {
					/*
					The value is an enumerated-string; valid strings are YES and NO.
					This attribute is OPTIONAL. Its absence indicates an implicit value of NO.
					*/
					return M3U8_VATTR_TYPE_ESTRING;
				}
				case M3U8_ATTRIBUTE_AUTOSELECT: {
					/*
					The value is an enumerated-string; valid strings are YES and NO.
					This attribute is OPTIONAL. Its absence indicates an implicit value of NO.
					*/
					return M3U8_VATTR_TYPE_ESTRING;
				}
				case M3U8_ATTRIBUTE_FORCED: {
					/*
					The value is an enumerated-string; valid strings are YES and NO.
					This attribute is OPTIONAL. Its absence indicates an implicit value of NO. 
					*/
					return M3U8_VATTR_TYPE_ESTRING;
				}
				case M3U8_ATTRIBUTE_INSTREAM_ID: {
					/*
					The value is a quoted-string that specifies a Rendition within the
					segments in the Media Playlist. This attribute is REQUIRED if the
					TYPE attribute is CLOSED-CAPTIONS.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_BIT_DEPTH: {
					/*
					The value is a non-negative decimal-integer specifying the audio
					bit depth of the Rendition. This attribute is OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_UINT;
				}
				case M3U8_ATTRIBUTE_SAMPLE_RATE: {
					/*
					The value is a non-negative decimal-integer specifying the audio
					sample rate of the Rendition. This attribute is OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_UINT;
				}
				case M3U8_ATTRIBUTE_CHARACTERISTICS: {
					/*
					The value is a quoted-string containing one or more Uniform Type
					Identifiers [UTI] separated by comma (,) characters. This attribute is
					OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_CHANNELS: {
					/*
					The value is a quoted-string that specifies an ordered, backslash-
					separated ("/") list of parameters.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				default: {
					break;
				}
			}
			
			break;
		}
		case M3U8_TAG_EXT_X_STREAM_INF: {
			switch (attribute->type) {
				case M3U8_ATTRIBUTE_BANDWIDTH: {
					/*
					The value is a decimal-integer of bits per second. It represents
					the peak segment bit rate of the Variant Stream. This attribute is
					REQUIRED.
					*/
					return M3U8_VATTR_TYPE_UINT;
				}
				case M3U8_ATTRIBUTE_PROGRAM_ID: {
					/*
					The value is a decimal-integer that uniquely identifies a particular
					presentation within the scope of the Playlist file. This attribute is
					REQUIRED. This attribute was removed in protocol version 6.
					*/
					return M3U8_VATTR_TYPE_UINT;
				}
				case M3U8_ATTRIBUTE_AVERAGE_BANDWIDTH: {
					/*
					The value is a decimal-integer of bits per second. It represents
					the average segment bit rate of the Variant Stream. This attribute is
					OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_UINT;
				}
				case M3U8_ATTRIBUTE_CODECS: {
					/*
					The value is a quoted-string containing a comma-separated list of
					formats. This attribute is REQUIRED.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_RESOLUTION: {
					/*
					The value is a decimal-resolution describing the optimal pixel
					resolution at which to display all the video in the Variant Stream.
					This attribute is OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_RESOLUTION;
				}
				case M3U8_ATTRIBUTE_FRAME_RATE: {
					/*
					The value is a decimal-floating-point describing the maximum frame
					rate for all the video in the Variant Stream, rounded to three
					decimal places. This attribute is OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_UFLOAT;
				}
				case M3U8_ATTRIBUTE_HDCP_LEVEL: {
					/*
					The value is an enumerated-string; valid strings are TYPE-0 and
					NONE. This attribute is OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_ESTRING;
				}
				case M3U8_ATTRIBUTE_AUDIO: {
					/*
					The value is a quoted-string. It MUST match the value of the
					GROUP-ID attribute of an EXT-X-MEDIA tag elsewhere in the Master
					Playlist whose TYPE attribute is AUDIO. This attribute is OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_VIDEO: {
					/*
					The value is a quoted-string. It MUST match the value of the
					GROUP-ID attribute of an EXT-X-MEDIA tag elsewhere in the Master
					Playlist whose TYPE attribute is VIDEO. This attribute is OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_SUBTITLES: {
					/*
					The value is a quoted-string. It MUST match the value of the
					GROUP-ID attribute of an EXT-X-MEDIA tag elsewhere in the Master
					Playlist whose TYPE attribute is SUBTITLES. This attribute is OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_CLOSED_CAPTIONS: {
					/*
					The value can be either a quoted-string or an enumerated-string
					with the value NONE. This attribute is OPTIONAL.
					*/
					return (enum M3U8VAttrType) (M3U8_VATTR_TYPE_QSTRING | M3U8_VATTR_TYPE_ESTRING);
				}
				case M3U8_ATTRIBUTE_SCORE: {
					return M3U8_VATTR_TYPE_UFLOAT;
				}
				case M3U8_ATTRIBUTE_SUPPLEMENTAL_CODECS: {
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_ALLOWED_CPC: {
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_VIDEO_RANGE: {
					return M3U8_VATTR_TYPE_ESTRING;
				}
				case M3U8_ATTRIBUTE_REQ_VIDEO_LAYOUT: {
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_STABLE_VARIANT_ID: {
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_PATHWAY_ID: {
					return M3U8_VATTR_TYPE_QSTRING;
				}
				default: {
					break;
				}
			}
			
			break;
		}
		case M3U8_TAG_EXT_X_I_FRAME_STREAM_INF: {
			/*
			All attributes defined for the EXT-X-STREAM-INF tag are also defined for
			the EXT-X-I-FRAME-STREAM-INF tag, except for the FRAME-RATE, AUDIO,
			SUBTITLES, and CLOSED-CAPTIONS attributes. In addition, the attribute URI
			is also defined.
			*/
			
			switch (attribute->type) {
				case M3U8_ATTRIBUTE_URI: {
					/*
					The value is a quoted-string containing a URI that identifies the
					I-frame Media Playlist file. This attribute is REQUIRED.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_BANDWIDTH: {
					return M3U8_VATTR_TYPE_UINT;
				}
				case M3U8_ATTRIBUTE_PROGRAM_ID: {
					return M3U8_VATTR_TYPE_UINT;
				}
				case M3U8_ATTRIBUTE_AVERAGE_BANDWIDTH: {
					return M3U8_VATTR_TYPE_UINT;
				}
				case M3U8_ATTRIBUTE_CODECS: {
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_RESOLUTION: {
					return M3U8_VATTR_TYPE_RESOLUTION;
				}
				case M3U8_ATTRIBUTE_HDCP_LEVEL: {
					return M3U8_VATTR_TYPE_ESTRING;
				}
				case M3U8_ATTRIBUTE_VIDEO: {
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_SCORE: {
					return M3U8_VATTR_TYPE_UFLOAT;
				}
				case M3U8_ATTRIBUTE_SUPPLEMENTAL_CODECS: {
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_ALLOWED_CPC: {
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_VIDEO_RANGE: {
					return M3U8_VATTR_TYPE_ESTRING;
				}
				case M3U8_ATTRIBUTE_REQ_VIDEO_LAYOUT: {
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_STABLE_VARIANT_ID: {
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_PATHWAY_ID: {
					return M3U8_VATTR_TYPE_QSTRING;
				}
				default: {
					break;
				}
			}
			
			break;
		}
		case M3U8_TAG_EXT_X_SESSION_DATA: {
			switch (attribute->type) {
				case M3U8_ATTRIBUTE_DATA_ID: {
					/*
					The value of DATA-ID is a quoted-string that identifies a
					particular data value. This attribute is REQUIRED.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_VALUE: {
					/*
					VALUE is a quoted-string.  It contains the data identified by
					DATA-ID. This attribute is REQUIRED.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_URI: {
					/*
					The value is a quoted-string containing a URI. This attribute is
					REQUIRED.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_LANGUAGE: {
					/*
					The value is a quoted-string containing a language tag [RFC5646]
					that identifies the language of the VALUE. This attribute is OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				default: {
					break;
				}
			}
			
			break;
		}
		case M3U8_TAG_EXT_X_SESSION_KEY: {
			/*
			All attributes defined for the EXT-X-KEY tag are also defined for the
			EXT-X-SESSION-KEY, except that the value of the METHOD attribute
			MUST NOT be NONE.
			*/
			
			switch (attribute->type) {
				case M3U8_ATTRIBUTE_METHOD: {
					return M3U8_VATTR_TYPE_ESTRING;
				}
				case M3U8_ATTRIBUTE_URI: {
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_IV: {
					return M3U8_VATTR_TYPE_HEXSEQ;
				}
				case M3U8_ATTRIBUTE_KEYFORMAT: {
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_KEYFORMATVERSIONS: {
					return M3U8_VATTR_TYPE_QSTRING;
				}
				default: {
					break;
				}
			}
			
			break;
		}
		case M3U8_TAG_EXT_X_INDEPENDENT_SEGMENTS: {
			break;
		}
		case M3U8_TAG_EXT_X_START: {
			switch (attribute->type) {
				case M3U8_ATTRIBUTE_TIME_OFFSET: {
					/*
					The value of TIME-OFFSET is a signed-decimal-floating-point number
					of seconds. This attribute is REQUIRED.
					*/
					return M3U8_VATTR_TYPE_FLOAT;
				}
				case M3U8_ATTRIBUTE_PRECISE: {
					/*
					The value is an enumerated-string; valid strings are YES and NO.
					This attribute is OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_ESTRING;
				}
				default: {
					break;
				}
			}
			
			break;
		}
		case M3U8_TAG_EXT_X_DEFINE: {
			switch (attribute->type) {
				case M3U8_ATTRIBUTE_NAME: {
					/*
					The value is a quoted-string which specifies the Variable Name.
					All characters in the quoted-string MUST be from the following
					set: [a..z], [A..Z], [0..9], '-', and '_'. This attribute is OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_VALUE: {
					/*
					The value is a quoted-string which specifies the Variable Value.
					This attribute is REQUIRED if the EXT-X-DEFINE tag has a NAME
					attribute.  The quoted-string MAY be empty. This attribute is REQUIRED.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_IMPORT: {
					/*
					The value is a quoted-string which specifies the Variable Name and
					indicates that its value is that of the variable of the same name
					in the Multivariant Playlist. This attribute is OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_QUERYPARAM: {
					/*
					The value is a quoted-string which specifies the Variable Name and
					indicates that its value is the value associated with the query
					parameter of the same name in the URI of the Playlist. This attribute is OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				default: {
					break;
				}
			}
			
			break;
		}
		case M3U8_TAG_EXT_X_PART_INF: {
			switch (attribute->type) {
				case M3U8_ATTRIBUTE_PART_TARGET: {
					/*
					The value is a decimal-floating-point number of seconds indicating
					the Part Target Duration. This attribute is REQUIRED.
					*/
					return M3U8_VATTR_TYPE_UFLOAT;
				}
				default: {
					break;
				}
			}
			
			break;
		}
		case M3U8_TAG_EXT_X_SERVER_CONTROL: {
			switch (attribute->type) {
				case M3U8_ATTRIBUTE_CAN_SKIP_UNTIL: {
					/*
					Indicates that the Server can produce Playlist Delta Updates
					(Section 6.2.5.1) in response to the _HLS_skip Delivery Directive.
					Its value is the Skip Boundary, a decimal-floating-point number of
					seconds. This attribute is OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_UFLOAT;
				}
				case M3U8_ATTRIBUTE_CAN_SKIP_DATERANGES: {
					/*
					The value is an enumerated-string whose value is YES if the Server
					can produce Playlist Delta Updates (Section 6.2.5.1) that skip
					older EXT-X-DATERANGE tags in addition to Media Segments.
					This attribute is OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_ESTRING;
				}
				case M3U8_ATTRIBUTE_HOLD_BACK: {
					/*
					The value is a decimal-floating-point number of seconds that
					indicates the server-recommended minimum distance from the end of
					the Playlist at which clients should begin to play or to which
					they should seek, unless PART-HOLD-BACK applies. This attribute is OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_UFLOAT;
				}
				case M3U8_ATTRIBUTE_PART_HOLD_BACK: {
					/*
					The value is a decimal-floating-point number of seconds that
					indicates the server-recommended minimum distance from the end of
					the Playlist at which clients should begin to play or to which
					they should seek when playing in Low-Latency Mode.
					PART-HOLD-BACK is REQUIRED if the Playlist contains the EXT-X-
					PART-INF tag.
					*/
					return M3U8_VATTR_TYPE_UFLOAT;
				}
				case M3U8_ATTRIBUTE_CAN_BLOCK_RELOAD: {
					/*
					The value is an enumerated-string whose value is YES if the server
					supports Blocking Playlist Reload (Section 6.2.5.2). This
					attribute is OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_ESTRING;
				}
				default: {
					break;
				}
			}
			
			break;
		}
		case M3U8_TAG_EXT_X_GAP: {
			break;
		}
		case M3U8_TAG_EXT_X_BITRATE: {
			break;
		}
		case M3U8_TAG_EXT_X_PART: {
			switch (attribute->type) {
				case M3U8_ATTRIBUTE_URI: {
					/*
					The value is a quoted-string containing the URI for the Partial
					Segment. This attribute is REQUIRED.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_DURATION: {
					/*
					The value is the duration of the Partial Segment as a decimal-
					floating-point number of seconds. This attribute is REQUIRED.
					*/
					return M3U8_VATTR_TYPE_UFLOAT;
				}
				case M3U8_ATTRIBUTE_INDEPENDENT: {
					/*
					The value is an enumerated-string whose value is YES if the
					Partial Segment contains an independent frame.  This attribute is
					OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_ESTRING;
				}
				case M3U8_ATTRIBUTE_BYTERANGE: {
					/*
					Indicates that the Partial Segment is a sub-range of the resource
					specified by the URI attribute. This attribute is OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_GAP: {
					/*
					The value is an enumerated-string whose value is YES if the
					Partial Segment is not available. It is REQUIRED for such Partial
					Segments.
					*/
					return M3U8_VATTR_TYPE_ESTRING;
				}
				default: {
					break;
				}
			}
			
			break;
		}
		case M3U8_TAG_EXT_X_SKIP: {
			switch (attribute->type) {
				case M3U8_ATTRIBUTE_SKIPPED_SEGMENTS: {
					/*
					The value is a decimal-integer specifying the number of Media
					Segments replaced by the EXT-X-SKIP tag. This attribute is
					REQUIRED.
					*/
					return M3U8_VATTR_TYPE_UINT;
				}
				case M3U8_ATTRIBUTE_RECENTLY_REMOVED_DATERANGES: {
					/*
					The value is a quoted-string consisting of a tab (0x9) delimited
					list of EXT-X-DATERANGE IDs that have been removed from the
					Playlist recently. This attribute is REQUIRED if the Client requested
					an update that skips EXT-X-DATERANGE tags.  
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				default: {
					break;
				}
			}
			
			break;
		}
		case M3U8_TAG_EXT_X_PRELOAD_HINT: {
			switch (attribute->type) {
				case M3U8_ATTRIBUTE_TYPE: {
					/*
					The value is an enumerated-string that specifies the type of the
					hinted resource. This attribute is REQUIRED.
					*/
					return M3U8_VATTR_TYPE_ESTRING;
				}
				case M3U8_ATTRIBUTE_URI: {
					/*
					The value is a quoted-string containing a URI identifying the
 					hinted resource. This attribute is REQUIRED.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_BYTERANGE_START: {
					/*
					The value is a decimal-integer specifying the byte offset of the
	 				first byte of the hinted resource, from the beginning of the
 					resource identified by the URI attribute. This attribute is
					OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_UINT;
				}
				case M3U8_ATTRIBUTE_BYTERANGE_LENGTH: {
					/*
					The value is a decimal-integer specifying the length of the hinted
					resource. This attribute is OPTIONAL.
					*/
					return M3U8_VATTR_TYPE_UINT;
				}
				default: {
					break;
				}
			}
			
			break;
		}
		case M3U8_TAG_EXT_X_RENDITION_REPORT: {
			switch (attribute->type) {
				case M3U8_ATTRIBUTE_URI: {
					/*
					The value is a quoted-string containing the URI for the Media
					Playlist of the specified Rendition. It MUST be relative to the
					URI of the Media Playlist containing the EXT-X-RENDITION-REPORT
					tag. This attribute is REQUIRED.
					*/
					return M3U8_VATTR_TYPE_QSTRING;
				}
				case M3U8_ATTRIBUTE_LAST_MSN: {
					/*
					The value is a decimal-integer specifying the Media Sequence
					Number of the last Media Segment currently in the specified
					Rendition. If the Rendition contains Partial Segments then this
					value is the Media Sequence Number of the last Partial Segment.
					This attribute is REQUIRED.
					*/
					return M3U8_VATTR_TYPE_UINT;
				}
				case M3U8_ATTRIBUTE_LAST_PART: {
					/*
					The value is a decimal-integer that indicates the Part Index of
					the last Partial Segment currently in the specified Rendition
					whose Media Sequence Number is equal to the LAST-MSN attribute
					value. This attribute is REQUIRED if the Rendition contains a
					Partial Segment.
					*/
					return M3U8_VATTR_TYPE_UINT;
				}
				default: {
					break;
				}
			}
			
			break;
		}
	}
	
	return M3U8_VATTR_TYPE_UNKNOWN;
	
}

const char* m3u8attribute_getkey(const struct M3U8Attribute* const attribute) {
	/*
	Get the string representation of the key of this M3U8 attribute.
	*/
	
	if (attribute->type == M3U8_ATTRIBUTE_X_CUSTOM) {
		return attribute->key;
	}
	
	return m3u8attribute_stringify(attribute->type);
	
}

static int m3u8attr_parsev(
	struct M3U8Attribute* const attribute,
	const enum M3U8VAttrType vtype,
	const char* const value
) {
	
	int status = 0;
	
	switch (vtype) {
		case M3U8_VATTR_TYPE_UINT: {
			status = m3u8parser_getuint(value, &attribute->value);
			break;
		}
		case M3U8_VATTR_TYPE_HEXSEQ: {
			status = m3u8parser_gethexseq(value, &attribute->value);
			break;
		}
		case M3U8_VATTR_TYPE_UFLOAT: {
			status = m3u8parser_getufloat(value, &attribute->value);
			break;
		}
		case M3U8_VATTR_TYPE_FLOAT: {
			status = m3u8parser_getfloat(value, &attribute->value);
			break;
		}
		case M3U8_VATTR_TYPE_QSTRING: {
			status = m3u8parser_getqstring(value, &attribute->value);
			break;
		}
		case M3U8_VATTR_TYPE_ESTRING: {
			status = m3u8parser_getestring(value, &attribute->value);
			break;
		}
		case M3U8_VATTR_TYPE_RESOLUTION: {
			status = m3u8parser_getresolution(value, &attribute->value);
			break;
		}
	}
	
	switch (status) {
		case M3U8ERR_PARSER_INVALID_UINT:
			return M3U8ERR_ATTRIBUTE_INVALID_UINT;
		case M3U8ERR_PARSER_INVALID_UFLOAT:
			return M3U8ERR_ATTRIBUTE_INVALID_UFLOAT;
		case M3U8ERR_PARSER_INVALID_HEXSEQ:
			return M3U8ERR_ATTRIBUTE_INVALID_HEXSEQ;
		case M3U8ERR_PARSER_INVALID_FLOAT:
			return M3U8ERR_ATTRIBUTE_INVALID_FLOAT;
		case M3U8ERR_PARSER_INVALID_ESTRING:
			return M3U8ERR_ATTRIBUTE_INVALID_ESTRING;
		case M3U8ERR_PARSER_INVALID_QSTRING:
			return M3U8ERR_ATTRIBUTE_INVALID_QSTRING;
		case M3U8ERR_PARSER_INVALID_RESOLUTION:
			return M3U8ERR_ATTRIBUTE_INVALID_RESOLUTION;
	}
	
	return status;
	
	return M3U8ERR_SUCCESS;
	
}

static int m3u8attr_vparse(
	const struct M3U8Tag* const tag,
	struct M3U8Attribute* const attribute,
	const char* const value
) {
	/*
	Parse the string pointed to by 'value' as a M3U8 attribute value.
	
	This function performs syntax and type checks to ensure that the
	value is valid.
	*/
	
	size_t index = 0;
	
	const enum M3U8VAttrType vtype = m3u8tag_getvattr(tag, attribute);
	
	for (index = 0; index < sizeof(M3U8_VATTR_TYPES) / sizeof(*M3U8_VATTR_TYPES); index++) {
		const enum M3U8VAttrType type = M3U8_VATTR_TYPES[index];
		
		if (vtype & type) {
			const int status = m3u8attr_parsev(attribute, type, value);
			
			if (status == M3U8ERR_SUCCESS) {
				attribute->vtype = type;
				break;
			}
			
			if (status == M3U8ERR_MEMORY_ALLOCATE_FAILURE) {
				return status;
			}
			
			continue;
		}
	}
	
	if (attribute->value == NULL) {
		return M3U8ERR_ATTRIBUTE_VALUE_INVALID;
	}
	
	return M3U8ERR_SUCCESS;
	
}

static int m3u8item_parsev(
	struct M3U8Item* const item,
	const enum M3U8VItemType vtype,
	const char* const value
) {
	
	int status = 0;
	
	switch (vtype) {
		case M3U8_VITEM_TYPE_UINT:
			status = m3u8parser_getuint(value, &item->value);
			break;
		case M3U8_VITEM_TYPE_UFLOAT:
			status = m3u8parser_getufloat(value, &item->value);
			break;
		case M3U8_VITEM_TYPE_BRANGE:
			status = m3u8parser_getbrange(value, &item->value);
			break;
		case M3U8_VITEM_TYPE_DTIME:
			status = m3u8parser_getdtime(value, &item->value);
			break;
		case M3U8_VITEM_TYPE_ESTRING:
			status = m3u8parser_getestring(value, &item->value);
			break;
		case M3U8_VITEM_TYPE_USTRING:
			status = m3u8parser_getustring(value, &item->value);
			break;
	}
	
	switch (status) {
		case M3U8ERR_PARSER_INVALID_UINT:
			return M3U8ERR_ITEM_INVALID_UINT;
		case M3U8ERR_PARSER_INVALID_UFLOAT:
			return M3U8ERR_ITEM_INVALID_UFLOAT;
		case M3U8ERR_PARSER_INVALID_BRANGE:
			return M3U8ERR_ITEM_INVALID_BRANGE;
		case M3U8ERR_PARSER_INVALID_DTIME:
			return M3U8ERR_ITEM_INVALID_DTIME;
		case M3U8ERR_PARSER_INVALID_ESTRING:
			return M3U8ERR_ITEM_INVALID_ESTRING;
		case M3U8ERR_PARSER_INVALID_USTRING:
			return M3U8ERR_ITEM_INVALID_USTRING;
	}
	
	return status;
	
}

static int m3u8item_vparse(
	const struct M3U8Tag* const tag,
	struct M3U8Item* const item,
	const char* const value,
	const size_t position
) {
	/*
	Parse the string pointed to by 'value' as a M3U8 item value.
	
	This function performs syntax and type checks to ensure that the
	value is valid.
	*/
	
	size_t index = 0;
	
	const enum M3U8VItemType vtype = m3u8tag_getvitem(tag, position);
	
	for (index = 0; index < sizeof(M3U8_VITEM_TYPES) / sizeof(*M3U8_VITEM_TYPES); index++) {
		const enum M3U8VItemType type = M3U8_VITEM_TYPES[index];
		
		if (vtype & type) {
			const int status = m3u8item_parsev(item, type, value);
			
			if (status == M3U8ERR_SUCCESS) {
				item->vtype = type;
				break;
			}
			
			if (status == M3U8ERR_MEMORY_ALLOCATE_FAILURE) {
				return status;
			}
			
			continue;
		}
	}
	
	if (item->value == NULL) {
		return M3U8ERR_ITEM_VALUE_INVALID;
	}
	
	return M3U8ERR_SUCCESS;
	
}

static enum M3U8PlaylistType m3u8playlist_guess_type(struct M3U8Playlist* const playlist) {
	/*
	Guess whether this playlist is a Media Playlist or Master Playlist.
	
	The RFC 8216 does not specify a standard way of distinguishing between
	media and master playlists so all we can do is to guess it based on the tags
	within the playlist.
	
	The tags available for media playlists are:
	
	- M3U8-TAG-EXTINF
	- EXT-X-BYTERANGE
	- EXT-X-DISCONTINUITY
	- EXT-X-KEY
	- EXT-X-MAP
	- EXT-X-PROGRAM-DATE-TIME
	- EXT-X-ALLOW-CACHE
	- EXT-X-DATERANGE
	- EXT-X-TARGETDURATION
	- EXT-X-MEDIA-SEQUENCE
	- EXT-X-DISCONTINUITY-SEQUENCE
	- EXT-X-ENDLIST
	- EXT-X-PLAYLIST-TYPE
	- EXT-X-I-FRAMES-ONLY
	
	The tags available for master playlists are:
	
	- EXT-X-MEDIA
	- EXT-X-STREAM-INF
	- EXT-X-I-FRAME-STREAM-INF
	- EXT-X-SESSION-DATA
	- EXT-X-SESSION-KEY
	*/
	
	size_t index = 0;
	
	for (index = 0; index < playlist->tags.offset; index++) {
		const struct M3U8Tag* const tag = &playlist->tags.items[index];
		
		switch (tag->type) {
			case M3U8_TAG_EXTINF:
			case M3U8_TAG_EXT_X_BYTERANGE:
			case M3U8_TAG_EXT_X_DISCONTINUITY:
			case M3U8_TAG_EXT_X_KEY:
			case M3U8_TAG_EXT_X_MAP:
			case M3U8_TAG_EXT_X_PROGRAM_DATE_TIME:
			case M3U8_TAG_EXT_X_ALLOW_CACHE:
			case M3U8_TAG_EXT_X_DATERANGE:
			case M3U8_TAG_EXT_X_TARGETDURATION:
			case M3U8_TAG_EXT_X_MEDIA_SEQUENCE:
			case M3U8_TAG_EXT_X_DISCONTINUITY_SEQUENCE:
			case M3U8_TAG_EXT_X_ENDLIST:
			case M3U8_TAG_EXT_X_PLAYLIST_TYPE:
			case M3U8_TAG_EXT_X_I_FRAMES_ONLY:
				return M3U8_PLAYLIST_TYPE_MEDIA;
			case M3U8_TAG_EXT_X_MEDIA:
			case M3U8_TAG_EXT_X_STREAM_INF:
			case M3U8_TAG_EXT_X_I_FRAME_STREAM_INF:
			case M3U8_TAG_EXT_X_SESSION_DATA:
			case M3U8_TAG_EXT_X_SESSION_KEY:
				return M3U8_PLAYLIST_TYPE_MASTER;
			default:
				break;
		}
	}
	
	return 0;
	
}

static int m3u8tag_expects_uri(const struct M3U8Tag* const tag) {
	
	const int status = (
		tag->type == M3U8_TAG_EXT_X_KEY ||
		tag->type == M3U8_TAG_EXT_X_STREAM_INF ||
		tag->type == M3U8_TAG_EXTINF
	);
	
	return status;
	
}

int m3u8_parse(
	struct M3U8Playlist* const playlist,
	const char* const s
) {
	/*
	Parse the string pointed to by 's' following the RFC 8216 (HTTP Live Streaming) standard.
	*/
	const int strict=0;
	char line[M3U8_MAX_LINE_LEN];
	char tag_name[M3U8_MAX_TAG_NAME_LEN];
	char attr_value[M3U8_MAX_ATTR_VALUE_LEN];
	char attr_name[M3U8_MAX_ATTR_NAME_LEN];
	char item_value[M3U8_MAX_ITEM_VALUE_LEN];
	
	strsplit_t split = {0};
	strsplit_part_t current_line = {0};
	
	struct M3U8Tag tag = {0};
	struct M3U8Item item = {0};
	struct M3U8Attribute attribute = {0};
	
	struct M3U8Tag* tags = NULL;
	struct M3U8Attribute* attributes = NULL;
	struct M3U8Item* items = NULL;
	
	enum M3U8TagType current = M3U8_TAG_TYPE_UNKNOWN;
	
	int type = 0, err = 0;
	size_t index = 0, size = 0;
	
	const char *start = NULL, *end = NULL, *lend = NULL;
	char* pos = NULL;
	
	strsplit_init(&split, s, "\n");
	
	while (1) {
		if (strsplit_next(&split, &current_line) == NULL) {
			break;
		}
		
		if (current_line.size == 0) {
			continue;
		}
		
		if ((current_line.size + 1) > sizeof(line)) {
			err = M3U8ERR_PLAYLIST_LINE_TOO_LONG;
			goto end;
		}
		
		memcpy(line, current_line.begin, current_line.size);
		
		line[current_line.size] = '\0';
		
		if (line[0] == '#') {
			type = (memcmp(line + 1, "EXT", 3) == 0) ? M3U8_PLAYLIST_TAG : M3U8_PLAYLIST_COMMENT;
		} else {
			type = M3U8_PLAYLIST_URI;
		}
		
		/*
		RFC 8216 states that the first line in the playlist must be the 'EXTM3U' tag; neither
		comments nor URIs are allowed before this.
		*/
		if (current_line.index == 0 && type != M3U8_PLAYLIST_TAG) {
			err = M3U8ERR_PLAYLIST_UNEXPECTED_ITEM;
			goto end;
		}
		
		switch (type) {
			case M3U8_PLAYLIST_TAG: {
				unsigned char ch = 0;
				
				/*
				In RFC 8216, Section 8.6, it is stated that M3U8 tags can continue on the next line by
				specifying a backslash ('\') at the end of the line
				*/
				while (current_line.begin[current_line.size - 1] == '\\') {
					if (strsplit_next(&split, &current_line) == NULL) {
						return M3U8ERR_PLAYLIST_LINE_UNTERMINATED;
					}
					
					current_line.index--;
					
					pos = line + (strlen(line) - 1);
					
					/* Remove the backslash from the line. */
					*pos = '\0';
					
					/* Remove trailing whitespaces. */
					while (pos != line) {
						pos--;
						
						ch = *pos;
						
						if (!isspace(ch)) {
							break;
						}
						
						*pos = '\0';
					}
					
					pos++;
					
					if ((strlen(line) + current_line.size + 1) > sizeof(line)) {
						err = M3U8ERR_PLAYLIST_LINE_TOO_LONG;
						goto end;
					}
					
					memcpy(pos, current_line.begin, current_line.size);
					pos[current_line.size] = '\0';
				}
				
				start = line + 1;
				end = strstr(start, ":");
				
				if (end == NULL) {
					end = strchr(start, '\0');
				}
				
				size = (size_t) (end - start);
				
				if ((size + 1) > sizeof(tag_name)) {
					err = M3U8ERR_TAG_NAME_INVALID;
					goto end;
				}
				
				memcpy(tag_name, start, size);
				tag_name[size] = '\0';
				
				/*
				RFC 8216 states that an tag name is only valid if it contains characters
				from the set [A-Z], [0-9], and '-'.
				*/
				if (!namesafe(tag_name)) {
					err = M3U8ERR_TAG_NAME_INVALID;
					goto end;
				}
				
				tag.type = m3u8tag_unstringify(tag_name);
				
				if (tag.type == 0) {
					err = M3U8ERR_TAG_NAME_INVALID;
					goto end;
				}
				
				/*
				RFC 8216 states that the 'EXTM3U' tag must be the first line of every
				Media Playlist and Master Playlist.
				*/
				if (current_line.index == 0 && tag.type != M3U8_TAG_EXTM3U) {
					err = M3U8ERR_PLAYLIST_UNEXPECTED_TAG;
					goto end;
				}
				
				tag.vtype = m3u8tag_getvtype(tag);
				
				lend = strchr(line, '\0');
				
				/*
				According to RFC 8216, an M3U8 tag may or may not require a value to be supplied after it.
				For those tags for which a value is required, there are basically three types of values accepted:
				
				1. A list of key=value pairs delimited with a comma (e.g., "X-EXT-EXAMPLE:key=value,anotherkey=anothervalue").
				2. A list of items delimited with a comma (e.g., "X-EXT-EXAMPLE:9999,Some string").
				3. A single-value one (e.g., "X-EXT-EXAMPLE:value").
				*/
				switch (tag.vtype) {
					case M3U8_VTAG_NONE: {
						if (end != lend) {
							err = M3U8ERR_TAG_TRAILING_OPTIONS;
							goto end;
						}
						
						break;
					}
					case M3U8_VTAG_ATTRIBUTE_LIST: {
						strsplit_t split = {0};
						strsplit_part_t part = {0};
						
						strsplit_t subsplit = {0};
						strsplit_part_t subpart = {0};
						
						if (end == lend) {
							err = M3U8ERR_TAG_MISSING_ATTRIBUTES;
							goto end;
						}
						
						end++;
						
						strsplit_init(&split, end, ",");
						
						while (strsplit_next(&split, &part) != NULL) {
							if (part.size == 0) {
								err = M3U8ERR_ATTRIBUTE_EMPTY;
								goto end;
							}
							
							strsplit_init(&subsplit, part.begin, "=");
							
							/* Parse attribute name */
							strsplit_next(&subsplit, &subpart);
							
							if (subpart.begin == NULL) {
								err = M3U8ERR_ATTRIBUTE_MISSING_NAME;
								goto end;
							}
							
							strsplit_resize(&split, &subpart);
							
							if (subpart.size == 0) {
								err = M3U8ERR_ATTRIBUTE_MISSING_NAME;
								goto end;
							}
							
							if ((subpart.size + 1) > M3U8_MAX_ATTR_NAME_LEN) {
								err = M3U8ERR_ATTRIBUTE_NAME_TOO_LONG;
								goto end;
							}
							
							memcpy(attr_name, subpart.begin, subpart.size);
							attr_name[subpart.size] = '\0';
							
							if (!namesafe(attr_name)) {
								err = M3U8ERR_ATTRIBUTE_INVALID_NAME;
								goto end;
							}
							
							attribute.type = m3u8attribute_unstringify(attr_name);
							
							if (attribute.type == M3U8_ATTRIBUTE_TYPE_UNKNOWN) {
								err = M3U8ERR_ATTRIBUTE_INVALID_NAME;
								goto end;
							}
							
							if (attribute.type == M3U8_ATTRIBUTE_X_CUSTOM) {
								attribute.key = malloc(size + 1);
								
								if (attribute.key == NULL) {
									err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
									goto end;
								}
								
								strcpy(attribute.key, attr_name);
							}
							
							/*
							RFC 8216 states that there must not have duplicate attributes
							within the same tag.
							*/
							for (index = 0; index < tag.attributes.offset; index++) {
								const struct M3U8Attribute* const attr = &tag.attributes.items[index];
								
								if (attr->type == attribute.type) {
									if (attr->type == M3U8_ATTRIBUTE_X_CUSTOM) {
										const char* const fkey = m3u8attribute_getkey(attr);
										const char* const skey = m3u8attribute_getkey(&attribute);
										
										if (strcmp(fkey, skey) != 0) {
											continue;
										}
									}
									
									err = M3U8ERR_ATTRIBUTE_DUPLICATE;
									goto end;
								}
							}
							
							/* Parse attribute value */
							strsplit_next(&subsplit, &subpart);
							
							if (subpart.begin == NULL) {
								err = M3U8ERR_ATTRIBUTE_MISSING_VALUE;
								goto end;
							}
							
							strsplit_resize(&split, &subpart);
							
							if (subpart.size == 0) {
								err = M3U8ERR_ATTRIBUTE_MISSING_VALUE;
								goto end;
							}
							
							if (*subpart.begin == '"') {
								end = strstr(subpart.begin + 1, "\"");
								
								if (end == NULL || end > subsplit.cur_pend) {
									err = M3U8ERR_ATTRIBUTE_INVALID_QSTRING;
									goto end;
								}
								
								end++;
								
								subpart.size = (size_t) (end - subpart.begin);
								
								while (split.pbegin != NULL && (subpart.begin + subpart.size) > split.pbegin) {
									strsplit_next(&split, &part);
								}
							}
							
							if (subpart.size < 1) {
								err = M3U8ERR_ATTRIBUTE_MISSING_VALUE;
								goto end;
							}
							
							if ((subpart.size + 1) > M3U8_MAX_ATTR_VALUE_LEN) {
								err = M3U8ERR_ATTRIBUTE_VALUE_TOO_LONG;
								goto end;
							}
							
							memcpy(attr_value, subpart.begin, subpart.size);
							attr_value[subpart.size] = '\0';
							
							err = m3u8attr_vparse(&tag, &attribute, attr_value);
							
							if (err != M3U8ERR_SUCCESS) {
								goto end;
							}
							
							size = tag.attributes.size + sizeof(*tag.attributes.items) * 1;
							attributes = realloc(tag.attributes.items, size);
							
							if (attributes == NULL) {
								err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
								goto end;
							}
							
							tag.attributes.items = attributes;
							tag.attributes.size = size;
							
							tag.attributes.items[tag.attributes.offset++] = attribute;
							
							memset(&attribute, 0, sizeof(attribute));
						}
						
						break;
					}
					case M3U8_VTAG_LIST: {
						strsplit_t split = {0};
						strsplit_part_t part = {0};
						
						const ssize_t maxoffset = m3u8tag_getmaxoffset(&tag);
						
						if (end == lend) {
							err = M3U8ERR_TAG_MISSING_ITEMS;
							goto end;
						}
						
						end++;
						
						strsplit_init(&split, end, ",");
						
						while (strsplit_next(&split, &part) != NULL) {
							if (part.size == 0) {
								continue;
							}
							
							if ((part.size + 1) > M3U8_MAX_ITEM_VALUE_LEN) {
								err = M3U8ERR_ITEM_VALUE_TOO_LONG;
								goto end;
							}
							
							memcpy(item_value, part.begin, part.size);
							item_value[part.size] = '\0';
							
							err = m3u8item_vparse(&tag, &item, item_value, part.index);
							
							if (err != M3U8ERR_SUCCESS) {
								goto end;
							}
							
							size = tag.items.size + sizeof(*tag.items.items) * 1;
							items = realloc(tag.items.items, size);
							
							if (items == NULL) {
								err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
								goto end;
							}
							
							tag.items.items = items;
							tag.items.size = size;
							
							tag.items.items[tag.items.offset++] = item;
							
							memset(&item, 0, sizeof(item));
							
							if ((ssize_t) (part.index + 1) > maxoffset) {
								break;
							}
						}
						
						break;
					}
					case M3U8_VTAG_SINGLE_VALUE: {
						if (end == lend) {
							err = M3U8ERR_TAG_MISSING_VALUE;
							goto end;
						}
						
						end++;
						
						start = end;
						end = lend;
						
						size = (size_t) (end - start);
						
						if (size < 1) {
							err = M3U8ERR_TAG_MISSING_VALUE;
							goto end;
						}
						
						if ((size + 1) > M3U8_MAX_ITEM_VALUE_LEN) {
							err = M3U8ERR_ITEM_VALUE_TOO_LONG;
							goto end;
						}
						
						memcpy(item_value, start, size);
						item_value[size] = '\0';
						
						err = m3u8item_vparse(&tag, &tag.value, item_value, 0);
						
						if (err != M3U8ERR_SUCCESS) {
							goto end;
						}
						
						break;
					}
				}
				
				/*
				There is nothing in RFC 8216 explicitly saying that these tags cannot appear multiple times in the same playlist file,
				but based on their common usage, I judged that they don't need to appear multiple times or would not make sense
				if they were specified multiple times.
				*/
				if ((tag.type == M3U8_TAG_EXT_X_VERSION || tag.type == M3U8_TAG_EXT_X_TARGETDURATION
					|| tag.type ==  M3U8_TAG_EXT_X_MEDIA_SEQUENCE || tag.type == M3U8_TAG_EXT_X_ENDLIST
					|| tag.type ==  M3U8_TAG_EXTM3U || tag.type ==  M3U8_TAG_EXT_X_PLAYLIST_TYPE
					|| tag.type == M3U8_TAG_EXT_X_I_FRAMES_ONLY) && (current & tag.type) != 0) {
					err = M3U8ERR_TAG_DUPLICATE;
					goto end;
				}
				
				current |= tag.type;
				
				size = playlist->tags.size + sizeof(*playlist->tags.items) * 1;
				tags = realloc(playlist->tags.items, size);
				
				if (tags == NULL) {
					err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
					goto end;
				}
				
				playlist->tags.items = tags;
				playlist->tags.size = size;
				
				playlist->tags.items[playlist->tags.offset++] = tag;
				
				memset(&tag, 0, sizeof(tag));
				
				break;
			}
			case M3U8_PLAYLIST_COMMENT: {
				break; /* Does anyone even read these comments?! */
			}
			case M3U8_PLAYLIST_URI: {
				const size_t position = playlist->tags.offset - 1;
				struct M3U8Tag* destination = &playlist->tags.items[position];
				
				/*
				Only the EXT-X-KEY, EXT-X-STREAM-INF and EXTINF tags are allowed to have an URI.
				*/
				if (!m3u8tag_expects_uri(destination)) {
					if ((destination->type == M3U8_TAG_EXT_X_BYTERANGE || destination->type == M3U8_TAG_EXT_X_BITRATE) && position > 0) {
						destination = &playlist->tags.items[position - 1];
						
						if (destination->type != M3U8_TAG_EXTINF) {
							err = M3U8ERR_PLAYLIST_UNEXPECTED_URI;
							goto end;
						}
					} else {
						err = M3U8ERR_PLAYLIST_UNEXPECTED_URI;
						goto end;
					}
				}
				
				destination->uri = malloc(sizeof(line));
				
				if (destination->uri == NULL) {
					err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
					goto end;
				}
				
				strcpy(destination->uri, line);
			}
		}
	}
	
	end:;
	
	if (err != M3U8ERR_SUCCESS) {
		m3u8playlist_free(playlist);
	}
	
	m3u8attribute_free(&attribute);
	m3u8item_free(&item);
	m3u8tag_free(&tag);
	
	if (err == M3U8ERR_SUCCESS) {
		playlist->type = m3u8playlist_guess_type(playlist);
		playlist->livestream = (
			playlist->type == M3U8_PLAYLIST_TYPE_MEDIA &&
			(current & M3U8_TAG_EXT_X_ENDLIST) == 0
		);
		
		if (playlist->type == M3U8_PLAYLIST_TYPE_UNKNOWN) {
			err = M3U8ERR_PLAYLIST_UNKNOWN_TYPE;
			m3u8playlist_free(playlist);
		}
	}
	
	return err;
	
}

void m3u8attribute_free(struct M3U8Attribute* const attribute) {
	/*
	Free the given M3U8 Attribute structure.
	*/
	
	free(attribute->key);
	attribute->key = NULL;
	
	if (attribute->vtype == M3U8_VATTR_TYPE_HEXSEQ) {
		struct M3U8Bytes* const bytes = attribute->value;
		
		free(bytes->data);
		bytes->data = NULL;
		bytes->offset = 0;
		bytes->size = 0;
	}
	
	free(attribute->value);
	attribute->value = NULL;
	
}

void m3u8item_free(struct M3U8Item* const item) {
	/*
	Free the given M3U8 Item structure.
	*/
	
	item->vtype = M3U8_VITEM_TYPE_UNKNOWN;
	
	free(item->value);
	item->value = NULL;
	
}

void m3u8tag_free(struct M3U8Tag* const tag) {
	/*
	Free the given M3U8 Tag structure.
	*/
	
	size_t index = 0;
	
	for (index = 0; index < tag->attributes.offset; index++) {
		struct M3U8Attribute* const attribute = &tag->attributes.items[index];
		m3u8attribute_free(attribute);
	}
	
	tag->attributes.offset = 0;
	tag->attributes.size = 0;
	
	free(tag->attributes.items);
	tag->attributes.items = NULL;
	
	for (index = 0; index < tag->items.offset; index++) {
		struct M3U8Item* const item = &tag->items.items[index];
		m3u8item_free(item);
	}
	
	tag->items.offset = 0;
	tag->items.size = 0;
	
	free(tag->items.items);
	tag->items.items = NULL;
	
	free(tag->uri);
	tag->uri = NULL;
	
	m3u8item_free(&tag->value);
	
}

void m3u8playlist_free(struct M3U8Playlist* const playlist) {
	/*
	Free the given M3U8 Playlist structure.
	*/
	
	size_t index = 0;
	
	for (index = 0; index < playlist->tags.offset; index++) {
		struct M3U8Tag* const tag = &playlist->tags.items[index];
		m3u8tag_free(tag);
	}
	
	playlist->tags.offset = 0;
	playlist->tags.size = 0;
	
	free(playlist->tags.items);
	playlist->tags.items = NULL;
	
	free(playlist->uri.uri);
	playlist->uri.uri = NULL;
	
	free(playlist->suburi.uri);
	playlist->suburi.uri = NULL;
	
	if (!playlist->subresource) {
		httpclient_free(&playlist->client);
		multihttpclient_free(&playlist->multi_client);
	}
	
	playlist->subresource = 0;
	playlist->livestream = 0;
	
}

static char* uri_ensure_slashes(char* const s) {
	
	size_t index = 0;
	
	for (index = 0; index < strlen(s); index++) {
		char* const ch = &s[index];
			
		if (*ch == '\\') {
			*ch = '/';
		}
	}
	
	return s;
	
}

struct M3U8Attribute* m3u8tag_sgetattr(
	const struct M3U8Tag* const tag,
	const char* const key
) {
	/*
	Looks for an M3U8 attribute matching the 'key' within the given M3U8 tag.
	*/
	
	size_t index = 0;
	
	for (index = 0; index < tag->attributes.offset; index++) {
		struct M3U8Attribute* const attribute = &tag->attributes.items[index];
		const char* const skey = m3u8attribute_getkey(attribute);
		
		if (strcmp(key, skey) == 0) {
			return attribute;
		}
	}
	
	return NULL;
	
}

struct M3U8Attribute* m3u8tag_igetattr(
	const struct M3U8Tag* const tag,
	enum M3U8AttributeType type
) {
	/*
	Looks for an M3U8 attribute matching the 'type' within the given M3U8 tag.
	
	This is the same as m3u8tag_sgetattr(), but instead looks for the M3U8 attribute
	by its type.
	*/
	
	size_t index = 0;
	
	for (index = 0; index < tag->attributes.offset; index++) {
		struct M3U8Attribute* const attribute = &tag->attributes.items[index];
		
		if (type == attribute->type) {
			return attribute;
		}
	}
	
	return NULL;
	
}

struct M3U8Tag* m3u8playlist_igettag(
	 const struct M3U8Playlist* const playlist,
	enum M3U8TagType type
) {
	/*
	Looks for an M3U8 tag matching the 'type' within the given M3U8 playlist.
	*/
	
	size_t index = 0;
	
	for (index = 0; index < playlist->tags.offset; index++) {
		struct M3U8Tag* const tag = &playlist->tags.items[index];
		
		if (type == tag->type) {
			return tag;
		}
	}
	
	return NULL;
	
}

void m3u8playlist_deltag_index(
	struct M3U8Playlist* const playlist,
	const size_t index
) {
	
	struct M3U8Tag* const tag = &playlist->tags.items[index];
	m3u8tag_free(tag);
	
	if ((index + 1) < playlist->tags.offset) {
		const struct M3U8Tag* const subtag = &playlist->tags.items[index + 1];
		const size_t size = sizeof(*playlist->tags.items) * (playlist->tags.offset - (index + 1));
		memmove(tag, subtag, size);
	}
	
	playlist->tags.offset--;
	
}

int m3u8playlist_ideltag(
	struct M3U8Playlist* const playlist,
	enum M3U8TagType type
) {
	/*
	Delete the M3U8 tag matching the 'type' within the given M3U8 playlist.
	*/
	
	size_t index = 0;
	
	for (index = 0; index < playlist->tags.offset; index++) {
		struct M3U8Tag* const tag = &playlist->tags.items[index];
		
		if (type != tag->type) {
			continue;
		}
		
		m3u8playlist_deltag_index(playlist, index);
		
		return 1;
	}
	
	return 0;
	
}

void m3u8tag_delattr_index(
	struct M3U8Tag* const tag,
	const size_t index
) {
	
	struct M3U8Attribute* const attribute = &tag->attributes.items[index];
	m3u8attribute_free(attribute);
	
	if ((index + 1) < tag->attributes.offset) {
		const struct M3U8Attribute* const subattribute = &tag->attributes.items[index + 1];
		const size_t size = sizeof(*tag->attributes.items) * (tag->attributes.offset - (index + 1));
		memmove(attribute, subattribute, size);
	}
	
	tag->attributes.offset--;
	
}

int m3u8tag_idelattr(
	struct M3U8Tag* const tag,
	enum M3U8AttributeType type
) {
	/*
	Delete the M3U8 attribute matching the 'type' within the given M3U8 tag.
	*/
	
	size_t index = 0;
	
	for (index = 0; index < tag->attributes.offset; index++) {
		const struct M3U8Attribute* const attribute = &tag->attributes.items[index];
		
		if (type != attribute->type) {
			continue;
		}
		
		m3u8tag_delattr_index(tag, index);
		
		return 1;
	}
	
	return 0;
	
}

int m3u8playlist_tagexists(
	 const struct M3U8Playlist* const playlist,
	enum M3U8TagType type
) {
	/*
	Checks whether the M3U8 tag matching the 'type' exists in the given M3U8 playlist.
	*/
	
	return (m3u8playlist_igettag(playlist, type) != NULL);
	
}

int m3u8attr_setvalue(
	struct M3U8Attribute* const attribute,
	const char* const value
) {
	/*
	Sets the value of a specified M3U8 attribute to the given value.
	*/
	
	free(attribute->value);
	
	attribute->value = malloc(strlen(value) + 1);
	
	if (attribute->value == NULL) {
		return M3U8ERR_MEMORY_ALLOCATE_FAILURE;
	}
	
	strcpy(attribute->value, value);
	
	return M3U8ERR_SUCCESS;
	
}

static int m3u8tag_setval(
	struct M3U8Tag* const tag,
	const int what,
	const void* const value
) {
	
	size_t index = 0;
	
	switch (what) {
		case M3U8TAG_SETVAL_URI: {
			free(tag->uri);
			
			tag->uri = malloc(strlen(value) + 1);
			
			if (tag->uri == NULL) {
				return M3U8ERR_MEMORY_ALLOCATE_FAILURE;
			}
			
			strcpy(tag->uri, value);
			
			break;
		}
		case M3U8TAG_SETVAL_VALUE: {
			const struct M3U8Item* const item = (const struct M3U8Item* const) value;
			const size_t vsize = m3u8item_getvsize(item);
			
			free(tag->value.value);
			
			tag->value.vtype = item->vtype;
			tag->value.value = malloc(vsize);
			
			if (tag->value.value == NULL) {
				return M3U8ERR_MEMORY_ALLOCATE_FAILURE;
			}
			
			memcpy(tag->value.value, item->value, vsize);
			
			break;
		}
		case M3U8TAG_SETVAL_ATTRIBUTES: {
			const struct M3U8Attributes* const attributes = (const struct M3U8Attributes* const) value;
			
			tag->attributes.size = attributes->size;
			tag->attributes.items = malloc(tag->attributes.size);
			
			if (tag->attributes.items == NULL) {
				return M3U8ERR_MEMORY_ALLOCATE_FAILURE;
			}
			
			for (index = 0; index < attributes->offset; index++) {
				const struct M3U8Attribute* const source = &attributes->items[index];
				const size_t vsize = m3u8attribute_getvsize(source);
				
				struct M3U8Attribute destination = {0};
				
				destination.type = source->type;
				destination.vtype = source->vtype;
				
				if (source->key != NULL) {
					destination.key = malloc(strlen(source->key) + 1);
					
					if (destination.key == NULL) {
						return M3U8ERR_MEMORY_ALLOCATE_FAILURE;
					}
					
					strcpy(destination.key, source->key);
				}
				
				memcpy(destination.value, source->value, vsize);
				
				tag->attributes.items[tag->attributes.offset++] = destination;
			}
			
			break;
		}
		case M3U8TAG_SETVAL_ITEMS: {
			const struct M3U8Items* const items = (const struct M3U8Items* const) value;
			
			tag->items.size = items->size;
			tag->items.items = malloc(tag->items.size);
			
			if (tag->items.items == NULL) {
				return M3U8ERR_MEMORY_ALLOCATE_FAILURE;
			}
			
			for (index = 0; index < items->offset; index++) {
				const struct M3U8Item* const source = &items->items[index];
				const size_t vsize = m3u8item_getvsize(source);
				
				struct M3U8Item destination;
				
				destination.vtype = source->vtype;
				destination.value = malloc(vsize);
				
				if (destination.value == NULL) {
					return M3U8ERR_MEMORY_ALLOCATE_FAILURE;
				}
				
				memcpy(destination.value, source->value, vsize);
				
				tag->items.items[tag->items.offset++] = destination;
			}
			
			break;
		}
	}
	
	return M3U8ERR_SUCCESS;
	
}

int m3u8tag_seturi(
	struct M3U8Tag* const tag,
	const void* const uri
) {
	/*
	Set the URI for this M3U8 tag.
	
	The content pointed to by 'uri' is copied to an internal buffer, so
    you don't have to keep the pointer alive after setting this value.
    
	On success, zero is returned. On error, a number lower than 0 is returned.
	*/
	
	const int status = m3u8tag_setval(tag, M3U8TAG_SETVAL_URI, uri);
	return status;
	
}

const char* m3u8tag_geturi(const struct M3U8Tag* const tag) {
	/*
	Get the URI for this M3U8 tag.
	
	Returns a null pointer if this value was not set.
	*/
	
	return tag->uri;
	
}

int m3u8tag_setvalue(
	struct M3U8Tag* const tag,
	const char* const value
) {
	/*
	Set the single-value option for this M3U8 tag.
	
	The content pointed to by 'value' is copied to an internal buffer,
    so you don't have to keep the pointer alive after setting this value.
    
	On success, zero is returned. On error, a number lower than 0 is returned.
	*/
	
	const int status = m3u8tag_setval(tag, M3U8TAG_SETVAL_VALUE, value);
	return status;
	
}

struct M3U8Item* m3u8tag_getitem(struct M3U8Tag* const tag, const size_t index) {
	/*
	Get a M3U8 item within the M3U8 tag by its index.
	
	Returns a null pointer if this value was not set.
	*/
	
	if (!(tag->vtype == M3U8_VTAG_SINGLE_VALUE || tag->vtype == M3U8_VTAG_LIST)) {
		return NULL;
	}
	
	if (tag->vtype == M3U8_VTAG_SINGLE_VALUE) {
		if (index > 0) {
			return NULL;
		}
		
		return &tag->value;
	}
	
	return (index < tag->items.offset ? &tag->items.items[index] : NULL);
	
}


int m3u8tag_setattributes(
	struct M3U8Tag* const tag,
	const struct M3U8Attributes* const attributes
) {
	/*
	Set the list of M3U8 attributes for this M3U8 tag.
	
	The content pointed to by 'attributes' is copied to an internal buffer,
    so you don't have to keep the pointer alive after setting this value.
    
	On success, zero is returned. On error, a number greater than 0 is returned.
	*/
	
	const int status = m3u8tag_setval(tag, M3U8TAG_SETVAL_ATTRIBUTES, attributes);
	return status;
	
}

const struct M3U8Attributes* m3u8tag_getattributes(const struct M3U8Tag* const tag) {
	/*
	Get the list of M3U8 attributes for this M3U8 tag.
	
	Returns a null pointer if the list of attributes is empty.
	*/
	
	return tag->attributes.offset > 0 ? &tag->attributes : NULL;
	
}

int m3u8_dump_callback(
	const struct M3U8Playlist* const playlist,
	int (*dump_callback)(const char* const, const size_t, void* const),
	void* const data
) {
	
	size_t index = 0;
	size_t subindex = 0;
	size_t subsubindex = 0;
	
	char tmp[M3U8_MAX_ATTR_VALUE_LEN + M3U8_MAX_ITEM_VALUE_LEN];
	
	for (index = 0; index < playlist->tags.offset; index++) {
		const struct M3U8Tag* const tag = &playlist->tags.items[index];
		const char* const name = m3u8tag_stringify(tag->type);
		
		if ((*dump_callback)("#", 1, data) == -1) {
			return M3U8ERR_CALLBACK_WRITE_FAILURE;
		}
		
		if ((*dump_callback)(name, strlen(name), data) == -1) {
			return M3U8ERR_CALLBACK_WRITE_FAILURE;
		}
		
		switch (tag->vtype) {
			case M3U8_VTAG_NONE: {
				break;
			}
			case M3U8_VTAG_ATTRIBUTE_LIST: {
				for (subindex = 0; subindex < tag->attributes.offset; subindex++) {
					const struct M3U8Attribute* const attribute = &tag->attributes.items[subindex];
					const char* const attribute_name = m3u8attribute_getkey(attribute);
					
					if ((*dump_callback)((subindex == 0) ? ":" : ",", 1, data) == -1) {
						return M3U8ERR_CALLBACK_WRITE_FAILURE;
					}
					
					if ((*dump_callback)(attribute_name, strlen(attribute_name), data) == -1) {
						return M3U8ERR_CALLBACK_WRITE_FAILURE;
					}
					
					if ((*dump_callback)("=", 1, data) == -1) {
						return M3U8ERR_CALLBACK_WRITE_FAILURE;
					}
					
					switch (attribute->vtype) {
						case M3U8_VATTR_TYPE_UINT: {
							const biguint_t value = *((biguint_t*) attribute->value);
							const int wsize = snprintf(tmp, sizeof(tmp), "%"FORMAT_BIGGEST_UINT_T, value);
							
							if (wsize < 1) {
								return M3U8ERR_PRINTF_WRITE_FAILURE;
							}
							
							if ((size_t) wsize > (sizeof(tmp) - 1)) {
								return M3U8ERR_BUFFER_OVERFLOW;
							}
							
							break;
						}
						case M3U8_VATTR_TYPE_HEXSEQ: {
							size_t offset = 0;
							
							const struct M3U8Bytes* const bytes = attribute->value;
							
							if ((2 + (bytes->size * 2)) > (sizeof(tmp) - 1)) {
								return M3U8ERR_BUFFER_OVERFLOW;
							}
							
							strcpy(tmp, "0x");
							offset += 2;
							
							for (subsubindex = 0; subsubindex < bytes->size; subsubindex++) {
								const unsigned char ch = bytes->data[subsubindex];
								
								tmp[offset++] = to_hex((ch & 0xF0) >> 4);
								tmp[offset++] = to_hex((ch & 0x0F) >> 0);
							}
							
							tmp[offset++] = '\0';
							
							break;
						}
						case M3U8_VATTR_TYPE_FLOAT:
						case M3U8_VATTR_TYPE_UFLOAT: {
							const bigfloat_t value = *((bigfloat_t*) attribute->value);
							const int wsize = snprintf(tmp, sizeof(tmp), "%"FORMAT_BIGGEST_FLOAT_T, value);
							
							if (wsize < 1) {
								return M3U8ERR_PRINTF_WRITE_FAILURE;
							}
							
							if ((size_t) wsize > (sizeof(tmp) - 1)) {
								return M3U8ERR_BUFFER_OVERFLOW;
							}
							
							break;
						}
						case M3U8_VATTR_TYPE_QSTRING: {
							if (attribute->type == M3U8_ATTRIBUTE_URI) {
								uri_ensure_slashes(attribute->value);
							}
							
							if ((strlen(attribute->value) + 2 + 1) > sizeof(tmp)) {
								return M3U8ERR_BUFFER_OVERFLOW;
							}
							
							strcpy(tmp, "\"");
							strcat(tmp, attribute->value);
							strcat(tmp, "\"");
							
							break;
						}
						case M3U8_VATTR_TYPE_ESTRING: {
							if ((strlen(attribute->value) + 1) > sizeof(tmp)) {
								return M3U8ERR_BUFFER_OVERFLOW;
							}
							
							strcpy(tmp, attribute->value);
							
							break;
						}
						case M3U8_VATTR_TYPE_RESOLUTION: {
							const struct M3U8Resolution* const resolution = (struct M3U8Resolution*) attribute->value;
							const int wsize = snprintf(tmp, sizeof(tmp), "%"FORMAT_BIGGEST_UINT_T"x%"FORMAT_BIGGEST_UINT_T, resolution->width, resolution->height);
							
							if (wsize < 1) {
								return M3U8ERR_PRINTF_WRITE_FAILURE;
							}
							
							if ((size_t) wsize > (sizeof(tmp) - 1)) {
								return M3U8ERR_BUFFER_OVERFLOW;
							}
							
							break;
						}
					}
					
					if ((*dump_callback)(tmp, strlen(tmp), data) == -1) {
						return M3U8ERR_CALLBACK_WRITE_FAILURE;
					}
					
				}
				
				break;
			}
			case M3U8_VTAG_SINGLE_VALUE:
			case M3U8_VTAG_LIST: {
				const size_t offset = (tag->vtype == M3U8_VTAG_LIST ? tag->items.offset : 1);
				
				for (subindex = 0; subindex < offset; subindex++) {
					const struct M3U8Item* const item = (
						tag->vtype == M3U8_VTAG_LIST ?
						&tag->items.items[subindex] :
						&tag->value
					);
					
					if ((*dump_callback)((subindex == 0) ? ":" : ",", 1, data) == -1) {
						return M3U8ERR_CALLBACK_WRITE_FAILURE;
					}
					
					switch (item->vtype) {
						case M3U8_VITEM_TYPE_UINT: {
							const biguint_t value = *((biguint_t*) item->value);
							const int wsize = snprintf(tmp, sizeof(tmp), "%"FORMAT_BIGGEST_UINT_T, value);
							
							if (wsize < 1) {
								return M3U8ERR_PRINTF_WRITE_FAILURE;
							}
							
							if ((size_t) wsize > (sizeof(tmp) - 1)) {
								return M3U8ERR_BUFFER_OVERFLOW;
							}
							
							break;
						}
						case M3U8_VITEM_TYPE_UFLOAT: {
							const bigfloat_t value = *((bigfloat_t*) item->value);
							const int wsize = snprintf(tmp, sizeof(tmp), "%"FORMAT_BIGGEST_FLOAT_T, value);
							
							if (wsize < 1) {
								return M3U8ERR_PRINTF_WRITE_FAILURE;
							}
							
							if ((size_t) wsize > (sizeof(tmp) - 1)) {
								return M3U8ERR_BUFFER_OVERFLOW;
							}
							
							break;
						}
						case M3U8_VITEM_TYPE_BRANGE: {
							const struct M3U8ByteRange* const range = (struct M3U8ByteRange*) item->value;
							
							size_t bsize = sizeof(tmp);
							int wsize = 0;
							
							wsize = snprintf(tmp, bsize, "%"FORMAT_BIGGEST_UINT_T, range->length);
							
							if (wsize < 1) {
								return M3U8ERR_PRINTF_WRITE_FAILURE;
							}
							
							if ((size_t) wsize > (bsize - 1)) {
								return M3U8ERR_BUFFER_OVERFLOW;
							}
							
							if (range->offset == 0) {
								break;
							}
							
							bsize -= (size_t) wsize;
							
							if ((bsize - 1) < 1) {
								return M3U8ERR_BUFFER_OVERFLOW;
							}
							
							strcat(tmp, "@");
							
							bsize--;
							
							wsize = snprintf(tmp + strlen(tmp), bsize, "%"FORMAT_BIGGEST_UINT_T, range->offset);
							
							if (wsize < 1) {
								return M3U8ERR_PRINTF_WRITE_FAILURE;
							}
							
							if ((size_t) wsize > (bsize - 1)) {
								return M3U8ERR_BUFFER_OVERFLOW;
							}
							
							break;
						}
						case M3U8_VITEM_TYPE_DTIME: {
							const struct M3U8DateTime* const dtime = (struct M3U8DateTime*) item->value;
							
							size_t bsize = sizeof(tmp);
							int wsize = 0;
							
							const int status = (
								(dtime->year < 1000 || dtime->year > 2999) ||
								(dtime->mon < 1 || dtime->mon > 12) ||
								(dtime->mday < 1 || dtime->mday > 31) ||
								(dtime->hour > 23) ||
								(dtime->min > 59) ||
								(dtime->sec > 59) ||
								(dtime->msec != 0 && (dtime->msec < 1 || dtime->msec > 999)) ||
								(dtime->gmtoff != 100000 && (dtime->gmtoff < -(3600 * 24) || dtime->gmtoff > (3600 * 24)))
							);
							
							if (status) {
								return M3U8ERR_ITEM_INVALID_DTIME;
							}
							
							wsize = snprintf(
								tmp,
								bsize,
								"%04"FORMAT_BIGGEST_UINT_T"-%02"FORMAT_BIGGEST_UINT_T"-%02"FORMAT_BIGGEST_UINT_T"T%02"FORMAT_BIGGEST_UINT_T":%02"FORMAT_BIGGEST_UINT_T":%02"FORMAT_BIGGEST_UINT_T,
								dtime->year,
								dtime->mon,
								dtime->mday,
								dtime->hour,
								dtime->min,
								dtime->sec
							);
							
							if (wsize < 1) {
								return M3U8ERR_PRINTF_WRITE_FAILURE;
							}
							
							if ((size_t) wsize > (bsize - 1)) {
								return M3U8ERR_BUFFER_OVERFLOW;
							}
							
							bsize -= (size_t) wsize;
							
							/* Milliseconds */
							if (dtime->msec != 0) {
								wsize = snprintf(tmp + strlen(tmp), bsize, ".%03"FORMAT_BIGGEST_UINT_T, dtime->msec);
								
								if (wsize < 1) {
									return M3U8ERR_PRINTF_WRITE_FAILURE;
								}
								
								if ((size_t) wsize > (bsize - 1)) {
									return M3U8ERR_BUFFER_OVERFLOW;
								}
								
								bsize -= (size_t) wsize;
							}
							
							/* Timezone */
							if (dtime->gmtoff != 100000) {
								const int isbehind = (dtime->gmtoff < 0);
								const char* const prefix = (isbehind ? "-" : "+");
								
								const biguint_t hour = (biguint_t) (dtime->gmtoff / 3600);
								const biguint_t minutes = (biguint_t) ((dtime->gmtoff % 3600) / 60);
								
								wsize = snprintf(
									tmp + strlen(tmp),
									bsize,
									"%s%02"FORMAT_BIGGEST_UINT_T":%02"FORMAT_BIGGEST_UINT_T,
									prefix,
									hour,
									minutes
								);
								
								if (wsize < 1) {
									return M3U8ERR_PRINTF_WRITE_FAILURE;
								}
								
								if ((size_t) wsize > (bsize - 1)) {
									return M3U8ERR_BUFFER_OVERFLOW;
								}
							}
							
							break;
						}
						case M3U8_VITEM_TYPE_ESTRING:
						case M3U8_VITEM_TYPE_USTRING: {
							const char* const string = (char*) item->value;
							
							if (strlen(string) > (sizeof(tmp) - 1)) {
								return M3U8ERR_BUFFER_OVERFLOW;
							}
							
							strcpy(tmp, string);
						}
					}
					
					if ((*dump_callback)(tmp, strlen(tmp), data) == -1) {
						return M3U8ERR_CALLBACK_WRITE_FAILURE;
					}
					
					tmp[0] = '\0';
				}
				
				break;
			}
					
		}
		
		if ((*dump_callback)("\n", 1, data) == -1) {
			return M3U8ERR_CALLBACK_WRITE_FAILURE;
		}
		
		if (tag->uri != NULL) {
			uri_ensure_slashes(tag->uri);
			
			if ((*dump_callback)(tag->uri, strlen(tag->uri), data) == -1) {
				return M3U8ERR_CALLBACK_WRITE_FAILURE;
			}
			
			if ((*dump_callback)("\n", 1, data) == -1) {
				return M3U8ERR_CALLBACK_WRITE_FAILURE;
			}
		}
		
	}
	
	return M3U8ERR_SUCCESS;
	
}

int dump_file_callback(const char* const chunk, const size_t size, void* const data) {
	
	struct FStream* stream = (struct FStream*) data;
	
	const int status = fstream_write(stream, chunk, size);
	
	if (status == -1) {
		return -1;
	}
	
	return 0;
	
}

int m3u8_dump_file(
	const struct M3U8Playlist* const playlist,
	const char* const filename
) {
	
	int status = 0;
	
	struct FStream* stream = fstream_open(filename, FSTREAM_WRITE);
	
	if (stream == NULL) {
		status = M3U8ERR_FSTREAM_OPEN_FAILURE;
		goto end;
	}
	
	if (fstream_lock(stream) == -1) {
		status = M3U8ERR_FSTREAM_LOCK_FAILURE;
		goto end;
	}
	
	status = m3u8_dump_callback(playlist, &dump_file_callback, (void*) stream);
	
	if (status == M3U8ERR_CALLBACK_WRITE_FAILURE) {
		status = M3U8ERR_FSTREAM_WRITE_FAILURE;
		goto end;
	}
	
	end:;
	
	fstream_close(stream);
	
	return status;
	
}

static int m3u8playlist_seturi(
	struct M3U8Playlist* const playlist,
	const enum M3U8BaseURIType type,
	const char* const uri,
	const char* const suburi
) {
	/*
	Set the base URI of the M3U8 playlist.
	
	This can be a path to a local file in the filesystem
	or an HTTP URL.
	*/
	
	int err = M3U8ERR_SUCCESS;
	int subtype = 0;
	
	playlist->uri.type = type;
	
	playlist->uri.uri = malloc(strlen(uri) + 1);
	
	if (playlist->uri.uri == NULL) {
		err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	strcpy(playlist->uri.uri, uri);
	
	if (suburi == NULL) {
		goto end;
	}
	
	subtype = uri_guess_type(suburi);
	
	switch (subtype) {
		case GUESS_URI_TYPE_URL:
			playlist->suburi.type = M3U8_BASE_URI_TYPE_URL;
			break;
		case GUESS_URI_TYPE_LOCAL_FILE:
			playlist->suburi.type = M3U8_BASE_URI_TYPE_LOCAL_FILE;
			break;
		case GUESS_URI_TYPE_LOCAL_DIRECTORY:
			playlist->suburi.type = M3U8_BASE_URI_TYPE_LOCAL_DIRECTORY;
			break;
		default:
			err = M3U8ERR_LOAD_UNSUPPORTED_URI;
			goto end;
	}
	
	playlist->suburi.uri = malloc(strlen(suburi) + 1);
	
	if (playlist->suburi.uri == NULL) {
		err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	strcpy(playlist->suburi.uri, suburi);
	
	end:;
	
	return err;
	
}

int m3u8playlist_load_buffer(
	struct M3U8Playlist* const playlist,
	const char* const buffer
) {
	
	const int err = m3u8_parse(playlist, buffer);
	return err;
	
}

int m3u8playlist_load_file(
	struct M3U8Playlist* const playlist,
	const char* const filename,
	const char* const base_uri
) {
	
	int status = 0;
	int err = 0;
	
	long int file_size = 0;
	ssize_t rsize = 0;
	
	char* buffer = NULL;
	char* absolute_path = NULL;
	
	struct FStream* stream = NULL;
	
	absolute_path = expand_filename(filename);
	
	if (absolute_path == NULL) {
		err = M3U8ERR_EXPAND_FILENAME_FAILURE;
		goto end;
	}
	
	stream = fstream_open(absolute_path, FSTREAM_READ);
	
	if (stream == NULL) {
		err = M3U8ERR_FSTREAM_OPEN_FAILURE;
		goto end;
	}
	
	status = fstream_seek(stream, 0, FSTREAM_SEEK_END);
	
	if (status == -1) {
		err = M3U8ERR_FSTREAM_SEEK_FAILURE;
		goto end;
	}
	
	file_size = fstream_tell(stream);
	
	if (file_size == -1) {
		err = M3U8ERR_FSTREAM_TELL_FAILURE;
		goto end;
	}
	
	if (file_size > (M3U8_MAX_PLAYLIST_LEN - 1)) {
		err = M3U8ERR_PLAYLIST_TOO_LARGE;
		goto end;
	}
	
	status = fstream_seek(stream, 0, FSTREAM_SEEK_BEGIN);
	
	if (status == -1) {
		err = M3U8ERR_FSTREAM_SEEK_FAILURE;
		goto end;
	}
	
	buffer = malloc((size_t) file_size + 1);
	
	if (buffer == NULL) {
		err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	rsize = fstream_read(stream, buffer, (size_t) file_size);
	
	if (rsize == -1) {
		err = M3U8ERR_FSTREAM_READ_FAILURE;
		goto end;
	}
	
	buffer[(size_t) rsize] = '\0';
	
	err = m3u8playlist_load_buffer(playlist, buffer);
	
	if (err != M3U8ERR_SUCCESS) {
		goto end;
	}
	
	err = m3u8playlist_seturi(
		playlist,
		M3U8_BASE_URI_TYPE_LOCAL_FILE,
		absolute_path,
		base_uri
	);
	
	if (err != M3U8ERR_SUCCESS) {
		goto end;
	}
	
	end:;
	
	fstream_close(stream);
	free(buffer);
	free(absolute_path);
	
	return err;
	
}

size_t curl_write_string_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
	
	char* buffer = userdata;
	
	const size_t chunk_size = size * nmemb;
	const size_t slength = strlen(buffer);
	
	if ((slength + chunk_size) > (M3U8_MAX_PLAYLIST_LEN - 1)) {
		return 0;
	}
	
	memcpy(buffer + slength, ptr, chunk_size);
	
	buffer[slength + chunk_size] = '\0';
	
	return chunk_size;
	
}

int m3u8playlist_load_url(
	struct M3U8Playlist* const playlist,
	const char* const url,
	const char* const base_uri
) {
	
	int err = 0;
	CURLcode code = CURLE_OK;
	
	char* effective_url = NULL;
	
	char* buffer = NULL;
	CURL* curl = NULL;
	
	buffer = malloc(M3U8_MAX_PLAYLIST_LEN);
	
	if (buffer == NULL) {
		err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	buffer[0] = '\0';
	
	 err = httpclient_init(&playlist->client);
	
	if (err != M3U8ERR_SUCCESS) {
		goto end;
	}
	
	curl = httpclient_getclient(&playlist->client);
	
	code = curl_easy_setopt(curl, CURLOPT_URL, url);
	
	if (code != CURLE_OK) {
		err = M3U8ERR_CURL_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_string_cb);
	
	if (code != CURLE_OK) {
		err = M3U8ERR_CURL_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
	
	if (code != CURLE_OK) {
		err = M3U8ERR_CURL_SETOPT_FAILURE;
		goto end;
	}
	
	err = httpclient_perform(&playlist->client);
	
	if (err != M3U8ERR_SUCCESS) {
		goto end;
	}
	
	err = m3u8playlist_load_buffer(playlist, buffer);
	
	if (err != M3U8ERR_SUCCESS) {
		goto end;
	}
	
	code = curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &effective_url);
	
	if (code != CURLE_OK) {
		err = M3U8ERR_CURL_SETOPT_FAILURE;
		goto end;
	}
	
	err = m3u8playlist_seturi(
		playlist,
		M3U8_BASE_URI_TYPE_URL,
		effective_url,
		base_uri
	);
	
	if (err != M3U8ERR_SUCCESS) {
		goto end;
	}
	
	end:;
	
	free(buffer);
	
	code = curl_easy_setopt(curl, CURLOPT_URL, NULL);
	
	if (code != CURLE_OK) {
		err = M3U8ERR_CURL_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
	
	if (code != CURLE_OK) {
		err = M3U8ERR_CURL_SETOPT_FAILURE;
		goto end;
	}
	
	code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);
	
	if (code != CURLE_OK) {
		err = M3U8ERR_CURL_SETOPT_FAILURE;
		goto end;
	}
	
	return err;
	
}

int m3u8playlist_load(
	struct M3U8Playlist* const playlist,
	const char* const something,
	const char* const base_uri
) {
	
	int err = 0;
	
	const int type = uri_guess_type(something);
	
	switch (type) {
		case GUESS_URI_TYPE_URL:
			err = m3u8playlist_load_url(playlist, something, base_uri);
			break;
		case GUESS_URI_TYPE_LOCAL_FILE:
			err = m3u8playlist_load_file(playlist, something, base_uri);
			break;
		case GUESS_URI_TYPE_SOMETHING_ELSE:
			err = m3u8playlist_load_buffer(playlist, something);
			break;
		default:
			err = M3U8ERR_LOAD_UNSUPPORTED_URI;
			break;
	}
	
	return err;
	
}

int m3u8playlist_load_subresource(
	const struct M3U8Playlist* const root,
	struct M3U8Playlist* const subresource,
	const char* const something
) {
	
	int err = 0;
	
	const struct M3U8BaseURI* const base_uri = m3u8playlist_geturi(root);
	
	char* uri = NULL;
	
	subresource->subresource = 1;
	
	switch (base_uri->type) {
		case M3U8_BASE_URI_TYPE_URL: {
			CURLcode code = CURLE_OK;
			
			err = m3u8uri_resolve_url(base_uri->uri, something, &uri);
			
			if (err != M3U8ERR_SUCCESS) {
				goto end;
			}
			
			code = curl_easy_setopt(root->client.curl, CURLOPT_REFERER, base_uri->uri);
			
			if (code != CURLE_OK) {
				err = M3U8ERR_CURL_SETOPT_FAILURE;
				goto end;
			}
			
			memcpy(&subresource->client, &root->client, sizeof(root->client));
			
			err = m3u8playlist_load_url(subresource, uri, NULL);
			
			code = curl_easy_setopt(root->client.curl, CURLOPT_REFERER, NULL);
			
			if (code != CURLE_OK) {
				err = M3U8ERR_CURL_SETOPT_FAILURE;
				goto end;
			}
			
			break;
		}
		case M3U8_BASE_URI_TYPE_LOCAL_DIRECTORY:
		case M3U8_BASE_URI_TYPE_LOCAL_FILE: {
			if (base_uri->type == M3U8_BASE_URI_TYPE_LOCAL_DIRECTORY) {
				err = m3u8uri_resolve_directory(base_uri->uri, something, &uri);
			} else {
				err = m3u8uri_resolve_file(base_uri->uri, something, &uri);
			}
			
			if (err != M3U8ERR_SUCCESS) {
				goto end;
			}
			
			err = m3u8playlist_load_file(subresource, uri, NULL);
			break;
		}
		case M3U8_BASE_URI_TYPE_TEXT:
			err = m3u8playlist_load_buffer(subresource, something);
			break;
		default:
			err = M3U8ERR_LOAD_UNSUPPORTED_URI;
			break;
	}
	
	end:;
	
	if (base_uri->type == M3U8_BASE_URI_TYPE_URL) {
		curl_free(uri);
	} else {
		free(uri);
	}
	
	return err;
	
	
}
