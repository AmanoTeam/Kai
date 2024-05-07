#if !defined(M3U8TYPES_H)
#define M3U8TYPES_H

#include <stdlib.h>

#include "biggestint.h"
#include "m3u8httpclient.h"

enum M3U8VTagType {
	/*
	Those tags does not required any additional value to be supplied.
	*/
	M3U8_VTAG_NONE = 0x00000001,
	
	/*
	Those tags require a comma-separated list of attribute/value pairs.
	*/
	M3U8_VTAG_ATTRIBUTE_LIST,
	
	/*
	Those tags require a list of items to be supplied.
	*/
	M3U8_VTAG_LIST,
	
	/*
	Those tags require single-value option to be supplied.
	*/
	M3U8_VTAG_SINGLE_VALUE
};

#define M3U8_VTAG_TYPE_UNKNOWN ((enum M3U8VTagType) 0x00000000)

enum M3U8VAttrType {
	/*
	An unquoted string of characters from the set [0..9] expressing an integer in base-10
	arithmetic in the range from 0 to 2^64-1 (18446744073709551615).  A decimal-integer
	may be from 1 to 20 characters long.
	*/
	M3U8_VATTR_TYPE_UINT = 0x00000001,
	
	/*
	An unquoted string of characters from the set [0..9] and [A..F] that is prefixed with 0x or 0X.
	The maximum length of a hexadecimal-sequence depends on its AttributeNames.
	*/
	M3U8_VATTR_TYPE_HEXSEQ = 0x00000002,
	
	/*
	An unquoted string of characters from the set [0..9] and '.' that expresses a non-negative
	floating-point number in decimal positional notation.
	*/
	M3U8_VATTR_TYPE_UFLOAT = 0x00000004,
	
	/*
	An unquoted string of characters from the set [0..9], '-', and '.' that expresses a signed
	floating-point number in decimal positional notation.
	*/
	M3U8_VATTR_TYPE_FLOAT = 0x00000010,
	
	/*
	A string of characters within a pair of double quotes (0x22).  The following characters
	MUST NOT appear in a quoted-string: line feed (0xA), carriage return (0xD), or double
	quote (0x22). Quoted-string AttributeValues SHOULD be constructed so that byte-wise
	comparison is sufficient to test two quoted-string AttributeValues for equality. Note that this
	implies case-sensitive comparison.
	*/
	M3U8_VATTR_TYPE_QSTRING = 0x00000020,
	
	/*
	An unquoted character string from a set that is explicitly defined by the AttributeName.
	An enumerated-string will never contain double quotes ("), commas (,), or whitespace.
	*/
	M3U8_VATTR_TYPE_ESTRING = 0x00000040,
	
	/*
	Two decimal-integers separated by the "x" character. The first integer is a horizontal pixel
	dimension (width); the second is a vertical pixel dimension (height).
	*/
	M3U8_VATTR_TYPE_RESOLUTION = 0x00000080
};

#define M3U8_VATTR_TYPE_UNKNOWN ((enum M3U8VAttrType) 0x00000000)

enum M3U8VItemType {
	M3U8_VITEM_TYPE_UINT = 0x00000001,
	M3U8_VITEM_TYPE_UFLOAT = 0x00000002,
	M3U8_VITEM_TYPE_BRANGE = 0x00000004,
	M3U8_VITEM_TYPE_DTIME = 0x00000010,
	M3U8_VITEM_TYPE_ESTRING = 0x00000020,
	M3U8_VITEM_TYPE_USTRING = 0x00000040
};

#define M3U8_VITEM_TYPE_UNKNOWN ((enum M3U8VItemType) 0x00000000)

