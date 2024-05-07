#include <stdlib.h>
#include <stdio.h>

#include "m3u8.h"
#include "m3u8stream.h"
#include "m3u8utils.h"
#include "errors.h"
#include "biggestint.h"

#if defined(_WIN32)
	#include "wio.h"
#endif

static const char JSON_TRUE[] = "true";
static const char JSON_FALSE[] = "false";
static const char JSON_NULL[] = "null";

static const char JSON_ARRAY_BEGIN[] = "[";
static const char JSON_ARRAY_END[] = "]";

static const char JSON_OBJECT_BEGIN[] = "{";
static const char JSON_OBJECT_END[] = "}";

static const char JSON_STRING_BEGIN[] = "\"";
static const char JSON_STRING_END[] = "\"";

static const char JSON_DELIMITER[] = ",";

static const char JSON_IDENTATION[] = "                ";

static const char KTYPE[] = "type";
static const char KID[] = "id";
static const char KNAME[] = "name";
static const char KSTREAMS[] = "streams";

static const char KBANDWIDTH[] = "bandwidth";
static const char KAVERAGE_BANDWIDTH[] = "average-bandwidth";
static const char KCODECS[] = "codecs";
static const char KRESOLUTION[] = "resolution";
static const char KWIDTH[] = "width";
static const char KHEIGHT[] = "height";
static const char KFRAME_RATE[] = "frame-rate";
static const char KDURATION[] = "duration";
static const char KAVERAGE_DURATION[] = "average-duration";
static const char KAUDIO[] = "audio";
static const char KVIDEO[] = "video";
static const char KSUBTITLES[] = "subtitles";
static const char KCLOSED_CAPTIONS[] = "closed-captions";

static const char KGROUP_ID[] = "group-id";
static const char KSEGMENTS[] = "segments";
static const char KSIZE[] = "size";
static const char KURI[] = "uri";
static const char KPROGRAM_ID[] = "program-id";
static const char KMEDIA[] = "media";
static const char KTITLE[] = "title";
static const char KBYTE_RANGE[] = "byte-range";
static const char KOFFSET[] = "offset";
static const char KLENGTH[] = "length";
static const char KBITRATE[] = "bitrate";
static const char KKEY[] = "key";
static const char KLANGUAGE[] = "language";
static const char KMETHOD[] = "method";
static const char KIV[] = "iv";
static const char KASSOC_LANGUAGE[] = "assoc-language";
static const char KFORMAT[] = "format";
static const char KFORMAT_VERSIONS[] = "format-versions";

static const char KDEFAULT[] = "default";
static const char KAUTOSELECT[] = "autoselect";
static const char KFORCED[] = "forced";
static const char KINSTREAM_ID[] = "instream-id";

static const char KCHARACTERISTICS[] = "characteristics";
static const char KCHANNELS[] = "channels";
static const char KMEDIA_TYPE[] = "mtype";

static const int MIN_INDENTATION = 2;

static void put_jkey(const char* const key, const int indentation) {
	
	printf(
		"%.*s%s%s%s: ",
		MIN_INDENTATION * indentation,
		JSON_IDENTATION,
		JSON_STRING_BEGIN,
		key,
		JSON_STRING_END
	);
	
}

static void put_jvalue_null(void) {
	
	printf("%s", JSON_NULL);

}

static void put_jvalue_bool(const int value) {
	
	printf("%s", (value) ? JSON_TRUE : JSON_FALSE);
	
}

static void put_jvalue_uint(const biguint_t value) {
	
	if (value == 0) {
		put_jvalue_null();
	} else {
		printf("%"FORMAT_BIGGEST_UINT_T, value);
	}
	
}

static void put_jvalue_int(const bigint_t value) {
	
	if (value == 0) {
		put_jvalue_null();
	} else {
		printf("%"FORMAT_BIGGEST_INT_T, value);
	}
	
}

static void put_jvalue_string(const char* const value) {
	
	if (value == NULL) {
		put_jvalue_null();
	} else {
		printf("%s%s%s", JSON_STRING_BEGIN, value, JSON_STRING_END);
	}

}

static void put_jdelimiter(void) {
	
	printf("%s\n", JSON_DELIMITER);
	
}

static void json_object_begin(const int indentation) {
	
	printf(
		"%.*s%s\n",
		MIN_INDENTATION * indentation,
		JSON_IDENTATION,
		JSON_OBJECT_BEGIN
	);
	
}

