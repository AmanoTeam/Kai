#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "m3u8.h"
#include "m3u8utils.h"
#include "m3u8stream.h"
#include "errors.h"

/*
struct OrderedSetItem {
	int type;
	void* value;
};

struct OrderedSet {
	size_t offset;
	size_t size;
	struct OrderedSetItem* items;
};

static const size_t MIN_SET_SIZE = 128;

int orderedset_exists(
	const struct OrderedSet* const ordered_set,
	const int type,
	const void* const value
) {
	
	size_t index = 0;
	int exists = 0;
	
	for (index = 0; index < ordered_set->offset; index++) {
		const struct OrderedSetItem* const item = &ordered_set->items[index];
		exists = strcmp(value, item->value) == 0;
		
		if (exists) {
			break;
		}
	}
	
	return exists;
	
}

static PyObject* orderedset_init(PyObject* self, PyObject* args)
{
	
	const char *command;
	int sts;

	if (!PyArg_ParseTuple(args, "s", &command))
		return NULL;
	
	
	uintptr_t pointer = 0;
	
	struct OrderedSet* ordered_set = malloc(sizeof(struct OrderedSet));
	
	if (ordered_set == NULL) {
		return PyErr_NoMemory();
	}
	
	ordered_set->offset = 0;
	ordered_set->size = sizeof(struct OrderedSetItem) * MIN_SET_SIZE;
	ordered_set->items = malloc(ordered_set->size);
	
	if (ordered_set->items == NULL) {
		return PyErr_NoMemory();
	}
	
	pointer = (uintptr_t) ordered_set;
	
	return PyLong_FromLongLong(pointer);
	
}

static PyObject* orderedset_add(PyObject* self, PyObject* args)
{
	
	unsigned long long pointer = 0;
	const char* value = NULL;
	
	char* vitem = NULL;
	
	struct OrderedSet* ordered_set = NULL;
	struct OrderedSetItem item = {0};
	
	if (!PyArg_ParseTuple(args, "Ks", &pointer, &value)) {
		return NULL;
	}
	
	ordered_set = (struct OrderedSet*) pointer;
	
	if (orderedset_exists(ordered_set, 0, value)) {
		return PyBool_FromLong(0);
	}
	
	vitem = malloc(strlen(value) + 1);
	
	if (vitem == NULL) {
		return PyErr_NoMemory();
	}
	
	strcpy(vitem, value);
	
	item.value = vitem;
	
	ordered_set->items[ordered_set->offset++] = item;
	
	if (ordered_set->offset * sizeof(*ordered_set->items) == ordered_set->size) {
		size_t size = ordered_set->size * 2;
		struct OrderedSetItem* items = realloc(ordered_set->items, size);
		
		if (items == NULL) {
			return PyErr_NoMemory();
		}
		
		ordered_set->size = size;
		ordered_set->items = items;
	}
	
	return PyBool_FromLong(1);
	
}


static PyObject* orderedset_size(PyObject* self, PyObject* args)
{
	
	unsigned long long pointer = 0;
	struct OrderedSet* ordered_set = NULL;
	
	if (!PyArg_ParseTuple(args, "K", &pointer)) {
		return NULL;
	}
	
	ordered_set = (struct OrderedSet*) pointer;
	
	return PyLong_FromLongLong(ordered_set->offset);
	
}

static PyObject* orderedset_getitem(PyObject* self, PyObject* args)
{
	
	unsigned long long pointer = 0;
	unsigned long long position = 0;
	
	struct OrderedSet* ordered_set = NULL;
	struct OrderedSetItem* item = NULL;
	
	if (!PyArg_ParseTuple(args, "KK", &pointer, &position)) {
		return NULL;
	}
	
	ordered_set = (struct OrderedSet*) pointer;
	
	if (position > ordered_set->offset) {
		return NULL;
	}
	
	item = &ordered_set->items[position];
	
	return PyUnicode_FromString(item->value);
	
}
*/

static PyObject* _m3u8stream_init(
	PyObject* self,
	PyObject* args
) {
	/*
	const char *command;
	int sts;

	if (!PyArg_ParseTuple(args, "s", &command))
		return NULL;
	*/
	
	uintptr_t pointer = 0;
	
	struct M3U8Stream* const stream = malloc(sizeof(struct M3U8Stream));
	
	if (stream == NULL) {
		return PyErr_NoMemory();
	}
	
	memset(stream, 0, sizeof(*stream));
	
	pointer = (uintptr_t) stream;
	
	return PyLong_FromLongLong(pointer);
	
}

