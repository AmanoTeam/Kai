#if !defined(SHOWFORMATS_H)
#define SHOWFORMATS_H

#include "m3u8stream.h"

void show_media_playlist_metadata(const struct M3U8Stream* const stream);

void show_media(const struct M3U8StreamItem* const item);
void show_variant_stream(const struct M3U8StreamItem* const item);

void show_media_playlist(const struct M3U8Stream* const stream);
void show_master_playlist(const struct M3U8Stream* const stream);
void show_formats(const struct M3U8Stream* const stream);

#endif
