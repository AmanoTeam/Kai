#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "query.h"
#include "biggestint.h"
#include "urlencode.h"
#include "urldecode.h"
#include "strsplit.h"
#include "fstream.h"

static const char AND = '&';
static const char EQUAL[] = "=";

enum HTTPQueryNumeric {
	HTTP_QUERY_INT,
	HTTP_QUERY_UINT,
	HTTP_QUERY_FLOAT
};

static int put_parameter(
	struct HTTPQuery* const query,
	struct HTTPQueryParam* const parameter
) {
	
	size_t size = 0;
	struct HTTPQueryParam* parameters = NULL;
	
	if (sizeof(*query->parameters) * (query->offset + 1) > query->size) {
		size = query->size + sizeof(*query->parameters) * (query->offset + 1);
		parameters = realloc(query->parameters, size);
		
		if (parameters == NULL) {
			return -1;
		}
		
		query->size = size;
		query->parameters = parameters;
	}
	
	query->parameters[query->offset++] = *parameter;
	
	return 0;
	
}

struct HTTPQueryParam* query_get_param(
	struct HTTPQuery* const query,
	const char* const key
) {
	
	size_t index = 0;
	
	for (index = 0; index < query->offset; index++) {
		struct HTTPQueryParam* const parameter = &query->parameters[index];
		
		if (strcmp(parameter->key, key) != 0) {
			continue;
		}
		
		return parameter;
	}
	
	return NULL;
	
}

char* query_get_string(
	struct HTTPQuery* const query,
	const char* const key
) {
	
	const struct HTTPQueryParam* const parameter = query_get_param(query, key);
	
	if (parameter == NULL || parameter->value == NULL) {
		return NULL;
	}
	
	return parameter->value;
	
}

static int get_numeric_value(
	const enum HTTPQueryNumeric type,
	const char* const source,
	bigint_storage_t* destination
) {
	
	int err = 0;
	
	switch (type) {
		case HTTP_QUERY_INT: {
			const bigint_t val = strtobi(source, NULL, 10);
			
			if (errno == ERANGE) {
				err = -1;
				break;
			}
			
			memcpy(destination, &val, sizeof(val));
			
			break;
		}
		case HTTP_QUERY_UINT: {
			const biguint_t val = strtobui(source, NULL, 10);
			
			if (errno == ERANGE) {
				err = -1;
				break;
			}
			
			memcpy(destination, &val, sizeof(val));
			
			break;
		}
		case HTTP_QUERY_FLOAT: {
			const bigfloat_t val = strtobf(source, NULL);
			
			if (errno == ERANGE) {
				err = -1;
				break;
			}
			
			memcpy(destination, &val, sizeof(val));
			
			break;
		}
		default:
			break;
	}
	
	return err;
	
}

static int query_get_numeric(
	struct HTTPQuery* const query,
	const enum HTTPQueryNumeric type,
	const char* const key,
	bigint_storage_t* destination
) {
	
	int err = 0;
	
	const char* const source = query_get_string(query, key);
	
	if (source == NULL) {
		return -1;
	}
	
	err = get_numeric_value(type, source, destination);
	
	return err;
	
}

const char* param_get_string(const struct HTTPQueryParam* const param) {
	return param->value;
}

bigint_t param_get_int(const struct HTTPQueryParam* const param) {
	
	bigint_storage_t value = {0};
	const int err = get_numeric_value(HTTP_QUERY_INT, param->value, &value);
	
	if (err != 0) {
		return BIGINT_MAX;
	}
	
	return *(bigint_t*) &value;
	
}

bigint_t query_get_int(
	struct HTTPQuery* const query,
	const char* const key
) {
	
	bigint_storage_t value = {0};
	const int err = query_get_numeric(query, HTTP_QUERY_INT, key, &value);
	
	if (err != 0) {
		return BIGINT_MAX;
	}
	
	return *(bigint_t*) &value;
	
}

biguint_t param_get_uint(const struct HTTPQueryParam* const param) {
	
	bigint_storage_t value = {0};
	const int err = get_numeric_value(HTTP_QUERY_UINT, param->value, &value);
	
	if (err != 0) {
		return BIGINT_MAX;
	}
	
	return *(biguint_t*) &value;
	
}

biguint_t query_get_uint(
	struct HTTPQuery* const query,
	const char* const key
) {
	
	bigint_storage_t value = {0};
	const int err = query_get_numeric(query, HTTP_QUERY_UINT, key, &value);
	
	if (err != 0) {
		return BIGUINT_MAX;
	}
	
	return *(biguint_t*) &value;
	
}

