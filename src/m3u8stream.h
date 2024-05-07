#if !defined(M3U8STREAM_H)
#define M3U8STREAM_H

#include "m3u8types.h"

enum M3U8MediaType {
	M3U8_MEDIA_TYPE_VIDEO = 0x00000001,
	M3U8_MEDIA_TYPE_AUDIO,
	M3U8_MEDIA_TYPE_SUBTITLES,
	M3U8_MEDIA_TYPE_CLOSED_CAPTIONS
};

#define M3U8_MEDIA_TYPE_UNKNOWN ((enum M3U8MediaType) 0x00000000)

enum M3U8ClosedCaption {
	M3U8_CLOSED_CAPTION_CC1 = 0x00000001,
	M3U8_CLOSED_CAPTION_CC2,
	M3U8_CLOSED_CAPTION_CC3,
	M3U8_CLOSED_CAPTION_CC4,
	M3U8_CLOSED_CAPTION_SERVICE1,
	M3U8_CLOSED_CAPTION_SERVICE2,
	M3U8_CLOSED_CAPTION_SERVICE3,
	M3U8_CLOSED_CAPTION_SERVICE4,
	M3U8_CLOSED_CAPTION_SERVICE5,
	M3U8_CLOSED_CAPTION_SERVICE6,
	M3U8_CLOSED_CAPTION_SERVICE7,
	M3U8_CLOSED_CAPTION_SERVICE8,
	M3U8_CLOSED_CAPTION_SERVICE9,
	M3U8_CLOSED_CAPTION_SERVICE10,
	M3U8_CLOSED_CAPTION_SERVICE11,
	M3U8_CLOSED_CAPTION_SERVICE12,
	M3U8_CLOSED_CAPTION_SERVICE13,
	M3U8_CLOSED_CAPTION_SERVICE14,
	M3U8_CLOSED_CAPTION_SERVICE15,
	M3U8_CLOSED_CAPTION_SERVICE16,
	M3U8_CLOSED_CAPTION_SERVICE17,
	M3U8_CLOSED_CAPTION_SERVICE18,
	M3U8_CLOSED_CAPTION_SERVICE19,
	M3U8_CLOSED_CAPTION_SERVICE20,
	M3U8_CLOSED_CAPTION_SERVICE21,
	M3U8_CLOSED_CAPTION_SERVICE22,
	M3U8_CLOSED_CAPTION_SERVICE23,
	M3U8_CLOSED_CAPTION_SERVICE24,
	M3U8_CLOSED_CAPTION_SERVICE25,
	M3U8_CLOSED_CAPTION_SERVICE26,
	M3U8_CLOSED_CAPTION_SERVICE27,
	M3U8_CLOSED_CAPTION_SERVICE28,
	M3U8_CLOSED_CAPTION_SERVICE29,
	M3U8_CLOSED_CAPTION_SERVICE30,
	M3U8_CLOSED_CAPTION_SERVICE31,
	M3U8_CLOSED_CAPTION_SERVICE32,
	M3U8_CLOSED_CAPTION_SERVICE33,
	M3U8_CLOSED_CAPTION_SERVICE34,
	M3U8_CLOSED_CAPTION_SERVICE35,
	M3U8_CLOSED_CAPTION_SERVICE36,
	M3U8_CLOSED_CAPTION_SERVICE37,
	M3U8_CLOSED_CAPTION_SERVICE38,
	M3U8_CLOSED_CAPTION_SERVICE39,
	M3U8_CLOSED_CAPTION_SERVICE40,
	M3U8_CLOSED_CAPTION_SERVICE41,
	M3U8_CLOSED_CAPTION_SERVICE42,
	M3U8_CLOSED_CAPTION_SERVICE43,
	M3U8_CLOSED_CAPTION_SERVICE44,
	M3U8_CLOSED_CAPTION_SERVICE45,
	M3U8_CLOSED_CAPTION_SERVICE46,
	M3U8_CLOSED_CAPTION_SERVICE47,
	M3U8_CLOSED_CAPTION_SERVICE48,
	M3U8_CLOSED_CAPTION_SERVICE49,
	M3U8_CLOSED_CAPTION_SERVICE50,
	M3U8_CLOSED_CAPTION_SERVICE51,
	M3U8_CLOSED_CAPTION_SERVICE52,
	M3U8_CLOSED_CAPTION_SERVICE53,
	M3U8_CLOSED_CAPTION_SERVICE54,
	M3U8_CLOSED_CAPTION_SERVICE55,
	M3U8_CLOSED_CAPTION_SERVICE56,
	M3U8_CLOSED_CAPTION_SERVICE57,
	M3U8_CLOSED_CAPTION_SERVICE58,
	M3U8_CLOSED_CAPTION_SERVICE59,
	M3U8_CLOSED_CAPTION_SERVICE60,
	M3U8_CLOSED_CAPTION_SERVICE61,
	M3U8_CLOSED_CAPTION_SERVICE62,
	M3U8_CLOSED_CAPTION_SERVICE63
};