static void json_object_end(const int indentation) {
	
	printf(
		"%.*s%s",
		MIN_INDENTATION * indentation,
		JSON_IDENTATION,
		JSON_OBJECT_END
	);
	
}

static void json_array_begin(const int indentation) {
	
	printf(
		"%.*s%s\n",
		MIN_INDENTATION * indentation,
		JSON_IDENTATION,
		JSON_ARRAY_BEGIN
	);
	
}

static void json_array_end(const int indentation) {
	
	printf(
		"%.*s%s",
		MIN_INDENTATION * indentation,
		JSON_IDENTATION,
		JSON_ARRAY_END
	);
	
}

static void put_newline(void) {
	
	printf("\n");
	
}

static void dump_segments(const struct M3U8Stream* const stream) {
	
	size_t index = 0;
	size_t subindex = 0;
	
	int begin = 1;
	
	int err = M3U8ERR_SUCCESS;
	
	const struct M3U8BaseURI* const base_uri = m3u8playlist_geturi(&stream->playlist);
	
	char* resolved_uri = NULL;
	
	for (index = 0; index < stream->offset; index++) {
		const struct M3U8StreamItem* const item = &stream->items[index];
		
		if (!(item->type == M3U8_STREAM_SEGMENT || item->type == M3U8_STREAM_MAP)) {
			continue;
		}
		
		if (!begin) {
			put_jdelimiter();
		}
		
		switch (item->type) {
			case M3U8_STREAM_SEGMENT: {
				const struct M3U8Segment* const segment = ((struct M3U8Segment*) item->item);
				
				json_object_begin(4);
				
				/* Type */
				put_jkey(KTYPE, 5);
				json_object_begin(0);
				
				/* Type -> ID */
				put_jkey(KID, 6);
				put_jvalue_uint(M3U8_STREAM_SEGMENT);
				
				put_jdelimiter();
				
				/* Type -> Name */
				put_jkey(KNAME, 6);
				put_jvalue_string("Media Segment");
				
				put_newline();
				
				json_object_end(5);
				
				put_jdelimiter();
				
				/* Duration */
				put_jkey(KDURATION, 5);
				put_jvalue_uint(segment->duration);
				
				put_jdelimiter();
				
				/* Title */
				put_jkey(KTITLE, 5);
				put_jvalue_string(segment->title);
				
				put_jdelimiter();
				
				/* Byte range */
				put_jkey(KBYTE_RANGE, 5);
				
				if (segment->byterange.length > 0) {
					json_object_begin(0);
					
					/* Byte range -> Offset */
					put_jkey(KOFFSET, 6);
					put_jvalue_uint(segment->byterange.offset);
					
					put_jdelimiter();
					
					/* Byte range -> Length */
					put_jkey(KLENGTH, 6);
					put_jvalue_uint(segment->byterange.length);
					
					put_newline();
					
					json_object_end(5);
				} else {
					put_jvalue_null();
				}
				
				put_jdelimiter();
				
				/* Bitrate */
				put_jkey(KBITRATE, 5);
				put_jvalue_uint(segment->bitrate);
				
				put_jdelimiter();
				
				/* Key */
				put_jkey(KKEY, 5);
				
				if (segment->key.uri != NULL) {
					const char* const name = m3u8em_stringify(segment->key.method);
					
					json_object_begin(0);
					
					/* Method */
					put_jkey(KMETHOD, 6);
					
					json_object_begin(0);
					
					/* Method -> ID */
					put_jkey(KID, 7);
					put_jvalue_uint(segment->key.method);
					
					put_jdelimiter();
					
					/* Method -> Name */
					put_jkey(KNAME, 7);
					put_jvalue_string(name);
					
					put_newline();
					
					json_object_end(6);
					
					put_jdelimiter();
					
					/* URI */
					free(resolved_uri);
					
					err = m3u8uri_resolve(base_uri, segment->key.uri, &resolved_uri);
					
					put_jkey(KURI, 6);
					put_jvalue_string((err == M3U8ERR_SUCCESS) ? resolved_uri : segment->key.uri);
					
					put_jdelimiter();
					
					/* IV (Initialization Vector) */
					put_jkey(KIV, 6);
					
					if (segment->key.iv.offset > 0) {
						json_array_begin(0);
						
						for (subindex = 0; subindex < segment->key.iv.offset; subindex++) {
							const int value = segment->key.iv.data[subindex];
							
							if (subindex > 0) {
								put_jdelimiter();
							}
							
							put_jvalue_int(value);
						}
						
						json_array_end(6);
					} else {
						put_jvalue_null();
					}
					
					put_jdelimiter();
					
					/* Key format */
					put_jkey(KFORMAT, 6);
					put_jvalue_string(segment->key.keyformat);
					
					put_jdelimiter();
					
					/* Key format versions */
					put_jkey(KFORMAT_VERSIONS, 6);
					
					if (segment->key.keyformatversions.offset > 0) {
						json_array_begin(0);
						
						for (subindex = 0; subindex < segment->key.keyformatversions.offset; subindex++) {
							const int value = segment->key.keyformatversions.items[subindex];
							
							if (subindex > 0) {
								put_jdelimiter();
							}
							
							put_jvalue_uint(value);
						}
						
						json_array_end(6);
					} else {
						put_jvalue_null();
					}
					
					put_newline();
					
					json_object_end(5);
				} else {
					put_jvalue_null();
				}
				
				put_jdelimiter();
				
				/* URI */
				free(resolved_uri);
				
				err = m3u8uri_resolve(base_uri, segment->uri, &resolved_uri);
				
				put_jkey(KURI, 5);
				put_jvalue_string((err == M3U8ERR_SUCCESS) ? resolved_uri : segment->uri);
				
				put_newline();
				
				json_object_end(4);
				
				break;
			}
			case M3U8_STREAM_MAP: {
				const struct M3U8Map* const map = ((struct M3U8Map*) item->item);
				
				json_object_begin(4);
				
				/* Type */
				put_jkey(KTYPE, 5);
				json_object_begin(0);
				
				/* Type -> ID */
				put_jkey(KID, 6);
				put_jvalue_uint(M3U8_STREAM_MAP);
				
				put_jdelimiter();
				
				/* Type -> Name */
				put_jkey(KNAME, 6);
				put_jvalue_string("Media Mapping");
				
				put_newline();
				
				json_object_end(5);
				
				put_jdelimiter();
				
				/* Byte range */
				put_jkey(KBYTE_RANGE, 5);
				
				if (map->byterange.length > 0) {
					json_object_begin(0);
					
					/* Byte range -> Offset */
					put_jkey(KOFFSET, 6);
					put_jvalue_uint(map->byterange.offset);
					
					put_jdelimiter();
					
					/* Byte range -> Length */
					put_jkey(KLENGTH, 6);
					put_jvalue_uint(map->byterange.length);
					
					put_newline();
					
					json_object_end(5);
				} else {
					put_jvalue_null();
				}
				
				put_jdelimiter();
				
				/* URI */
				free(resolved_uri);
				
				err = m3u8uri_resolve(base_uri, map->uri, &resolved_uri);
				
				put_jkey(KURI, 5);
				put_jvalue_string((err == M3U8ERR_SUCCESS) ? resolved_uri : map->uri);
				
				put_newline();
				
				json_object_end(4);
				
				break;
			}
			default: {
				break;
			}
		}
		
		begin = 0;
	}
	
	free(resolved_uri);
	
}