enum M3U8TagType {
	M3U8_TAG_EXTM3U = 0x000000001,
	M3U8_TAG_EXT_X_VERSION = 0x000000002,
	M3U8_TAG_EXTINF = 0x000000004,
	M3U8_TAG_EXT_X_BYTERANGE = 0x000000010,
	M3U8_TAG_EXT_X_DISCONTINUITY = 0x000000020,
	M3U8_TAG_EXT_X_KEY = 0x000000040,
	M3U8_TAG_EXT_X_MAP = 0x000000080,
	M3U8_TAG_EXT_X_PROGRAM_DATE_TIME = 0x000000100,
	M3U8_TAG_EXT_X_ALLOW_CACHE = 0x000000200,
	M3U8_TAG_EXT_X_DATERANGE = 0x000000400,
	M3U8_TAG_EXT_X_TARGETDURATION = 0x000000800,
	M3U8_TAG_EXT_X_MEDIA_SEQUENCE = 0x000001000,
	M3U8_TAG_EXT_X_DISCONTINUITY_SEQUENCE = 0x000002000,
	M3U8_TAG_EXT_X_ENDLIST = 0x000004000,
	M3U8_TAG_EXT_X_PLAYLIST_TYPE = 0x000008000,
	M3U8_TAG_EXT_X_I_FRAMES_ONLY = 0x000010000,
	M3U8_TAG_EXT_X_MEDIA = 0x000020000,
	M3U8_TAG_EXT_X_STREAM_INF = 0x000040000,
	M3U8_TAG_EXT_X_I_FRAME_STREAM_INF = 0x000080000,
	M3U8_TAG_EXT_X_SESSION_DATA = 0x000100000,
	M3U8_TAG_EXT_X_SESSION_KEY = 0x000200000,
	M3U8_TAG_EXT_X_INDEPENDENT_SEGMENTS = 0x000400000,
	M3U8_TAG_EXT_X_START = 0x000800000,
	M3U8_TAG_EXT_X_DEFINE = 0x001000000,
	M3U8_TAG_EXT_X_PART_INF = 0x002000000,
	M3U8_TAG_EXT_X_SERVER_CONTROL = 0x004000000,
	M3U8_TAG_EXT_X_GAP = 0x008000000,
	M3U8_TAG_EXT_X_BITRATE = 0x010000000,
	M3U8_TAG_EXT_X_PART = 0x020000000,
	M3U8_TAG_EXT_X_SKIP = 0x040000000,
	M3U8_TAG_EXT_X_PRELOAD_HINT = 0x080000000,
	M3U8_TAG_EXT_X_RENDITION_REPORT = 0x100000000
};

#define M3U8_TAG_TYPE_UNKNOWN ((enum M3U8TagType) 0x00000000)

enum M3U8AttributeType {
	M3U8_ATTRIBUTE_METHOD = 0x00000001,
	M3U8_ATTRIBUTE_URI,
	M3U8_ATTRIBUTE_IV,
	M3U8_ATTRIBUTE_KEYFORMAT,
	M3U8_ATTRIBUTE_KEYFORMATVERSIONS,
	M3U8_ATTRIBUTE_BYTERANGE,
	M3U8_ATTRIBUTE_ID,
	M3U8_ATTRIBUTE_CLASS,
	M3U8_ATTRIBUTE_START_DATE,
	M3U8_ATTRIBUTE_END_DATE,
	M3U8_ATTRIBUTE_DURATION,
	M3U8_ATTRIBUTE_PLANNED_DURATION,
	M3U8_ATTRIBUTE_END_ON_NEXT,
	M3U8_ATTRIBUTE_TYPE,
	M3U8_ATTRIBUTE_GROUP_ID,
	M3U8_ATTRIBUTE_LANGUAGE,
	M3U8_ATTRIBUTE_ASSOC_LANGUAGE,
	M3U8_ATTRIBUTE_NAME,
	M3U8_ATTRIBUTE_DEFAULT,
	M3U8_ATTRIBUTE_AUTOSELECT,
	M3U8_ATTRIBUTE_FORCED,
	M3U8_ATTRIBUTE_INSTREAM_ID,
	M3U8_ATTRIBUTE_CHARACTERISTICS,
	M3U8_ATTRIBUTE_CHANNELS,
	M3U8_ATTRIBUTE_BANDWIDTH,
	M3U8_ATTRIBUTE_PROGRAM_ID,
	M3U8_ATTRIBUTE_AVERAGE_BANDWIDTH,
	M3U8_ATTRIBUTE_CODECS,
	M3U8_ATTRIBUTE_RESOLUTION,
	M3U8_ATTRIBUTE_FRAME_RATE,
	M3U8_ATTRIBUTE_HDCP_LEVEL,
	M3U8_ATTRIBUTE_AUDIO,
	M3U8_ATTRIBUTE_VIDEO,
	M3U8_ATTRIBUTE_SUBTITLES,
	M3U8_ATTRIBUTE_CLOSED_CAPTIONS,
	M3U8_ATTRIBUTE_DATA_ID,
	M3U8_ATTRIBUTE_VALUE,
	M3U8_ATTRIBUTE_TIME_OFFSET,
	M3U8_ATTRIBUTE_PRECISE,
	M3U8_ATTRIBUTE_IMPORT,
	M3U8_ATTRIBUTE_QUERYPARAM,
	M3U8_ATTRIBUTE_PART_TARGET,
	M3U8_ATTRIBUTE_CAN_SKIP_UNTIL,
	M3U8_ATTRIBUTE_CAN_SKIP_DATERANGES,
	M3U8_ATTRIBUTE_HOLD_BACK,
	M3U8_ATTRIBUTE_PART_HOLD_BACK,
	M3U8_ATTRIBUTE_CAN_BLOCK_RELOAD,
	M3U8_ATTRIBUTE_INDEPENDENT,
	M3U8_ATTRIBUTE_GAP,
	M3U8_ATTRIBUTE_CUE,
	M3U8_ATTRIBUTE_SKIPPED_SEGMENTS,
	M3U8_ATTRIBUTE_RECENTLY_REMOVED_DATERANGES,
	M3U8_ATTRIBUTE_BYTERANGE_START,
	M3U8_ATTRIBUTE_BYTERANGE_LENGTH,
	M3U8_ATTRIBUTE_LAST_MSN,
	M3U8_ATTRIBUTE_LAST_PART,
	M3U8_ATTRIBUTE_STABLE_RENDITION_ID,
	M3U8_ATTRIBUTE_BIT_DEPTH,
	M3U8_ATTRIBUTE_SAMPLE_RATE,
	M3U8_ATTRIBUTE_SCORE,
	M3U8_ATTRIBUTE_SUPPLEMENTAL_CODECS,
	M3U8_ATTRIBUTE_ALLOWED_CPC,
	M3U8_ATTRIBUTE_REQ_VIDEO_LAYOUT,
	M3U8_ATTRIBUTE_VIDEO_RANGE,
	M3U8_ATTRIBUTE_STABLE_VARIANT_ID,
	M3U8_ATTRIBUTE_PATHWAY_ID,
	M3U8_ATTRIBUTE_X_CUSTOM
};

