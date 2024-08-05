#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

#include "m3u8utils.h"
#include "m3u8parser.h"
#include "errors.h"
#include "m3u8types.h"
#include "hex.h"

int m3u8parser_getuint(const char* const source, void** destination) {
	
	biguint_t val = 0;
	
	if (!isuint(source)) {
		return M3U8ERR_PARSER_INVALID_UINT;
	}
	
	val = strtobui(source, NULL, 10);
	
	if (errno == ERANGE) {
		return M3U8ERR_PARSER_INVALID_UINT;
	}
	
	*destination = malloc(sizeof(val));
	
	if (*destination == NULL) {
		return M3U8ERR_MEMORY_ALLOCATE_FAILURE;
	}
	
	memcpy(*destination, &val, sizeof(val));
	
	return M3U8ERR_SUCCESS;
	
}

int m3u8parser_getfloat(const char* const source, void** destination) {
	
	bigfloat_t val = 0;
	
	if (!isfloat(source)) {
		return M3U8ERR_PARSER_INVALID_FLOAT;
	}
	
	val = strtobf(source, NULL);
	
	if (errno == ERANGE) {
		return M3U8ERR_PARSER_INVALID_FLOAT;
	}
	
	*destination = malloc(sizeof(val));
	
	if (*destination == NULL) {
		return M3U8ERR_MEMORY_ALLOCATE_FAILURE;
	}
	
	memcpy(*destination, &val, sizeof(val));
	
	return M3U8ERR_SUCCESS;
	
}

int m3u8parser_getufloat(const char* const source, void** destination) {
	
	bigfloat_t val = 0;
	
	if (!isufloat(source)) {
		return M3U8ERR_PARSER_INVALID_UFLOAT;
	}
	
	val = strtobf(source, NULL);
	
	if (errno == ERANGE) {
		return M3U8ERR_PARSER_INVALID_UFLOAT;
	}
	
	*destination = malloc(sizeof(val));
	
	if (*destination == NULL) {
		return M3U8ERR_MEMORY_ALLOCATE_FAILURE;
	}
	
	memcpy(*destination, &val, sizeof(val));
	
	return M3U8ERR_SUCCESS;
	
}

int m3u8parser_getbrange(const char* const source, void** destination) {
	
	const char* ptr = NULL;
	
	struct M3U8ByteRange range = {0};
	
	if (!isbrange(source)) {
		return M3U8ERR_PARSER_INVALID_BRANGE;
	}
	
	range.length = strtobui(source, NULL, 10);
	
	if (errno == ERANGE) {
		return M3U8ERR_PARSER_INVALID_BRANGE;
	}
	
	range.offset = -1;
	
	ptr = strstr(source, "@");
	
	if (ptr != NULL && *(ptr + 1) != '\0') {
		range.offset = strtobui(ptr + 1, NULL, 10);
		
		if (errno == ERANGE) {
			return M3U8ERR_PARSER_INVALID_BRANGE;
		}
	}
	
	*destination = malloc(sizeof(range));
	
	if (*destination == NULL) {
		return M3U8ERR_MEMORY_ALLOCATE_FAILURE;
	}
	
	memcpy(*destination, &range, sizeof(range));
	
	return M3U8ERR_SUCCESS;
	
}

int m3u8parser_getdtime(const char* const source, void** destination) {
	
	struct M3U8DateTime dt = {0};
	size_t index = 0;
	
	if (!isdtime(source)) {
		return M3U8ERR_PARSER_INVALID_DTIME;
	}
	
	dt.year = strtobui(source + index, NULL, 10);
	
	if (errno == ERANGE) {
		return M3U8ERR_PARSER_INVALID_DTIME;
	}
	
	index += 5;
	dt.mon = strtobui(source + index, NULL, 10);
	
	if (errno == ERANGE) {
		return M3U8ERR_PARSER_INVALID_DTIME;
	}
	
	index += 3;
	dt.mday = strtobui(source + index, NULL, 10);
	
	if (errno == ERANGE) {
		return M3U8ERR_PARSER_INVALID_DTIME;
	}
	
	index += 3;
	dt.hour = strtobui(source + index, NULL, 10);
	
	if (errno == ERANGE) {
		return M3U8ERR_PARSER_INVALID_DTIME;
	}
	
	index += 3;
	dt.min = strtobui(source + index, NULL, 10);
	
	if (errno == ERANGE) {
		return M3U8ERR_PARSER_INVALID_DTIME;
	}
	
	index += 3;
	dt.sec = strtobui(source + index, NULL, 10);
	
	if (errno == ERANGE) {
		return M3U8ERR_PARSER_INVALID_DTIME;
	}
	
	index += 2;
	
	dt.msec = 0;
	dt.gmtoff = 100000;
	
	if (source[index] == '.') {
		index++;
		dt.msec = strtobui(source + index, NULL, 10);
		
		if (errno == ERANGE) {
			return M3U8ERR_PARSER_INVALID_DTIME;
		}
		
		while (1) {
			const unsigned char ch = source[index];
			
			if (!isdigit(ch)) {
				break;
			}
			
			index++;
		}
	}
	
	if (source[index] == '-' || source[index] == '+') {
		const int isbehind = (source[index] == '-');
		
		dt.gmtoff = 0;
		
		index++;
		dt.gmtoff += strtobi(source + index, NULL, 10) * 3600;
		
		if (errno == ERANGE) {
			return M3U8ERR_PARSER_INVALID_DTIME;
		}
		
		index += 3;
		dt.gmtoff += strtobi(source + index, NULL, 10) * 60;
		
		if (errno == ERANGE) {
			return M3U8ERR_PARSER_INVALID_DTIME;
		}
		
		if (isbehind) {
			dt.gmtoff = -dt.gmtoff;
		}
	}
	
	*destination = malloc(sizeof(dt));
	
	if (*destination == NULL) {
		return M3U8ERR_MEMORY_ALLOCATE_FAILURE;
	}
	
	memcpy(*destination, &dt, sizeof(dt));
	
	return M3U8ERR_SUCCESS;
	
}