static PyObject* _m3u8stream_load(
	PyObject* self,
	PyObject* args
) {
	
	int err = M3U8ERR_SUCCESS;
	
	struct M3U8Stream* stream = NULL;
	
	unsigned long long pointer = 0;
	const char* something = NULL;
	const char* base_uri = NULL;
	
	if (!PyArg_ParseTuple(args, "Ksz", &pointer, &something, &base_uri)) {
		return NULL;
	}
	
	stream = (struct M3U8Stream*) pointer;
	
	err = m3u8stream_load(stream, something, base_uri);
	
	return PyLong_FromLongLong(err);
	
}

static PyObject* _m3u8stream_load_url(
	PyObject* self,
	PyObject* args
) {
	
	int err = M3U8ERR_SUCCESS;
	
	struct M3U8Stream* stream = NULL;
	
	unsigned long long pointer = 0;
	const char* url = NULL;
	const char* base_uri = NULL;
	
	if (!PyArg_ParseTuple(args, "Ksz", &pointer, &url, &base_uri)) {
		return NULL;
	}
	
	stream = (struct M3U8Stream*) pointer;
	
	err = m3u8stream_load_url(stream, url, base_uri);
	
	return PyLong_FromLongLong(err);
	
}

static PyObject* _m3u8stream_load_file(
	PyObject* self,
	PyObject* args
) {
	
	int err = M3U8ERR_SUCCESS;
	
	struct M3U8Stream* stream = NULL;
	
	unsigned long long pointer = 0;
	const char* filename = NULL;
	const char* base_uri = NULL;
	
	if (!PyArg_ParseTuple(args, "Ksz", &pointer, &filename, &base_uri)) {
		return NULL;
	}
	
	stream = (struct M3U8Stream*) pointer;
	
	err = m3u8stream_load_file(stream, filename, base_uri);
	
	return PyLong_FromLongLong(err);
	
}

static PyObject* get_string(const char* const value) {
	
	if (value == NULL) {
		return Py_None;
	}
	
	return PyUnicode_FromString(value);
	
}

static PyObject* get_enum(const char* const name, const biguint_t value) {
	
	PyObject* result = NULL;
	
	PyObject* arg = NULL;
	PyObject* args = NULL;
	
	PyObject* class = NULL;
	PyObject* module = NULL;
	
	if (value == 0) {
		result = Py_None;
		goto end;
	}
    
    module = PyImport_ImportModule("kai");
    
    if (module == NULL) {
		goto end;
	}
	
	class = PyObject_GetAttrString(module, name);
	
	if (class == NULL) {
		goto end;
	}
	
	arg = PyLong_FromLongLong(value);
	
	if (arg == NULL) {
		goto end;
	}
	
	args = Py_BuildValue("(O)", arg);
			
	if (args == NULL) {
		goto end;
	}
	
	result = PyObject_Call(class, args, NULL);
	
	if (result == NULL) {
		goto end;
	}
	
	end:;
	
	Py_XDECREF(module);
	Py_XDECREF(class);
	Py_XDECREF(arg);
	Py_XDECREF(args);
	
	return result;
	
}

static PyObject* get_uint(const biguint_t value) {
	
	PyObject* result = NULL;
	
	if (value == 0) {
		return Py_None;
	}
	
	result = PyLong_FromLongLong(value);
	
	return result;
	
}

static PyObject* get_byterange(const struct M3U8ByteRange* const byterange) {
	
	PyObject* module = NULL;
	PyObject* class = NULL;
	PyObject* args = NULL;
	
	PyObject* length = NULL;
	PyObject* offset = NULL;
	
	PyObject* result = NULL;
	
	module = PyImport_ImportModule("kai");
    
	if (module == NULL) {
		goto end;
	}
	
	length = PyLong_FromLongLong(byterange->length);
	
	if (length == NULL) {
		goto end;
	}
	
	offset = PyLong_FromLongLong(byterange->offset);
	
	if (offset == NULL) {
		goto end;
	}
	
	class = PyObject_GetAttrString(module, "M3U8ByteRange");
	
	if (class == NULL) {
		goto end;
	}
	
	args = Py_BuildValue(
		"(OO)", 
		length,
		offset
	);
	
	if (args == NULL) {
		goto end;
	}
	
	result = PyObject_Call(class, args, NULL);
	
	end:;
	
	Py_XDECREF(module);
	Py_XDECREF(class);
	Py_XDECREF(args);
	
	Py_XDECREF(length);
	Py_XDECREF(offset);
	
	return result;
	
}