static void dump_media_stream(
	const struct M3U8Stream* const stream,
	const struct M3U8StreamItem* const item
) {
	
	int err = M3U8ERR_SUCCESS;
	
	const struct M3U8Media* const media = item->item;
	const struct M3U8BaseURI* const base_uri = m3u8playlist_geturi(&stream->playlist);
	
	const char* name = NULL;
	
	char* resolved_uri = NULL;
	
	json_object_begin(2);
	
	/* Type */
	put_jkey(KTYPE, 3);
	json_object_begin(0);
	
	/* Type -> ID */
	put_jkey(KID, 4);
	put_jvalue_uint(M3U8_STREAM_MEDIA);
	
	put_jdelimiter();
	
	/* Type -> Name */
	put_jkey(KNAME, 4);
	put_jvalue_string("Media Stream");
	
	put_newline();
	
	json_object_end(3);
	
	put_jdelimiter();
	
	/* Media type */
	name = m3u8media_stringify(media->type);
	
	put_jkey(KMEDIA_TYPE, 3);
	json_object_begin(0);
	
	/* Media type -> ID */
	put_jkey(KID, 4);
	put_jvalue_uint(media->type);
	
	put_jdelimiter();
	
	/* Media type -> Name */
	put_jkey(KNAME, 4);
	put_jvalue_string(name);
	
	put_newline();
	
	json_object_end(3);
	
	put_jdelimiter();
	
	/* Group ID */
	put_jkey(KGROUP_ID, 3);
	put_jvalue_string(media->group_id);
	
	put_jdelimiter();
	
	/* Language */
	put_jkey(KLANGUAGE, 3);
	put_jvalue_string(media->language);
	
	put_jdelimiter();
	
	/* Associated language */
	put_jkey(KASSOC_LANGUAGE, 3);
	put_jvalue_string(media->assoc_language);
	
	put_jdelimiter();
	
	/* Name */
	put_jkey(KNAME, 3);
	put_jvalue_string(media->name);
	
	put_jdelimiter();
	
	/* Default */
	put_jkey(KDEFAULT, 3);
	put_jvalue_bool(media->default_);
	
	put_jdelimiter();
	
	/* Autoselect */
	put_jkey(KAUTOSELECT, 3);
	put_jvalue_bool(media->autoselect);
	
	put_jdelimiter();
	
	/* Forced */
	put_jkey(KFORCED, 3);
	put_jvalue_bool(media->forced);
	
	put_jdelimiter();
	
	/* InStream ID */
	put_jkey(KINSTREAM_ID, 3);
	
	if (media->instream_id != M3U8_CLOSED_CAPTION_UNKNOWN) {
		name = m3u8cc_stringify(media->instream_id);
		
		json_object_begin(0);
	
		/* InStream ID -> ID */
		put_jkey(KID, 4);
		put_jvalue_uint(media->instream_id);
		
		put_jdelimiter();
		
		/* InStream ID -> Name */
		put_jkey(KNAME, 4);
		put_jvalue_string(name);
		
		put_newline();
		
		json_object_end(3);
	} else {
		put_jvalue_null();
	}
	
	put_jdelimiter();
	
	/* Characteristics */
	put_jkey(KCHARACTERISTICS, 3);
	put_jvalue_string(media->characteristics);
	
	put_jdelimiter();
	
	/* Channels */
	put_jkey(KCHANNELS, 3);
	put_jvalue_string(media->channels);
	
	put_jdelimiter();
	
	/* Duration */
	put_jkey(KDURATION, 3);
	put_jvalue_uint(media->duration);
	
	put_jdelimiter();
	
	/* Average duration */
	put_jkey(KAVERAGE_DURATION, 3);
	put_jvalue_uint(media->average_duration);
	
	put_jdelimiter();
	
	/* Segments */
	put_jkey(KSEGMENTS, 3);
	put_jvalue_uint(media->segments);
	
	put_jdelimiter();
	
	/* URI */
	err = m3u8uri_resolve(base_uri, media->uri, &resolved_uri);
	
	put_jkey(KURI, 3);
	put_jvalue_string((err == M3U8ERR_SUCCESS) ? resolved_uri : media->uri);
	
	put_jdelimiter();
	
	/* Media */
	put_jkey(KMEDIA, 3);
	
	json_array_begin(0);
	
	dump_segments(&media->stream);
	
	put_newline();
	
	json_array_end(3);
	
	put_newline();
	
	json_object_end(2);
	
	free(resolved_uri);
	
}

