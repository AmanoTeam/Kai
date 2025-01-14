#include <stdlib.h>
#include <stdio.h>

#if defined(_WIN32) && defined(_UNICODE)
	#include "wio.h"
#endif

#include "m3u8stream.h"
#include "m3u8utils.h"
#include "biggestint.h"

void show_media_playlist_metadata(const struct M3U8Stream* const stream) {
	
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
	
	printf("\n    Distribution: %s ", (stream->livestream) ? "Live Streaming": "VOD");
	
	printf("\n\n");
	
}

void show_media_playlist(const struct M3U8Stream* const stream) {
	
	printf("Playlist #0\n  Metadata:\n    Type: Media Playlist\n\n");
	
	printf("Media #0\n");
	
	show_media_playlist_metadata(stream);
	
}

void show_media(const struct M3U8StreamItem* const item) {
	
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
	
	printf("\n    Distribution: %s ", (media->stream.livestream) ? "Live Streaming": "VOD");
	
	printf("\n\n");
	
}

void show_variant_stream(const struct M3U8StreamItem* const item) {
	
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
	
	printf("\n    Distribution: %s ", (variant_stream->stream.livestream) ? "Live Streaming": "VOD");
	
	printf("\n\n");
	
}

void show_master_playlist(const struct M3U8Stream* const stream) {
	
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

void show_formats(const struct M3U8Stream* const stream) {
	
	switch (stream->playlist.type) {
		case M3U8_PLAYLIST_TYPE_MASTER:
			show_master_playlist(stream);
			break;
		case M3U8_PLAYLIST_TYPE_MEDIA:
			show_media_playlist(stream);
			break;
	}
	
}