static PyObject* get_resolution(const struct M3U8Resolution* const resolution) {
	
	PyObject* module = NULL;
	PyObject* class = NULL;
	PyObject* args = NULL;
	
	PyObject* width = NULL;
	PyObject* height = NULL;
	
	PyObject* result = NULL;
	
	module = PyImport_ImportModule("kai");
    
	if (module == NULL) {
		goto end;
	}
	
	width = PyLong_FromLongLong(resolution->width);
	
	if (width == NULL) {
		goto end;
	}
	
	height = PyLong_FromLongLong(resolution->height);
	
	if (height == NULL) {
		goto end;
	}
	
	class = PyObject_GetAttrString(module, "M3U8Resolution");
	
	if (class == NULL) {
		goto end;
	}
	
	args = Py_BuildValue(
		"(OO)", 
		width,
		height
	);
	
	if (args == NULL) {
		goto end;
	}
	
	result = PyObject_Call(class, args, NULL);
	
	end:;
	
	Py_XDECREF(module);
	Py_XDECREF(class);
	Py_XDECREF(args);
	
	Py_XDECREF(width);
	Py_XDECREF(height);
	
	return result;
	
}

static PyObject* get_list(const char* const name, PyObject* items) {
	
	PyObject* module = NULL;
	PyObject* class = NULL;
	PyObject* args = NULL;
	
	PyObject* result = NULL;
	
	module = PyImport_ImportModule("kai");
    
	if (module == NULL) {
		goto end;
	}
	
	class = PyObject_GetAttrString(module, name);
	
	if (class == NULL) {
		goto end;
	}
	
	args = Py_BuildValue(
		"(O)", 
		items
	);
	
	if (args == NULL) {
		goto end;
	}
	
	result = PyObject_Call(class, args, NULL);
	
	end:;
	
	Py_XDECREF(module);
	Py_XDECREF(class);
	Py_XDECREF(args);
	
	return result;
	
}