#define M3U8_CLOSED_CAPTION_UNKNOWN ((enum M3U8ClosedCaption) 0x00000000)

enum M3U8HDCPLevel {
	M3U8_HDCP_LEVEL_NONE = 0x00000001,
	M3U8_HDCP_LEVEL_TYPE_0
};

#define M3U8_HDCP_LEVEL_UNKNOWN ((enum M3U8HDCPLevel) 0x00000000)

enum M3U8EncryptionMethod {
	M3U8_ENCRYPTION_METHOD_NONE = 0x00000001,
	M3U8_ENCRYPTION_METHOD_AES_128,
	M3U8_ENCRYPTION_METHOD_SAMPLE_AES
};

#define M3U8_ENCRYPTION_METHOD_UNKNOWN ((enum M3U8EncryptionMethod) 0x00000000)

enum M3U8MediaPlaylistType {
	M3U8_MEDIA_PLAYLIST_TYPE_EVENT = 0x00000001,
	M3U8_MEDIA_PLAYLIST_TYPE_VOD
};

#define M3U8_MEDIA_PLAYLIST_TYPE_UNKNOWN ((enum M3U8MediaPlaylistType) 0x00000000)

enum M3U8StreamItemType {
	M3U8_STREAM_VERSION = 0x00000001,
	M3U8_STREAM_KEY,
	M3U8_STREAM_SEGMENT,
	M3U8_STREAM_MAP,
	M3U8_STREAM_ALLOW_CACHE,
	M3U8_STREAM_DATE_RANGE,
	M3U8_STREAM_TARGET_DURATION,
	M3U8_STREAM_MEDIA_SEQUENCE,
	M3U8_STREAM_DISCONTINUITY_SEQUENCE,
	M3U8_STREAM_PLAYLIST_TYPE,
	M3U8_STREAM_SESSION_DATA,
	M3U8_STREAM_START,
	M3U8_STREAM_MEDIA,
	M3U8_STREAM_VARIANT_STREAM,
	M3U8_STREAM_PROGRAM_DATE_TIME,
	M3U8_STREAM_END_LIST,
	M3U8_STREAM_IFRAMES_ONLY,
	M3U8_STREAM_DISCONTINUITY,
	M3U8_STREAM_INDEPENDENT_SEGMENTS
};

#define M3U8_STREAM_UNKNOWN ((enum M3U8StreamItemType) 0x00000000)

struct M3U8StreamItem {
	enum M3U8StreamItemType type;
	void* item;
};

struct M3U8Stream {
	size_t offset;
	size_t size;
	struct M3U8StreamItem* items;
	struct M3U8Playlist playlist;
};

struct M3U8Version {
	biguint_t version;
	struct M3U8Tag* tag;
};

struct M3U8StreamPlaylistType {
	enum M3U8MediaPlaylistType type;
	struct M3U8Tag* tag;
};

struct M3U8KeyFormatVersions {
	size_t offset;
	size_t size;
	biguint_t* items;
};

struct M3U8Key {
	enum M3U8EncryptionMethod method;
	char* uri;
	struct M3U8Bytes iv;
	char* keyformat;
	struct M3U8KeyFormatVersions keyformatversions;
	struct M3U8Tag* tag;
};