int m3u8parser_getestring(const char* const source, void** destination) {
	
	const size_t size = strlen(source);
	
	if (!isestring(source)) {
		return M3U8ERR_PARSER_INVALID_ESTRING;
	}
	
	*destination = malloc(size + 1);
	
	if (*destination == NULL) {
		return M3U8ERR_MEMORY_ALLOCATE_FAILURE;
	}
	
	memcpy(*destination, source, size);
	((char*) *destination)[size] = '\0';
	
	return M3U8ERR_SUCCESS;
	
}

int m3u8parser_getustring(const char* const source, void** destination) {
	
	const size_t size = strlen(source);
	
	if (size < 1) {
		return M3U8ERR_PARSER_INVALID_USTRING;
	}
	
	*destination = malloc(size + 1);
	
	if (*destination == NULL) {
		return M3U8ERR_MEMORY_ALLOCATE_FAILURE;
	}
	
	memcpy(*destination, source, size);
	((char*) *destination)[size] = '\0';
	
	return M3U8ERR_SUCCESS;
	
}

int m3u8parser_getqstring(const char* const source, void** destination) {
	
	const size_t size = strlen(source);
	
	if (!isqstring(source)) {
		return M3U8ERR_PARSER_INVALID_QSTRING;
	}
	
	*destination = malloc(size - 1);
	
	if (*destination == NULL) {
		return M3U8ERR_MEMORY_ALLOCATE_FAILURE;
	}
	
	memcpy(*destination, source + 1, size - 2);
	((char*) *destination)[size - 2] = '\0';
	
	return M3U8ERR_SUCCESS;
	
}

int m3u8parser_gethexseq(const char* const source, void** destination) {
	
	struct M3U8Bytes bytes = {0};
	
	size_t index = 0;
	size_t size = strlen(source);
	
	if (!ishex(source)) {
		return M3U8ERR_PARSER_INVALID_HEXSEQ;
	}
	
	bytes.size = (size - 2) / 2;
	bytes.data = malloc(bytes.size);
	
	if (bytes.data == NULL) {
		return M3U8ERR_MEMORY_ALLOCATE_FAILURE;
	}
	
	for (index = 2; index < size; index += 2) {
		const unsigned char a = source[index];
		const unsigned char b = source[index + 1];
		
		const unsigned char c = from_hex(a);
		const unsigned char d = from_hex(b);
		
		const unsigned char e = ((c << 4) | d);
		bytes.data[bytes.offset++] = e;
	}
	
	*destination = malloc(sizeof(bytes));
	
	if (*destination == NULL) {
		return M3U8ERR_MEMORY_ALLOCATE_FAILURE;
	}
	
	memcpy(*destination, &bytes, sizeof(bytes));
	
	return M3U8ERR_SUCCESS;
	
}

int m3u8parser_getresolution(const char* const source, void** destination) {
	
	struct M3U8Resolution resolution = {0};
	
	if (!isresolution(source)) {
		return M3U8ERR_ATTRIBUTE_INVALID_RESOLUTION;
	}
	
	resolution.width = strtobui(source, NULL, 10);
	
	if (errno == ERANGE) {
		return M3U8ERR_ATTRIBUTE_INVALID_RESOLUTION;
	}
	
	resolution.height = strtobui(strstr(source, "x") + 1, NULL, 10);
	
	if (errno == ERANGE) {
		return M3U8ERR_ATTRIBUTE_INVALID_RESOLUTION;
	}
	
	*destination = malloc(sizeof(resolution));
	
	if (*destination == NULL) {
		return M3U8ERR_MEMORY_ALLOCATE_FAILURE;
	}
	
	memcpy(*destination, &resolution, sizeof(resolution));

	return M3U8ERR_SUCCESS;
	
}
