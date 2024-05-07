#include <stdlib.h>
#include <string.h>

#include "m3u8types.h"
#include "m3u8sizeof.h"

static size_t m3u8vattr_getsize(const enum M3U8VAttrType type) {
	
	size_t size = 0;
	
	switch (type) {
		case M3U8_VATTR_TYPE_UINT:
			size = sizeof(biguint_t);
			break;
		case M3U8_VATTR_TYPE_HEXSEQ:
			size = sizeof(struct M3U8Bytes);
			break;
		case M3U8_VATTR_TYPE_UFLOAT:
		case M3U8_VATTR_TYPE_FLOAT:
			size = sizeof(bigfloat_t);
			break;
		case M3U8_VATTR_TYPE_QSTRING:
		case M3U8_VATTR_TYPE_ESTRING:
			size = 0;
			break;
		case M3U8_VATTR_TYPE_RESOLUTION:
			size = sizeof(struct M3U8Resolution);
			break;
	}
	
	return size;
	
}

static size_t m3u8vitem_getsize(const enum M3U8VItemType type) {
	
	size_t size = 0;
	
	switch (type) {
		case M3U8_VITEM_TYPE_UINT:
			size = sizeof(biguint_t);
			break;
		case M3U8_VITEM_TYPE_UFLOAT:
			size = sizeof(bigfloat_t);
			break;
		case M3U8_VITEM_TYPE_BRANGE:
			size = sizeof(struct M3U8ByteRange);
			break;
		case M3U8_VITEM_TYPE_DTIME:
			size = sizeof(struct M3U8DateTime);
			break;
		case M3U8_VITEM_TYPE_ESTRING:
		case M3U8_VITEM_TYPE_USTRING:
			size = 0;
			break;
	}
	
	return size;
	
}

size_t m3u8attribute_getvsize(const struct M3U8Attribute* const attribute) {
	/*
	Get the storage size for the value of this M3U8 attribute.
	*/
	
	size_t size = 0;
	
	switch (attribute->vtype) {
		case M3U8_VATTR_TYPE_QSTRING:
		case M3U8_VATTR_TYPE_ESTRING:
			size = strlen(attribute->value) + 1;
			break;
		default:
			size = m3u8vattr_getsize(attribute->vtype);
			break;
	}
	
	return size;
	
}

size_t m3u8item_getvsize(const struct M3U8Item* const item) {
	/*
	Get the storage size for the value of this M3U8 item.
	*/
	
	size_t size = 0;
	
	switch (item->vtype) {
		case M3U8_VITEM_TYPE_ESTRING:
		case M3U8_VITEM_TYPE_USTRING:
			size = strlen(item->value) + 1;
			break;
		default:
			size = m3u8vitem_getsize(item->vtype);
			break;
	}
	
	return size;
	
}