static void dump_variant_stream(
	const struct M3U8Stream* const stream,
	const struct M3U8StreamItem* const item
) {
	
	int err = M3U8ERR_SUCCESS;
	
	const struct M3U8VariantStream* const variant_stream = item->item;
	const struct M3U8BaseURI* const base_uri = m3u8playlist_geturi(&stream->playlist);
	
	char* resolved_uri = NULL;
	
	json_object_begin(2);
	
	/* Type */
	put_jkey(KTYPE, 3);
	json_object_begin(0);
	
	/* Type -> ID */
	put_jkey(KID, 4);
	put_jvalue_uint(M3U8_STREAM_VARIANT_STREAM);
	
	put_jdelimiter();
	
	/* Type -> Name */
	put_jkey(KNAME, 4);
	put_jvalue_string("Variant Stream");
	
	put_newline();
	
	json_object_end(3);
	
	put_jdelimiter();
	
	/* Bandwidth */
	put_jkey(KBANDWIDTH, 3);
	put_jvalue_uint(variant_stream->bandwidth);
	put_jdelimiter();
	
	/* Average bandwidth */
	put_jkey(KAVERAGE_BANDWIDTH, 3);
	put_jvalue_uint(variant_stream->average_bandwidth);
	
	put_jdelimiter();
	
	/* Program ID */
	put_jkey(KPROGRAM_ID, 3);
	put_jvalue_uint(variant_stream->program_id);
	
	put_jdelimiter();
	
	/* Codecs */
	put_jkey(KCODECS, 3);
	put_jvalue_string(variant_stream->codecs);
	
	put_jdelimiter();
	
	/* Resolution */
	put_jkey(KRESOLUTION, 3);
	
	if (variant_stream->resolution.width && variant_stream->resolution.height) { 
		json_object_begin(0);
		
		/* Resolution -> Width */
		put_jkey(KWIDTH, 4);
		put_jvalue_uint(variant_stream->resolution.width);
		
		put_jdelimiter();
		
		/* Resolution -> Height */
		put_jkey(KHEIGHT, 4);
		put_jvalue_uint(variant_stream->resolution.height);
		
		put_newline();
		
		json_object_end(3);
	} else {
		put_jvalue_null();
	}
	
	put_jdelimiter();
	
	/* Frame rate */
	put_jkey(KFRAME_RATE, 3);
	put_jvalue_uint(variant_stream->frame_rate);
	
	put_jdelimiter();
	
	/* Duration */
	put_jkey(KDURATION, 3);
	put_jvalue_uint(variant_stream->duration);
	
	put_jdelimiter();
	
	/* Average duration */
	put_jkey(KAVERAGE_DURATION, 3);
	put_jvalue_uint(variant_stream->average_duration);
	
	put_jdelimiter();
	
	/* Audio */
	put_jkey(KAUDIO, 3);
	
	if (variant_stream->audio) { 
		json_object_begin(0);
		
		/* Audio -> Name */
		put_jkey(KNAME, 4);
		put_jvalue_string(variant_stream->audio->name);
		
		put_jdelimiter();
		
		/* Audio -> Group ID */
		put_jkey(KGROUP_ID, 4);
		put_jvalue_string(variant_stream->audio->group_id);
		
		put_newline();
		
		json_object_end(3);
	} else {
		put_jvalue_null();
	}
	
	put_jdelimiter();
	
	/* Video */
	put_jkey(KVIDEO, 3);
	
	if (variant_stream->video) { 
		json_object_begin(0);
		
		/* Video -> Name */
		put_jkey(KNAME, 4);
		put_jvalue_string(variant_stream->video->name);
		
		put_jdelimiter();
		
		/* Video -> Group ID */
		put_jkey(KGROUP_ID, 4);
		put_jvalue_string(variant_stream->video->group_id);
		
		put_newline();
		
		json_object_end(3);
	} else {
		put_jvalue_null();
	}
	
	put_jdelimiter();
	
	/* Subtitles */
	put_jkey(KSUBTITLES, 3);
	
	if (variant_stream->subtitles) { 
		json_object_begin(0);
		
		/* Subtitles -> Name */
		put_jkey(KNAME, 4);
		put_jvalue_string(variant_stream->subtitles->name);
		
		put_jdelimiter();
		
		/* Subtitles -> Group ID */
		put_jkey(KGROUP_ID, 4);
		put_jvalue_string(variant_stream->subtitles->group_id);
		
		put_newline();
		
		json_object_end(3);
	} else {
		put_jvalue_null();
	}
	
	put_jdelimiter();
	
	/* Closed captions */
	put_jkey(KCLOSED_CAPTIONS, 3);
	
	if (variant_stream->closed_captions) { 
		json_object_begin(0);
		
		/* Closed captions -> Name */
		put_jkey(KNAME, 4);
		put_jvalue_string(variant_stream->closed_captions->name);
		
		put_jdelimiter();
		
		/* Closed captions -> Group ID */
		put_jkey(KGROUP_ID, 4);
		put_jvalue_string(variant_stream->closed_captions->group_id);
		
		put_newline();
		
		json_object_end(3);
	} else {
		put_jvalue_null();
	}
	
	put_jdelimiter();
	
	/* Segments */
	put_jkey(KSEGMENTS, 3);
	put_jvalue_uint(variant_stream->segments);
	
	put_jdelimiter();
	
	/* Size */
	put_jkey(KSIZE, 3);
	put_jvalue_uint(variant_stream->size);
	
	put_jdelimiter();
	
	err = m3u8uri_resolve(base_uri, variant_stream->uri, &resolved_uri);
	
	put_jkey(KURI, 3);
	put_jvalue_string((err == M3U8ERR_SUCCESS) ? resolved_uri : variant_stream->uri);
	
	put_jdelimiter();
	
	/* Media */
	put_jkey(KMEDIA, 3);
	
	json_array_begin(0);
	
	dump_segments(&variant_stream->stream);
	
	put_newline();
	
	json_array_end(3);
	
	put_newline();
	
	json_object_end(2);
	
	free(resolved_uri);
	
}

