#include <stdio.h>

#include "fstream.h"
#include "m3u8types.h"

int m3u8_parse(
	struct M3U8Playlist* const playlist,
	const char* const s
);
void m3u8attribute_free(struct M3U8Attribute* const attribute);
void m3u8item_free(struct M3U8Item* const item);
void m3u8tag_free(struct M3U8Tag* const tag);
void m3u8playlist_free(struct M3U8Playlist* const playlist);

const char* strm3u8err(const int code);

int m3u8tag_seturi(
	struct M3U8Tag* const tag,
	const void* const uri
);

const char* m3u8tag_geturi(const struct M3U8Tag* const tag);

int m3u8tag_setvalue(
	struct M3U8Tag* const tag,
	const char* const value
);

struct M3U8Item* m3u8tag_getitem(struct M3U8Tag* const tag, const size_t index);

int m3u8tag_setattributes(
	struct M3U8Tag* const tag,
	const struct M3U8Attributes* const attributes
);

const struct M3U8Attributes* m3u8tag_getattributes(const struct M3U8Tag* const tag);

struct M3U8Attribute* m3u8tag_getattr(
	const struct M3U8Tag* const tag,
	const char* const key
);

int m3u8tag_setattr(
	struct M3U8Tag* const tag,
	const char* const key,
	const char* const value
);

const char* m3u8attribute_getkey(const struct M3U8Attribute* const attribute);

struct M3U8Attribute* m3u8tag_sgetattr(
	const struct M3U8Tag* const tag,
	const char* const key
);

struct M3U8Attribute* m3u8tag_igetattr(
	const struct M3U8Tag* const tag,
	enum M3U8AttributeType type
);

struct M3U8Tag* m3u8playlist_igettag(
	 const struct M3U8Playlist* const playlist,
	enum M3U8TagType type
);

int m3u8playlist_tagexists(
	 const struct M3U8Playlist* const playlist,
	enum M3U8TagType type
);

int m3u8attr_setvalue(
	struct M3U8Attribute* const attribute,
	const char* const value
);

int m3u8playlist_ideltag(
	struct M3U8Playlist* const playlist,
	enum M3U8TagType type
);

int m3u8tag_idelattr(
	struct M3U8Tag* const tag,
	enum M3U8AttributeType type
);

int m3u8playlist_load_buffer(
	struct M3U8Playlist* const playlist,
	const char* const buffer
);

int m3u8playlist_load_file(
	struct M3U8Playlist* const playlist,
	const char* const filename,
	const char* const base_uri
);

int m3u8playlist_load_url(
	struct M3U8Playlist* const playlist,
	const char* const url,
	const char* const base_uri
);

int m3u8playlist_load(
	struct M3U8Playlist* const playlist,
	const char* const something,
	const char* const base_uri
);

int m3u8playlist_load_subresource(
	const struct M3U8Playlist* const root,
	struct M3U8Playlist* const subresource,
	const char* const something
);

int m3u8_dump_file(
	const struct M3U8Playlist* const playlist,
	const char* const filename
);

#define m3u8playlist_geturi(playlist) (((playlist)->suburi.uri == NULL) ? &(playlist)->uri : &(playlist)->suburi)
#define m3u8playlist_getclient(playlist) (&(playlist)->client)
#define m3u8playlist_get_mclient(playlist) (&(playlist)->multi_client)
