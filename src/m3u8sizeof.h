#if !defined(M3U8SIZEOF_H)
#define M3U8SIZEOF_H

#include <stdlib.h>

#include "m3u8types.h"

size_t m3u8attribute_getvsize(const struct M3U8Attribute* const attribute);
size_t m3u8item_getvsize(const struct M3U8Item* const item);

#endif