static void dump_media_playlist(const struct M3U8Stream* const stream) {
	
	const bigfloat_t duration = m3u8stream_getduration(stream);
	const biguint_t segments = m3u8stream_getsegments(stream);
	const bigfloat_t average_duration = duration / (segments > 0 ? segments : 1);
	
	json_object_begin(0);
	
	/* Type */
	put_jkey(KTYPE, 1);
	json_object_begin(0);
	
	/* Type -> ID */
	put_jkey(KID, 2);
	put_jvalue_uint(stream->playlist.type);
	
	put_jdelimiter();
	
	/* Type -> Name */
	put_jkey(KNAME, 2);
	put_jvalue_string("Media Playlist");
	
	put_newline();
	
	json_object_end(1);
	
	put_jdelimiter();
	
	/* Streams */
	put_jkey(KSTREAMS, 1);
	
	json_array_begin(0);
	
	json_object_begin(2);
	
	/* Type */
	put_jkey(KTYPE, 3);
	put_jvalue_null();
	
	put_jdelimiter();
	
	/* Duration */
	put_jkey(KDURATION, 3);
	put_jvalue_uint(duration);
	
	put_jdelimiter();
	
	/* Average duration */
	put_jkey(KAVERAGE_DURATION, 3);
	put_jvalue_uint(average_duration);
	
	put_jdelimiter();
	
	/* Segments */
	put_jkey(KSEGMENTS, 3);
	put_jvalue_uint(segments);
	
	put_jdelimiter();
	
	/* Media */
	put_jkey(KMEDIA, 3);
	
	json_array_begin(0);
	
	dump_segments(stream);
	
	put_newline();
	
	json_array_end(3);
	
	put_newline();
	
	json_object_end(2);
	
	put_newline();
	
	json_array_end(1);
	
	put_newline();
	
	json_object_end(0);
	
	put_newline();
	
}