bigfloat_t param_get_float(const struct HTTPQueryParam* const param) {
	
	bigint_storage_t value = {0};
	const int err = get_numeric_value(HTTP_QUERY_FLOAT, param->value, &value);
	
	if (err != 0) {
		return BIGINT_MAX;
	}
	
	return *(bigfloat_t*) &value;
	
}

bigfloat_t query_get_float(
	struct HTTPQuery* const query,
	const char* const key
) {
	
	bigint_storage_t value = {0};
	const int err = query_get_numeric(query, HTTP_QUERY_FLOAT, key, &value);
	
	if (err != 0) {
		return BIGFLOAT_MAX;
	}
	
	return *(bigfloat_t*) &value;
	
}

int query_add_string(
	struct HTTPQuery* const query,
	const char* const key,
	const char* const value
) {
	
	int err = 0;
	
	size_t size = 0;
	
	struct HTTPQueryParam parameter = {0};
	
	size = strlen(key);
	
	if (size > 0) {
		parameter.key = malloc(size + 1);
		
		if (parameter.key == NULL) {
			err = -1;
			goto end;
		}
		
		strcpy(parameter.key, key);
	}
	
	size = strlen(value);
	
	if (size > 0) {
		parameter.value = malloc(size + 1);
		
		if (parameter.value == NULL) {
			err = -1;
			goto end;
		}
		
		strcpy(parameter.value, value);
	}
	
	err = put_parameter(query, &parameter);
	
	end:;
	
	if (err != 0) {
		parameter_free(&parameter);
	}
	
	return err;
	
}

static int query_add_numeric(
	struct HTTPQuery* const query,
	const enum HTTPQueryNumeric type,
	const char* const key,
	const void* const value
) {
	
	int err = 0;
	
	char wtmp[32];
	int wsize = 0;
	
	switch (type) {
		case HTTP_QUERY_INT:
			wsize = snprintf(wtmp, sizeof(wtmp), "%"FORMAT_BIGGEST_INT_T, *(bigint_t*) value);
			break;
		case HTTP_QUERY_UINT:
			wsize = snprintf(wtmp, sizeof(wtmp), "%"FORMAT_BIGGEST_UINT_T, *(biguint_t*) value);
			break;
		case HTTP_QUERY_FLOAT:
			wsize = snprintf(wtmp, sizeof(wtmp), "%"FORMAT_BIGGEST_FLOAT_T, *(bigfloat_t*) value);
			break;
		default:
			break;
	}
	
	if (wsize < 1) {
		err = -1;
		goto end;
	}
	
	err = query_add_string(query, key, wtmp);
	
	end:;
	
	return err;
	
}

int query_add_int(
	struct HTTPQuery* const query,
	const char* const key,
	const bigint_t value
) {
	
	const int err = query_add_numeric(query, HTTP_QUERY_INT, key, &value);
	return err;
	
}

int query_add_uint(
	struct HTTPQuery* const query,
	const char* const key,
	const biguint_t value
) {
	
	const int err = query_add_numeric(query, HTTP_QUERY_UINT, key, &value);
	return err;
	
}

int query_add_float(
	struct HTTPQuery* const query,
	const char* const key,
	const bigfloat_t value
) {
	
	const int err = query_add_numeric(query, HTTP_QUERY_FLOAT, key, &value);
	return err;
	
}

size_t query_stringify(
	const struct HTTPQuery* const query,
	char* const destination
) {
	
	ssize_t size = 0;
	size_t index = 0;
	
	char* end = destination;
	
	const char separator[] = {query->sep, '\0'};
	
	if (destination != NULL) {
		*destination = '\0';
	}
	
	size++;
	
	for (index = 0; index < query->offset; index++) {
		const struct HTTPQueryParam* const parameter = &query->parameters[index];
		
		if (index != 0) {
			if (destination != NULL) {
				strcat(destination, separator);
			}
			
			size += strlen(separator);
		}
		
		if (parameter->key != NULL) {
			if (destination != NULL) {
				end = strchr(end, '\0');
				urlencode(parameter->key, end);
			}
			
			size += urlencode(parameter->key, NULL) - 1;
		}
		
		if (destination != NULL) {
			strcat(destination, EQUAL);
		}
		
		size += strlen(EQUAL);
		
		if (parameter->value != NULL) {
			if (destination != NULL) {
				end = strchr(end, '\0');
				urlencode(parameter->value, end);
			}
			
			size += urlencode(parameter->value, NULL) - 1;
		}
	}
	
	return size;
	
}