struct M3U8Segment {
	struct M3U8Key key;
	bigfloat_t duration;
	char* title;
	struct M3U8ByteRange byterange;
	biguint_t bitrate;
	char* uri;
	struct M3U8Tag* tag;
};

struct M3U8Map {
	char* uri;
	struct M3U8ByteRange byterange;
	struct M3U8Tag* tag;
};

struct M3U8AllowCache {
	int allow_cache;
	struct M3U8Tag* tag;
};

struct M3U8DateRange {
	char* id;
	char* class;
	struct M3U8DateTime start_date;
	struct M3U8DateTime end_date;
	bigfloat_t duration;
	bigfloat_t planned_duration;
	int end_on_next;
	struct M3U8Tag* tag;
};

struct M3U8TargetDuration {
	biguint_t duration;
	struct M3U8Tag* tag;
};

struct M3U8MediaSequence {
	biguint_t number;
	struct M3U8Tag* tag;
};

struct M3U8DiscontinuitySequence {
	biguint_t number;
	struct M3U8Tag* tag;
};

struct M3U8SessionData {
	char* data_id;
	char* value;
	char* uri;
	char* language;
	struct M3U8Tag* tag;
};

struct M3U8Start {
	bigfloat_t time_offset;
	int precise;
	struct M3U8Tag* tag;
};

struct M3U8Media {
	enum M3U8MediaType type;
	char* uri;
	char* group_id;
	char* language;
	char* assoc_language;
	char* name;
	int default_;
	int autoselect;
	int forced;
	enum M3U8ClosedCaption instream_id;
	char* characteristics;
	char* channels;
	struct M3U8Stream stream;
	bigfloat_t duration;
	bigfloat_t average_duration;
	biguint_t segments;
	struct M3U8Tag* tag;
};

struct M3U8VariantStream {
	biguint_t bandwidth;
	biguint_t average_bandwidth;
	biguint_t program_id;
	char* codecs;
	struct M3U8Resolution resolution;
	bigfloat_t frame_rate;
	enum M3U8HDCPLevel hdcp_level;
	struct M3U8Media* audio;
	struct M3U8Media* video;
	struct M3U8Media* subtitles;
	struct M3U8Media* closed_captions;
	char* uri;
	struct M3U8Stream stream;
	bigfloat_t duration;
	bigfloat_t average_duration;
	biguint_t size;
	biguint_t segments;
	struct M3U8Tag* tag;
};

enum M3U8SelectStreamCriteria {
	M3U8_SELECT_STREAM_BY_POSITION,
	M3U8_SELECT_STREAM_BY_RESOLUTION
};

size_t m3u8stream_getvarsize(struct M3U8Stream* const stream);

struct M3U8VariantStream* m3u8stream_getvariant(
	struct M3U8Stream* const stream,
	const enum M3U8SelectStreamCriteria criteria,
	const ssize_t position
);

int m3u8stream_load_buffer(
	struct M3U8Stream* const stream,
	const char* const buffer
);

int m3u8stream_load_file(
	struct M3U8Stream* const stream,
	const char* const filename,
	const char* const base_uri
);

int m3u8stream_load_url(
	struct M3U8Stream* const stream,
	const char* const url,
	const char* const base_uri
);

int m3u8stream_load(
	struct M3U8Stream* const stream,
	const char* const something,
	const char* const base_uri
);

int m3u8stream_load_subresource(
	const struct M3U8Stream* const root,
	struct M3U8Stream* const resource,
	const char* const something
);

void m3u8stream_free(struct M3U8Stream* const stream);

bigfloat_t m3u8stream_getduration(const struct M3U8Stream* const stream);
biguint_t m3u8stream_getsegments(const struct M3U8Stream* const stream);
const char* m3u8em_stringify(const enum M3U8EncryptionMethod value);
const char* m3u8cc_stringify(const enum M3U8ClosedCaption value);
const char* m3u8media_stringify(const enum M3U8MediaType value);

#endif