static void dump_master_playlist(const struct M3U8Stream* const stream) {
	
	int begin = 1;
	
	size_t index = 0;
	
	json_object_begin(0);
	
	/* Type */
	put_jkey(KTYPE, 1);
	json_object_begin(0);
	
	/* Type -> ID */
	put_jkey(KID, 2);
	put_jvalue_uint(stream->playlist.type);
	
	put_jdelimiter();
	
	/* Type -> Name */
	put_jkey(KNAME, 2);
	put_jvalue_string("Master Playlist");
	
	put_newline();
	
	json_object_end(1);
	
	put_jdelimiter();
	
	/* Streams */
	put_jkey(KSTREAMS, 1);
	
	json_array_begin(0);
	
	for (index = 0; index < stream->offset; index++) {
		const struct M3U8StreamItem* const item = &stream->items[index];
		
		if (!(item->type == M3U8_STREAM_MEDIA || item->type == M3U8_STREAM_VARIANT_STREAM)) {
			continue;
		}
		
		if (!begin) {
			put_jdelimiter();
		}
		
		switch (item->type) {
			case M3U8_STREAM_MEDIA: {
				
				dump_media_stream(stream, item);
				
				break;
			}
			case M3U8_STREAM_VARIANT_STREAM: {
				const struct M3U8VariantStream* const variant_stream = (struct M3U8VariantStream*) item->item;
				
				if (variant_stream->tag->type != M3U8_TAG_EXT_X_STREAM_INF) {
					break;
				}
				
				dump_variant_stream(stream, item);
				
				break;
			}
			default: {
				break;
			}
		}
		
		begin = 0;
	}
	
	put_newline();
	
	json_array_end(1);
	
	put_newline();
	
	json_object_end(0);
	
	put_newline();
	
}

void dump_playlist(const struct M3U8Stream* const stream) {
	
	switch (stream->playlist.type) {
		case M3U8_PLAYLIST_TYPE_MASTER:
			dump_master_playlist(stream);
			break;
		case M3U8_PLAYLIST_TYPE_MEDIA:
			dump_media_playlist(stream);
			break;
	}
	
}