static PyObject* get_segments(
	const struct M3U8Stream* const root,
	const struct M3U8Stream* const stream
) {
	
	size_t index = 0;
	size_t subindex = 0;
	
	int status = 0;
	
	int err = M3U8ERR_SUCCESS;
	
	const struct M3U8BaseURI* const base_uri = m3u8playlist_geturi(&root->playlist);
	
	char* resolved_uri = NULL;
	
	PyObject* key = NULL;
	PyObject* method = NULL;
	PyObject* uri = NULL;
	PyObject* iv = NULL;
	PyObject* keyformat = NULL;
	PyObject* keyformatversions = NULL;
	PyObject* tag = NULL;
	PyObject* duration = NULL;
	PyObject* title = NULL;
	PyObject* byterange = NULL;
	PyObject* length = NULL;
	PyObject* offset = NULL;
	PyObject* bitrate = NULL;
	
	PyObject* segments = NULL;
	
	PyObject* subitem = NULL;
	PyObject* items = NULL;
	
	PyObject* module = NULL;
	PyObject* class = NULL;
	PyObject* args = NULL;
	
	items = PyList_New(0);
	
	if (items == NULL) {
		status = 1;
		goto end;
	}
	
	module = PyImport_ImportModule("kai");
    
	if (module == NULL) {
		status = 1;
		goto end;
	}
	
	for (index = 0; index < stream->offset; index++) {
		const struct M3U8StreamItem* const item = &stream->items[index];
		
		if (!(item->type == M3U8_STREAM_SEGMENT || item->type == M3U8_STREAM_MAP)) {
			continue;
		}
		
		switch (item->type) {
			case M3U8_STREAM_SEGMENT: {
				const struct M3U8Segment* const segment = ((struct M3U8Segment*) item->item);
				
				/* Key */
				key = Py_None;
				
				if (segment->key.uri != NULL) {
					/* Key -> Method */
					Py_XDECREF(method);
					
					method = get_enum("M3U8EncryptionMethod", segment->key.method);
					
					if (method == NULL) {
						status = 1;
						goto end;
					}
					
					/* Key -> URI */
					Py_XDECREF(uri);
					
					free(resolved_uri);
					
					err = m3u8uri_resolve(base_uri, segment->key.uri, &resolved_uri);
					
					uri = get_string((err == M3U8ERR_SUCCESS) ? resolved_uri : segment->key.uri);
					
					if (uri == NULL) {
						status = 1;
						goto end;
					}
					
					/* Key -> IV */
					Py_XDECREF(iv);
					
					iv = Py_None;
					
					if (segment->key.iv.offset > 0) {
						Py_DECREF(iv);
						
						iv = PyByteArray_FromStringAndSize(
							segment->key.iv.data,
							segment->key.iv.offset
						);
					}
					
					if (iv == NULL) {
						status = 1;
						goto end;
					}
					
					/* Key -> Key format */
					Py_XDECREF(keyformat);
					
					keyformat = get_string(segment->key.keyformat);
					
					if (keyformat == NULL) {
						status = 1;
						goto end;
					}
					
					/* Key -> Key format versions */
					Py_XDECREF(keyformatversions);
					
					keyformatversions = Py_None;
					
					if (segment->key.keyformatversions.offset > 0) {
						Py_DECREF(keyformatversions);
						
						keyformatversions = PyList_New(segment->key.keyformatversions.offset);
						
						if (keyformatversions == NULL) {
							status = 1;
							goto end;
						}
						
						for (subindex = 0; subindex < segment->key.keyformatversions.offset; subindex++) {
							const int value = segment->key.keyformatversions.items[subindex];
							
							PyObject* item = PyLong_FromLongLong(value);
							
							if (item == NULL) {
								status = 1;
								goto end;
							}
							
							status = PyList_SetItem(keyformatversions, subindex, item);
							
							Py_DECREF(item);
							
							if (status != 0) {
								status = 1;
								goto end;
							}
						}
						
						Py_DECREF(keyformatversions);
						
						keyformatversions = get_list("M3U8KeyFormatVersions", keyformatversions);
					}
					
					/* Key -> Tag */
					Py_XDECREF(tag);
					
					tag = PyLong_FromLongLong((uintptr_t) segment->key.tag);
					
					if (tag == NULL) {
						status = 1;
						goto end;
					}
					
					Py_XDECREF(class);
					
					class = PyObject_GetAttrString(module, "M3U8Key");
					
					if (class == NULL) {
						status = 1;
						goto end;
					}
					
					Py_XDECREF(args);
					
					args = Py_BuildValue(
						"(OOOOOO)", 
						method,
						uri,
						iv,
						keyformat,
						keyformatversions,
						tag
					);
					
					if (args == NULL) {
						status = 1;
						goto end;
					}
					
					Py_XDECREF(key);
					
					key = PyObject_Call(class, args, NULL);
				}
				
				if (key == NULL) {
					status = 1;
					goto end;
				}
				
				/* Duration */
				Py_XDECREF(duration);
				
				duration = PyLong_FromLongLong(segment->duration);
				
				if (duration == NULL) {
					status = 1;
					goto end;
				}
				
				/* Title */
				Py_XDECREF(title);
				
				title = get_string(segment->title);
				
				if (title == NULL) {
					status = 1;
					goto end;
				}
				
				/* Byte range */
				Py_XDECREF(byterange);
				
				byterange = Py_None;
				
				if (segment->byterange.length > 0) {
					Py_DECREF(byterange);
					byterange = get_byterange(&segment->byterange);
				}
				
				if (byterange == NULL) {
					status = 1;
					goto end;
				}
				
				/* Bitrate */
				Py_XDECREF(bitrate);
				
				bitrate = get_uint(segment->bitrate);
					
				if (bitrate == NULL) {
					status = 1;
					goto end;
				}
				
				/* URI */
				Py_XDECREF(uri);
				
				free(resolved_uri);
				
				err = m3u8uri_resolve(base_uri, segment->uri, &resolved_uri);
				
				uri = get_string((err == M3U8ERR_SUCCESS) ? resolved_uri : segment->uri);
				
				if (uri == NULL) {
					status = 1;
					goto end;
				}
				
				/* Tag */
				Py_XDECREF(tag);
				
				tag = PyLong_FromLongLong((uintptr_t) segment->tag);
				
				if (tag == NULL) {
					status = 1;
					goto end;
				}
				
				Py_XDECREF(class);
				
				class = PyObject_GetAttrString(module, "M3U8Segment");
				
				if (class == NULL) {
					status = 1;
					goto end;
				}
				
				Py_XDECREF(args);
				
				args = Py_BuildValue(
					"(OOOOOOO)", 
					key,
					duration,
					title,
					byterange,
					bitrate,
					uri,
					tag
				);
				
				if (args == NULL) {
					status = 1;
					goto end;
				}
				
				Py_XDECREF(subitem);
				
				subitem = PyObject_Call(class, args, NULL);
				
				if (subitem == NULL) {
					status = 1;
					goto end;
				}
				
				break;
			}
			case M3U8_STREAM_MAP: {
				const struct M3U8Map* const map = ((struct M3U8Map*) item->item);
				
				/* URI */
				Py_XDECREF(uri);
				
				free(resolved_uri);
				
				err = m3u8uri_resolve(base_uri, map->uri, &resolved_uri);
				
				uri = get_string((err == M3U8ERR_SUCCESS) ? resolved_uri : map->uri);
				
				if (uri == NULL) {
					status = 1;
					goto end;
				}
				
				/* Byte range */
				Py_XDECREF(byterange);
				
				byterange = Py_None;
				
				if (map->byterange.length > 0) {
					Py_DECREF(byterange);
					byterange = get_byterange(&map->byterange);
				}
				
				if (byterange == NULL) {
					status = 1;
					goto end;
				}
				
				/* Tag */
				Py_XDECREF(tag);
				
				tag = PyLong_FromLongLong((uintptr_t) map->tag);
				
				if (tag == NULL) {
					status = 1;
					goto end;
				}
				
				Py_XDECREF(class);
				
				class = PyObject_GetAttrString(module, "M3U8Map");
				
				if (class == NULL) {
					status = 1;
					goto end;
				}
				
				Py_XDECREF(args);
				
				args = Py_BuildValue(
					"(OOO)", 
					uri,
					byterange,
					tag
				);
				
				if (args == NULL) {
					status = 1;
					goto end;
				}
				
				Py_XDECREF(subitem);
				
				subitem = PyObject_Call(class, args, NULL);
				
				if (subitem == NULL) {
					status = 1;
					goto end;
				}
				
				break;
			}
			default: {
				break;
			}
		}
		
		status = PyList_Append(items, subitem);
		
		if (status != 0) {
			status = 1;
			goto end;
		}
	}
	
	segments = get_list("M3U8Segments", items);
	
	end:;
	
	Py_XDECREF(key);
	Py_XDECREF(method);
	Py_XDECREF(uri);
	Py_XDECREF(iv);
	Py_XDECREF(keyformat);
	Py_XDECREF(keyformatversions);
	Py_XDECREF(tag);
	Py_XDECREF(duration);
	Py_XDECREF(title);
	Py_XDECREF(byterange);
	Py_XDECREF(length);
	Py_XDECREF(offset);
	Py_XDECREF(bitrate);
	Py_XDECREF(subitem);
	Py_XDECREF(items);
	
	if (status != 0) {
		Py_XDECREF(segments);
		segments = NULL;
	}
	
	return segments;
	
}