#define M3U8_ATTRIBUTE_TYPE_UNKNOWN ((enum M3U8AttributeType) 0x00000000)

enum M3U8PlaylistType {
	/*
	The Master Playlist provides information about different variants or renditions
	of the same content, such as different bitrates or resolutions.
	*/
	M3U8_PLAYLIST_TYPE_MASTER = 0x00000001,
	
	/*
	The Media Playlist contains the actual media segments and their URLs. It is specific
	to a particular variant identified in the Master Playlist. 
	*/
	M3U8_PLAYLIST_TYPE_MEDIA
};

#define M3U8_PLAYLIST_TYPE_UNKNOWN ((enum M3U8PlaylistType) 0x00000000)

struct M3U8Attribute {
	enum M3U8AttributeType type;
	enum M3U8VAttrType vtype;
	char* key;
	void* value;
};

struct M3U8Attributes {
	size_t offset;
	size_t size;
	struct M3U8Attribute* items;
};

struct M3U8Item {
	enum M3U8VItemType vtype;
	void* value;
};

struct M3U8Items {
	size_t offset;
	size_t size;
	struct M3U8Item* items;
};

struct M3U8Tag {
	enum M3U8TagType type;
	enum M3U8VTagType vtype;
	struct M3U8Attributes attributes;
	struct M3U8Items items;
	struct M3U8Item value;
	char* uri;
};

struct M3U8Tags {
	size_t offset;
	size_t size;
	struct M3U8Tag* items;
};

enum M3U8BaseURIType {
	M3U8_BASE_URI_TYPE_TEXT,
	M3U8_BASE_URI_TYPE_URL,
	M3U8_BASE_URI_TYPE_LOCAL_FILE,
	M3U8_BASE_URI_TYPE_LOCAL_DIRECTORY
};

struct M3U8BaseURI {
	enum M3U8BaseURIType type;
	char* uri;
};

struct M3U8Playlist {
	enum M3U8PlaylistType type;
	struct M3U8Tags tags;
	struct M3U8BaseURI uri;
	struct M3U8BaseURI suburi;
	struct M3U8HTTPClient client;
	struct M3U8MultiHTTPClient multi_client;
	int livestream;
	int subresource;
};

struct M3U8Resolution {
	biguint_t width;
	biguint_t height;
};

struct M3U8ByteRange {
	biguint_t length;
	biguint_t offset;
};

struct M3U8DateTime {
	biguint_t msec; /* Milliseconds */
	biguint_t sec; /* Seconds */
	biguint_t min; /* Minutes */
	biguint_t hour; /* Hour */
	biguint_t mday; /* Day */
	biguint_t mon; /* Month */
	biguint_t year; /* Year */
	bigint_t gmtoff; /* Seconds East of UTC */
};

struct M3U8Bytes {
	size_t offset;
	size_t size;
	unsigned char* data;
};

#endif