void parameter_free(struct HTTPQueryParam* const parameter) {
	
	free(parameter->key);
	parameter->key = NULL;
	
	free(parameter->value);
	parameter->value = NULL;
	
}

void query_free(struct HTTPQuery* const query) {
	
	size_t index = 0;
	
	for (index = 0; index < query->offset; index++) {
		struct HTTPQueryParam* const parameter = &query->parameters[index];
		parameter_free(parameter);
	}
	
	query->size = 0;
	query->offset = 0;
	
	free(query->parameters);
	query->parameters = NULL;
	
}

int query_load_string(
	struct HTTPQuery* const query,
	const char* const string
) {
	
	int err = 0;
	
	strsplit_t split = {0};
	strsplit_part_t part = {0};
	
	strsplit_t subsplit = {0};
	strsplit_part_t subpart = {0};
	
	const char separator[] = {query->sep, '\0'};
	
	struct HTTPQueryParam param = {0};
	
	strsplit_init(&split, string, separator);
	
	while (strsplit_next(&split, &part) != NULL) {
		if (part.size == 0) {
			continue;
		}
		
		strsplit_init(&subsplit, part.begin, EQUAL);
		
		/* Parse parameter name */
		strsplit_next(&subsplit, &subpart);
		
		if (subpart.begin == NULL) {
			continue;
		}
		
		strsplit_resize(&split, &subpart);
		
		if (subpart.size == 0) {
			continue;
		}
		
		param.key = NULL;
		param.value = NULL;
		
		param.key = malloc(subpart.size + 1);
		
		if (param.key == NULL) {
			err = -1;
			break;
		}
		
		memcpy(param.key, subpart.begin, subpart.size);
		param.key[subpart.size] = '\0';
		
		urldecode(param.key, param.key);
		
		/* Parse parameter value */
		strsplit_next(&subsplit, &subpart);
		
		if (subpart.begin == NULL) {
			parameter_free(&param);
			continue;
		}
		
		strsplit_resize(&split, &subpart);
		
		if (subpart.size == 0) {
			parameter_free(&param);
			continue;
		}
		
		param.value = malloc(subpart.size + 1);
		
		if (param.value == NULL) {
			err = -1;
			break;
		}
		
		memcpy(param.value, subpart.begin, subpart.size);
		param.value[subpart.size] = '\0';
		
		urldecode(param.value, param.value);
		
		err = put_parameter(query, &param);
		
		if (err != 0) {
			break;
		}
	}
	
	if (err != 0) {
		parameter_free(&param);
		query_free(query);
	}
	
	return err;
	
}

void query_init(
	struct HTTPQuery* const query,
	const char sep
) {
	
	query_free(query);
	
	query->sep = sep;
	
	if (query->sep == '\0') {
		query->sep = AND;
	}
	
}

int query_load_file(
	struct HTTPQuery* const query,
	const char* const filename
) {
	
	int early_eof = 0;
	int eof = 0;
	int err = 0;
	
	char chunk[8192];
	
	char* destination = chunk;
	size_t dsize = sizeof(chunk);
	
	size_t size = 0;
	ssize_t rsize = 0;
	
	char* end = NULL;
	
	struct FStream* stream = NULL;
	
	stream = fstream_open(filename, FSTREAM_READ);
	
	if (stream == NULL) {
		err = -1;
		goto end;
	}
	
	*chunk = '\0';
	
	while (1) {
		rsize = fstream_read(stream, destination, dsize - 1);
		
		if (rsize == -1) {
			err = -1;
			goto end;
		}
		
		early_eof = (rsize < (dsize - 1));
		eof = (rsize == 0);
		
		if (!eof) {
			rsize += size;
			chunk[rsize] = '\0';
			
			end = chunk + rsize;
			
			while (end != chunk) {
				const unsigned char ch = *end;
				
				if (ch == query->sep) {
					break;
				}
				
				end--;
			}
			
			*end = '\0';
		}
		
		err = query_load_string(query, chunk);
		
		if (err != 0) {
			err = -1;
			goto end;
		}
		
		if (eof) {
			break;
		}
		
		end++;
		
		size = (size_t) ((chunk + rsize) - end);
		
		destination = chunk;
		dsize = sizeof(chunk);
		
		if (size > 0) {
			memmove(destination, end, size);
			destination[size] = '\0';
			
			destination += size;
			dsize -= size;
		}
	}
	
	end:;
	
	if (err != 0) {
		query_free(query);
	}
	
	fstream_close(stream);
	
	return err;
	
}