static PyObject* topy(
	const struct M3U8Stream* const root,
	const struct M3U8StreamItem* const item
) {
	
	int err = M3U8ERR_SUCCESS;
	
	PyObject* module = NULL;
	PyObject* args = NULL;
	PyObject* class = NULL;
	
	PyObject* type = NULL;
	PyObject* uri = NULL;
	PyObject* group_id = NULL;
	PyObject* language = NULL;
	PyObject* assoc_language = NULL;
	PyObject* name = NULL;
	PyObject* _default = NULL;
	PyObject* autoselect = NULL;
	PyObject* forced = NULL;
	PyObject* instream_id = NULL;
	PyObject* characteristics = NULL;
	PyObject* channels = NULL;
	PyObject* stream = NULL;
	PyObject* duration = NULL;
	PyObject* average_duration = NULL;
	PyObject* segments = NULL;
	PyObject* tag = NULL;
	PyObject* resolution = NULL;
	PyObject* bandwidth = NULL;
	PyObject* average_bandwidth = NULL;
	PyObject* program_id = NULL;
	PyObject* codecs = NULL;
	PyObject* frame_rate = NULL;
	PyObject* hdcp_level = NULL;
	PyObject* audio = NULL;
	PyObject* video = NULL;
	PyObject* subtitles = NULL;
	PyObject* closed_captions = NULL;
	PyObject* size = NULL;
	PyObject* version = NULL;
	PyObject* value = NULL;
	PyObject* data_id = NULL;
	
	PyObject* result = NULL;
	
	const struct M3U8BaseURI* const base_uri = m3u8playlist_geturi(&root->playlist);
	
	char* resolved_uri = NULL;
	
	module = PyImport_ImportModule("kai");
    
    if (module == NULL) {
		return NULL;
	}
	
	switch (item->type) {
		case M3U8_STREAM_SESSION_DATA: {
			const struct M3U8SessionData* const session_data = item->item;
			
			/* Data ID */
			Py_XDECREF(data_id);
			
			data_id = get_string(session_data->data_id);
			
			if (data_id == NULL) {
				return NULL;
			}
			
			/* Value */
			Py_XDECREF(value);
			
			value = get_string(session_data->value);
			
			if (value == NULL) {
				return NULL;
			}
			
			/* URI */
			Py_XDECREF(uri);
			
			err = m3u8uri_resolve(base_uri, session_data->uri, &resolved_uri);
			
			uri = get_string((err == M3U8ERR_SUCCESS) ? resolved_uri : session_data->uri);
			
			if (uri == NULL) {
				return NULL;
			}
			
			/* Language */
			Py_XDECREF(language);
			
			language = get_string(session_data->language);
			
			if (language == NULL) {
				return NULL;
			}
			
			/* Tag */
			Py_XDECREF(tag);
			
			tag = PyLong_FromLongLong((uintptr_t) session_data->tag);
			
			if (tag == NULL) {
				return NULL;
			}
			
			Py_XDECREF(args);
			
			args = Py_BuildValue(
				"(OOOOO)", 
				data_id,
				value,
				uri,
				language,
				tag
			);
			
			if (args == NULL) {
				return NULL;
			}
			
			Py_XDECREF(class);
			
			class = PyObject_GetAttrString(module, "M3U8SessionData");
			
			if (class == NULL) {
				return NULL;
			}
			
			result = PyObject_Call(class, args, NULL);
			
			break;
		}
		case M3U8_STREAM_VERSION: {
			const struct M3U8Version* const ver = item->item;
			
			/* Version */
			Py_XDECREF(version);
			
			version = PyLong_FromLongLong(ver->version);
			
			if (version == NULL) {
				return NULL;
			}
			
			/* Tag */
			Py_XDECREF(tag);
			
			tag = PyLong_FromLongLong((uintptr_t) ver->tag);
			
			if (tag == NULL) {
				return NULL;
			}
			
			Py_XDECREF(args);
			
			args = Py_BuildValue(
				"(OO)", 
				version,
				tag
			);
			
			if (args == NULL) {
				return NULL;
			}
			
			Py_XDECREF(class);
			
			class = PyObject_GetAttrString(module, "M3U8Version");
			
			if (class == NULL) {
				return NULL;
			}
			
			result = PyObject_Call(class, args, NULL);
			
			break;
		}
		case M3U8_STREAM_MEDIA: {
			const struct M3U8Media* const media = item->item;
			
			/* Type */
			Py_XDECREF(type);
			
			type = get_enum("M3U8MediaType", media->type);
			
			if (type == NULL) {
				return NULL;
			}
			
			/* URI */
			Py_XDECREF(uri);
			
			err = m3u8uri_resolve(base_uri, media->uri, &resolved_uri);
			
			uri = get_string((err == M3U8ERR_SUCCESS) ? resolved_uri : media->uri);
			
			if (uri == NULL) {
				return NULL;
			}
			
			/* Group ID */
			Py_XDECREF(group_id);
			
			group_id = get_string(media->group_id);
			
			if (group_id == NULL) {
				return NULL;
			}
			
			/* Language */
			Py_XDECREF(language);
			
			language = get_string(media->language);
			
			if (language == NULL) {
				return NULL;
			}
			
			/* Associated language */
			Py_XDECREF(assoc_language);
			
			assoc_language = get_string(media->assoc_language);
			
			if (assoc_language == NULL) {
				return NULL;
			}
			
			/* Name */
			Py_XDECREF(name);
			
			name = get_string(media->name);
			
			if (name == NULL) {
				return NULL;
			}
			
			/* Default */
			Py_XDECREF(_default);
			
			_default = PyBool_FromLong(media->default_);
			
			/* Autoselect */
			Py_XDECREF(autoselect);
			
			autoselect = PyBool_FromLong(media->autoselect);
			
			/* Forced */
			Py_XDECREF(forced);
			
			forced = PyBool_FromLong(media->forced);
			
			/* InStream ID */
			Py_XDECREF(instream_id);
			
			instream_id = get_enum("M3U8ClosedCaption", media->instream_id);
			
			if (instream_id == NULL) {
				return NULL;
			}
			
			/* Characteristics */
			Py_XDECREF(characteristics);
			
			characteristics = get_string(media->characteristics);
			
			if (characteristics == NULL) {
				return NULL;
			}
			
			/* Channels */
			Py_XDECREF(channels);
			
			channels = get_string(media->channels);
			
			if (channels == NULL) {
				return NULL;
			}
			
			/* Stream */
			Py_XDECREF(stream);
			
			stream = get_segments(root, &media->stream);
			
			if (stream == NULL) {
				return NULL;
			}
			
			/* Duration */
			Py_XDECREF(duration);
			
			duration = PyLong_FromLongLong(media->duration);
			
			if (duration == NULL) {
				return NULL;
			}
			
			/* Average duration */
			Py_XDECREF(average_duration);
			
			average_duration = PyLong_FromLongLong(media->average_duration);
			
			if (average_duration == NULL) {
				return NULL;
			}
			
			/* Segments */
			Py_XDECREF(segments);
			
			segments = PyLong_FromLongLong(media->segments);
			
			if (segments == NULL) {
				return NULL;
			}
			
			/* Tag */
			Py_XDECREF(tag);
			
			tag = PyLong_FromLongLong((uintptr_t) media->tag);
			
			if (tag == NULL) {
				return NULL;
			}
			
			Py_XDECREF(args);
			
			args = Py_BuildValue(
				"(OOOOOOOOOOOOOOOOO)", 
				type,
				uri,
				group_id,
				language,
				assoc_language,
				name,
				_default,
				autoselect,
				forced,
				instream_id,
				characteristics,
				channels,
				stream,
				duration,
				average_duration,
				segments,
				tag
			);
			
			if (args == NULL) {
				return NULL;
			}
			
			Py_XDECREF(class);
			
			class = PyObject_GetAttrString(module, "M3U8Media");
			
			if (class == NULL) {
				return NULL;
			}
			
			result = PyObject_Call(class, args, NULL);
			
			break;
		}
		case M3U8_STREAM_VARIANT_STREAM: {
			const struct M3U8VariantStream* const variant_stream = item->item;
			
			/* Bandwidth */
			Py_XDECREF(bandwidth);
			
			bandwidth = get_uint(variant_stream->bandwidth);
			
			if (bandwidth == NULL) {
				return NULL;
			}
			
			/* Average bandwidth */
			Py_XDECREF(average_bandwidth);
			
			average_bandwidth = get_uint(variant_stream->average_bandwidth);
			
			if (average_bandwidth == NULL) {
				return NULL;
			}
			
			/* Program ID */
			Py_XDECREF(program_id);
			
			program_id = get_uint(variant_stream->program_id);
			
			if (program_id == NULL) {
				return NULL;
			}
			
			/* Codecs */
			Py_XDECREF(codecs);
			
			codecs = get_string(variant_stream->codecs);
			
			if (codecs == NULL) {
				return NULL;
			}
			
			/* Resolution */
			Py_XDECREF(resolution);
			
			resolution = get_resolution(&variant_stream->resolution);
			
			if (resolution == NULL) {
				return NULL;
			}
			
			/* Frame rate */
			Py_XDECREF(frame_rate);
			
			frame_rate = get_uint(variant_stream->frame_rate);
			
			if (frame_rate == NULL) {
				return NULL;
			}
			
			/* HDCP level */
			Py_XDECREF(hdcp_level);
			
			hdcp_level = get_enum("M3U8HDCPLevel", variant_stream->hdcp_level);
			
			if (hdcp_level == NULL) {
				return NULL;
			}
			
			/* Audio */
			Py_XDECREF(audio);
			
			audio = Py_None;
			
			if (variant_stream->audio != NULL) {
				struct M3U8StreamItem subitem = {
					.type = M3U8_STREAM_MEDIA,
					.item = variant_stream->audio
				};
				
				Py_XDECREF(audio);
				
				audio = topy(root, &subitem);
			}
			
			if (audio == NULL) {
				return NULL;
			}
			
			/* Video */
			video = Py_None;
			
			if (variant_stream->video != NULL) {
				struct M3U8StreamItem subitem = {
					.type = M3U8_STREAM_MEDIA,
					.item = variant_stream->video
				};
				
				Py_XDECREF(video);
				
				video = topy(root, &subitem);
			}
			
			if (video == NULL) {
				return NULL;
			}
			
			/* Subtitles */
			subtitles = Py_None;
			
			if (variant_stream->subtitles != NULL) {
				struct M3U8StreamItem subitem = {
					.type = M3U8_STREAM_MEDIA,
					.item = variant_stream->subtitles
				};
				
				Py_XDECREF(subtitles);
				
				subtitles = topy(root, &subitem);
			}
			
			if (subtitles == NULL) {
				return NULL;
			}
			
			/* Closed captions */
			closed_captions = Py_None;
			
			if (variant_stream->subtitles != NULL) {
				struct M3U8StreamItem subitem = {
					.type = M3U8_STREAM_MEDIA,
					.item = variant_stream->closed_captions
				};
				
				Py_XDECREF(subtitles);
				
				closed_captions = topy(root, &subitem);
			}
			
			if (closed_captions == NULL) {
				return NULL;
			}
			
			/* URI */
			Py_XDECREF(uri);
			
			err = m3u8uri_resolve(base_uri, variant_stream->uri, &resolved_uri);
			
			uri = get_string((err == M3U8ERR_SUCCESS) ? resolved_uri : variant_stream->uri);
			
			if (uri == NULL) {
				return NULL;
			}
			
			/* Stream */
			Py_XDECREF(stream);
			
			stream = get_segments(root, &variant_stream->stream);
			
			if (stream == NULL) {
				return NULL;
			}
			
			/* Duration */
			Py_XDECREF(duration);
			
			duration = PyLong_FromLongLong(variant_stream->duration);
			
			if (duration == NULL) {
				return NULL;
			}
			
			/* Average duration */
			Py_XDECREF(average_duration);
			
			average_duration = PyLong_FromLongLong(variant_stream->average_duration);
			
			if (average_duration == NULL) {
				return NULL;
			}
			
			/* Size */
			Py_XDECREF(size);
			
			size = PyLong_FromLongLong(variant_stream->size);
			
			if (size == NULL) {
				return NULL;
			}
			
			/* Segments */
			Py_XDECREF(segments);
			
			segments = PyLong_FromLongLong(variant_stream->segments);
			
			if (segments == NULL) {
				return NULL;
			}
			
			/* Tag */
			Py_XDECREF(tag);
			
			tag = PyLong_FromLongLong((uintptr_t) variant_stream->tag);
			
			if (tag == NULL) {
				return NULL;
			}
			
			Py_XDECREF(args);
			
			args = Py_BuildValue(
				"(OOOOOOOOOOOOOOOOOO)", 
				bandwidth,
				average_bandwidth,
				program_id,
				codecs,
				resolution,
				frame_rate,
				hdcp_level,
				audio,
				video,
				subtitles,
				closed_captions,
				uri,
				stream,
				duration,
				average_duration,
				size,
				segments,
				tag
			);
			
			if (args == NULL) {
				return NULL;
			}
			
			Py_XDECREF(class);
			
			class = PyObject_GetAttrString(module, "M3U8VariantStream");
			
			if (class == NULL) {
				return NULL;
			}
			
			result = PyObject_Call(class, args, NULL);
			
			break;
		}
		default: {
			break;
		}
	}
	
	free(resolved_uri);
	
	Py_DECREF(module);
	Py_DECREF(class);
	
	return result;
	
}

