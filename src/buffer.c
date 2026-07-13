#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"

int buffer_init(buffer_t* const buffer, const size_t size) {
	
	buffer->size = size;
	buffer->data = malloc(buffer->size);
	buffer->offset = 0;
	
	if (buffer->data == NULL) {
		return -1;
	}
	
	return 0;
	
}

int buffer_append(buffer_t* const buffer, const char* const data, const size_t size) {
	
	const size_t wsize = (buffer->offset + size);
	
	if (wsize > (buffer->size - 1)) {
		return -1;
	}
	
	memcpy(buffer->data + buffer->offset, data, size);
	
	buffer->data[wsize] = '\0';
	buffer->offset += size;
	
	return 0;
	
}

void buffer_free(buffer_t* const buffer) {
	
	buffer->size = 0;
	buffer->offset = 0;
	free(buffer->data);
	
}
