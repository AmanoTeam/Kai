#if !defined(QUERY_H)
#define QUERY_H

#include "biggestint.h"

struct HTTPQueryParam {
	char* key;
	char* value;
};

struct HTTPQuery {
	size_t size;
	size_t offset;
	struct HTTPQueryParam* parameters;
	char sep;
};

void query_free(struct HTTPQuery* query);

int query_add_string(
	struct HTTPQuery* const query,
	const char* const key,
	const char* const value
);

int query_add_int(
	struct HTTPQuery* const query,
	const char* const key,
	const bigint_t value
);

int query_add_uint(
	struct HTTPQuery* const query,
	const char* const key,
	const biguint_t value
);

int query_add_float(
	struct HTTPQuery* const query,
	const char* const key,
	const bigfloat_t value
);

char* query_get_string(
	struct HTTPQuery* const query,
	const char* const key
);

const char* param_get_string(const struct HTTPQueryParam* const param);

bigint_t query_get_int(
	struct HTTPQuery* const query,
	const char* const key
);

bigint_t param_get_int(const struct HTTPQueryParam* const param);

biguint_t query_get_uint(
	struct HTTPQuery* const query,
	const char* const key
);

biguint_t param_get_uint(const struct HTTPQueryParam* const param);

bigfloat_t query_get_float(
	struct HTTPQuery* const query,
	const char* const key
);

bigfloat_t param_get_float(const struct HTTPQueryParam* const param);

void query_init(
	struct HTTPQuery* const query,
	const char sep
);

int query_load_string(
	struct HTTPQuery* const query,
	const char* const string
);

int query_load_file(
	struct HTTPQuery* const query,
	const char* const filename
);

void parameter_free(struct HTTPQueryParam* const parameter);

size_t query_stringify(
	const struct HTTPQuery* const query,
	char* const destination
);

#endif