static PyObject* _m3u8stream_getstream(
	PyObject* self,
	PyObject* args
) {
	
	size_t index = 0;
	
	int err = M3U8ERR_SUCCESS;
	
	struct M3U8Stream* stream = NULL;
	
	PyObject* kitem = NULL;
	PyObject* items = NULL;
	PyObject* module = NULL;
	
	unsigned long long pointer = 0;
	
	if (!PyArg_ParseTuple(args, "K", &pointer)) {
		return NULL;
	}
	
	stream = (struct M3U8Stream*) pointer;
	
	items = PyList_New(0);
	
	if (items == NULL) {
		return NULL;
	}
	
	module = PyImport_ImportModule("kai");
    
    if (module == NULL) {
		return NULL;
	}
	
	for (index = 0; index < stream->offset; index++) {
		const struct M3U8StreamItem* const item = &stream->items[index];
		
		switch (item->type) {
			case M3U8_STREAM_SESSION_DATA:
			case M3U8_STREAM_MEDIA:
			case M3U8_STREAM_VERSION:
			case M3U8_STREAM_VARIANT_STREAM: {
				kitem = topy(stream, item);
				PyList_Append(items, kitem);
				break;
			}
		}
	}
	
	return items;
	
}

static PyMethodDef methods[] = {
	{
		"m3u8stream_init",
		_m3u8stream_init,
		METH_VARARGS,
		"Execute a shell command."
	},
	{
		"m3u8stream_load",
		_m3u8stream_load,
		METH_VARARGS,
		"Execute a shell command."
	},
	{
		"m3u8stream_load_url",
		_m3u8stream_load_url,
		METH_VARARGS,
		"Execute a shell command."
	},
	{
		"m3u8stream_load_file",
		_m3u8stream_load_file,
		METH_VARARGS,
		"Execute a shell command."
	},
	{
		"m3u8stream_getstream",
		_m3u8stream_getstream,
		METH_VARARGS,
		"Execute a shell command."
	},
	{NULL, NULL, 0, NULL}		/* Sentinel */
};

static struct PyModuleDef module = {
	PyModuleDef_HEAD_INIT,
	"_kai",
	NULL,
	-1,
	methods
};

PyMODINIT_FUNC PyInit__kai(void) {
	
	return PyModule_Create(&module);
	
}