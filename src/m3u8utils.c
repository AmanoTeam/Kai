#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#include <curl/curl.h>

#include "m3u8utils.h"
#include "m3u8types.h"
#include "errors.h"
#include "path.h"
#include "pathsep.h"
#include "biggestint.h"

int namesafe(const char* const s) {
	
	size_t index = 0;
	
	for (index = 0; index < strlen(s); index++) {
		const unsigned char ch = s[index];
		
		if (!(isdigit(ch) || isupper(ch) || ch == '-')) {
			return 0;
		}
	}
	
	return 1;
	
}


int isuint(const char* const s) {
	/*
	Checks whether the string pointed to by 's' is a
	decimal integer.
	*/
	
	const char* st = s;
	const char* ed = strchr(st, '\0');
	
	if (st[0] == '-' || (strlen(st) > 1 && st[0] == '0')) {
		return 0;
	}
	
	while (st != ed) {
		const unsigned char ch = *st;
		
		if (!isdigit(ch)) {
			return 0;
		}
		
		st++;
	}
	
	return 1;
	
}

int isfloat(const char* const s) {
	/*
	Checks whether the string pointed to by 's' is a
	signed decimal floating point number.
	*/
	
	const char* st = s;
	const char* ed = strchr(st, '\0');
	unsigned char ch = 0;
	
	int dot = 0;
	
	if (*st == '-') {
		st++;
	}
	
	ch = *st;
	
	if (st == ed || !isdigit(ch)) {
		return 0;
	}
	
	st++;
	
	while (st != ed) {
		ch = *st;
		
		if ((st + 1) != ed && ch == '.') {
			/* There could not be multiple dots. */
			if (dot) {
				return 0;
			}
			
			dot = 1;
		} else if (!isdigit(ch)) {
			return 0;
		}
		
		st++;
	}
	
	return 1;
	
}

int isufloat(const char* const s) {
	/*
	Checks whether the string pointed to by 's' is a
	decimal floating point number.
	*/
	
	const int status = isfloat(s);
	
	if (!status) {
		return status;
	}
	
	if (*s == '-') {
		return 0;
	}
	
	return 1;
	
}

int ishex(const char* const s) {
	/*
	Checks whether the string pointed to by 's' is a
	hexadecimal sequence.
	*/
	
	const char* st = s;
	const char* ed = strchr(st, '\0');
	size_t size = (size_t) (ed - st);
	
	if (!(size >= 2 && st[0] == '0' && (st[1] == 'x' || st[1] == 'X'))) {
		return 0;
	}
	
	st += 2;
	
	size = (size_t) (ed - st);
	
	if (size == 0) {
		return 0;
	}
	
	while (st != ed) {
		const unsigned char ch = *st;
		
		if (!isalnum(ch)) {
			return 0;
		}
		
		st++;
	}
	
	return 1;
	
}

int isqstring(const char* const s) {
	/*
	Checks whether the string pointed to by 's' is a
	quoted string.
	*/
	
	const char* st = s;
	const char* ed = strchr(st, '\0');
	
	if (*st != '"') {
		return 0;
	}
	
	st++;
	
	if (st == ed) {
		return 0;
	}
	
	if ((ed - 1) == st || *(ed - 1) != '"') {
		return 0;
	}
	
	ed--;
	
	while (st != ed) {
		const char ch = *st;
		
		if (ch == '\r' || ch == '\n' || ch == '"') {
			return 0;
		}
		
		st++;
	}
	
	return 1;
	
}

int isestring(const char* const s) {
	/*
	Checks whether the string pointed to by 's' is an
	enumerated string.
	*/
	
	const char* st = s;
	const char* ed = strchr(st, '\0');
	
	while (st != ed) {
		const unsigned char ch = *st;
		
		if (ch == '"' || ch == ',' || isspace(ch)) {
			return 0;
		}
		
		st++;
	}
	
	return 1;
	
}

int isresolution(const char* const s) {
	/*
	Checks whether the string pointed to by 's' is a
	decimal resolution.
	*/
	
	const char* st = s;
	const char* ed = strchr(st, '\0');
	
	size_t index = 0;
	size_t size = (size_t) (ed - st);
	
	if (st == ed) {
		return 0;
	}
	
	for (index = 0; index < (size < 4 ? size : 4); index++) {
		const unsigned char ch = st[index];
		
		if (ch == 'x') {
			break;
		}
		
		if ((index == 0 && ch == '0') || !isdigit(ch)) {
			return 0;
		}
	}
	
	if (index <= 2 || (index + 1) > size) {
		return 0;
	}
	
	st += index;
	
	if (*st != 'x') {
		return 0;
	}
	
	st++;
	size -= index;
	
	if (size <= 2) {
		return 0;
	}
	
	for (index = 0; index < size; index++) {
		const unsigned char ch = st[index];
		
		if (ch == '\0') {
			break;
		}
		
		if ((index == 0 && ch == '0') || !isdigit(ch)) {
			return 0;
		}
	}
	
	if (index <= 2) {
		return 0;
	}
	
	return 1;
	
}

int isbrange(const char* const s) {
	/*
	Checks whether the string pointed to by 's' is a
	byte range.
	*/
	
	const char* st = s;
	const char* ed = strchr(st, '\0');
	
	size_t index = 0;
	size_t size = (size_t) (ed - st);
	
	if (st == ed) {
		return 0;
	}
	
	for (index = 0; index < size; index++) {
		const unsigned char ch = st[index];
		
		if (ch == '@') {
			break;
		}
		
		if ((index == 0 && ch == '0') || !isdigit(ch)) {
			return 0;
		}
	}
	
	if (index < 1) {
		return 0;
	}
	
	st += index;
	
	/*
	The byte offset is optional; its absence does not imply
	an error.
	*/
	if (*st != '@') {
		return 1;
	}
	
	st++;
	size -= index;
	
	if (size < 1) {
		return 0;
	}
	
	for (index = 0; index < size; index++) {
		const unsigned char ch = st[index];
		
		if (ch == '\0') {
			break;
		}
		
		if ((index == 1 && st[0] == '0') || !isdigit(ch)) {
			return 0;
		}
	}
	
	return 1;
	
}

int isdtime(const char* const s) {
	/*
	Checks whether the string pointed to by 's' is an
	ISO/IEC 8601:2004 date/time representation
	*/
	
	const char* st = s;
	const char* ed = strchr(st, '\0');
	
	size_t index = 0;
	size_t size = (size_t) (ed - st);
	
	int year = 0;
	int month = 0;
	int day = 0;
	
	int maxdays = 0;
	
	const int month_days[] = {
		0, 31, 28, 31, 30, 31,
		30, 31, 31, 30, 31, 30, 31
	};
	
	if (st == ed) {
		return 0;
	}
	
	/* Validate the year number */
	for (index = 0; index < (size < 4 ? size : 4); index++) {
		const unsigned char ch = st[index];
		
		const int status = (
			!isdigit(ch) ||
			(index == 0 && (ch < '1' || ch > '2'))
		);
		
		if (status) {
			return 0;
		}
	}
	
	if (index < 4) {
		return 0;
	}
	
	st += index;
	
	if (*st != '-') {
		return 0;
	}
	
	year = atoi(st - index);
	
	st++;
	size -= index;
	
	/* Validate the month number */
	for (index = 0; index < (size < 2 ? size : 2); index++) {
		const unsigned char ch = st[index];
		
		const int status = (
			!isdigit(ch) ||
			(index == 0 && ch > '1') ||
			(index == 1 && ((ch < '1' && st[0] < '1') || (ch > '2' && st[0] == '1')))
		);
		
		if (status) {
			return 0;
		}
	}
	
	if (index < 2) {
		return 0;
	}
	
	st += index;
	
	if (*st != '-') {
		return 0;
	}
	
	month = atoi(st - index);
	
	st++;
	size -= index;
	
	/* Validate the day number */
	for (index = 0; index < (size < 2 ? size : 2); index++) {
		const unsigned char ch = st[index];
		
		const int status = (
			!isdigit(ch) ||
			(index == 0 && ch > '3') ||
			(index == 1 && ((ch < '1' && st[0] < '1') || (ch > '1' && st[0] == '3')))
		);
		
		if (status) {
			return 0;
		}
	}
	
	if (index < 2) {
		return 0;
	}
	
	st += index;
	
	if (*st != 'T') {
		return 0;
	}
	
	day = atoi(st - index);
	
	maxdays = month_days[month] + (month == 2 && isleap(year));
	
	if (day > maxdays) {
		return 0;
	}
	
	st++;
	size -= index;
	
	/* Validate the hour number */
	for (index = 0; index < (size < 2 ? size : 2); index++) {
		const unsigned char ch = st[index];
		
		const int status = (
			!isdigit(ch) ||
			(index == 0 && ch > '2') ||
			(index == 1 && (ch > '3' && st[0] == '2'))
		);
		
		if (status) {
			return 0;
		}
	}
	
	if (index < 2) {
		return 0;
	}
	
	st += index;
	
	if (*st != ':') {
		return 0;
	}
	
	st++;
	size -= index;
	
	/* Validate the minutes number */
	for (index = 0; index < (size < 2 ? size : 2); index++) {
		const unsigned char ch = st[index];
		
		const int status = (!isdigit(ch) || (index == 0 && ch > '5'));
		
		if (status) {
			return 0;
		}
	}
	
	if (index < 2) {
		return 0;
	}
	
	st += index;
	
	if (*st != ':') {
		return 0;
	}
	
	st++;
	size -= index;
	
	/* Validate the seconds number */
	for (index = 0; index < (size < 2 ? size : 2); index++) {
		const unsigned char ch = st[index];
		
		const int status = (!isdigit(ch) || (index == 0 && ch > '5'));
		
		if (status) {
			return 0;
		}
	}
	
	if (index < 2) {
		return 0;
	}
	
	st += index;
	
	/*
	The presence of fractional parts of seconds (aka milliseconds)
	is not a strict requirement.
	*/
	if (*st == '.') {
		st++;
		size -= index;
		
		for (index = 0; index < (size < 3 ? size : 3); index++) {
			const unsigned char ch = st[index];
			
			if (!isdigit(ch)) {
				if (index == 0) {
					return 0;
				}
				
				break;
			}
		}
		
		if (index < 1) {
			return 0;
		}
		
		st += index;
	}
	
	/*
	The presence of a time zone is not a strict requirement.
	*/
	if (*st == '+' || *st == '-') {
		st++;
		size -= index;
		
		/* Validate the time zone hour number */
		for (index = 0; index < (size < 2 ? size : 2); index++) {
			const unsigned char ch = st[index];
			
			const int status = (
				!isdigit(ch) ||
				(index == 0 && ch > '2') ||
				(index == 1 && (ch > '3' && st[0] == '2'))
			);
			
			if (status) {
				return 0;
			}
		}
		
		if (index < 2) {
			return 0;
		}
		
		st += index;
		
		if (*st != ':') {
			return 0;
		}
		
		st++;
		size -= index;
		
		/* Validate the time zone minutes number */
		for (index = 0; index < (size < 2 ? size : 2); index++) {
			const unsigned char ch = st[index];
			
			const int status = (!isdigit(ch) || (index == 0 && ch > '5'));
			
			if (status) {
				return 0;
			}
		}
		
		if (index < 2) {
			return 0;
		}
		
		st += index;
	}
	
	if (*st == 'Z') {
		st += 1;
	}
	
	if (*st != '\0') {
		return 0;
	}
	
	return 1;
	
}

char* btos(const bigfloat_t b, char* const s) {
	
	size_t index = 0;
	bigfloat_t value = b;
	
	const char* const units[] = {
		"", "K", "M", "G",
		"T", "P", "E", "Z"
	};
	
	for (index = 0; index < sizeof(units) / sizeof(*units); index++) {
		const char* const unit = units[index];
		const bigfloat_t absolute = fabsl(value);
		
		if (absolute < 1024.0) {
			if (snprintf(s, BTOS_MAX_SIZE, "%3.2"FORMAT_BIGGEST_FLOAT_T" %sB", value, unit) == -1) {
				return NULL;
			}
				
			return s;
		}
		
		value /= 1024.0;
		
	}
	
	if (snprintf(s, BTOS_MAX_SIZE, "%.2"FORMAT_BIGGEST_FLOAT_T" YB", value) == -1) {
		return NULL;
	}
	
	return s;
	
}

int isleap(const int year) {
	
	const int status = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
	return status;
	
}

int m3u8uri_resolve_url(const char* const a, const char* const b, char** destination) {
	
	int err = M3U8ERR_SUCCESS;
	
	CURLU* cu = curl_url();
	
	if (cu == NULL) {
		err = M3U8ERR_CURLU_INIT_FAILURE;
		goto end;
	}
	
	if (curl_url_set(cu, CURLUPART_URL, a, 0) != CURLUE_OK) {
		err = M3U8ERR_CURLU_URL_SET_FAILURE;
		goto end;
	}
	
	if (curl_url_set(cu, CURLUPART_URL, b, 0) != CURLUE_OK) {
		err = M3U8ERR_CURLU_URL_SET_FAILURE;
		goto end;
	}
	
	if (curl_url_get(cu, CURLUPART_URL, destination, 0) != CURLUE_OK) {
		err = M3U8ERR_CURLU_URL_GET_FAILURE;
		goto end;
	}
	
	end:;
	
	curl_url_cleanup(cu);
	
	return err;
	
}

int m3u8uri_resolve_path(const char* const a, const char* const b, char** destination) {
	
	char path[PATH_MAX];
	
	if (isabsolute(b)) {
		strcpy(path, b);
	} else {
		const char* const name = basename(a);
		const size_t size = ((size_t) (name - a)) - 1;
		
		path[0] = '\0';
		
		if ((size + strlen(PATHSEP) + strlen(b) + 1) > sizeof(path)) {
			return M3U8ERR_BUFFER_OVERFLOW;
		}
		
		memcpy(path, a, size);
		path[size] = '\0';
		
		strcat(path, PATHSEP);
		strcat(path, b);
	}
	
	*destination = malloc(strlen(path) + 1);
	
	if (*destination == NULL) {
		return M3U8ERR_MEMORY_ALLOCATE_FAILURE;
	}
	
	strcpy(*destination, path);
	
	return M3U8ERR_SUCCESS;
	
}

int m3u8uri_resolve_file(const char* const a, const char* const b, char** destination) {
	
	return m3u8uri_resolve_path(a, b, destination);
	
}

int m3u8uri_resolve_directory(const char* const a, const char* const b, char** destination) {
	
	int status = M3U8ERR_SUCCESS;
	char* directory = NULL;
	
	const int sep = *(strchr(a, '\0') - 1) == *PATHSEP;
	
	if (sep) {
		return m3u8uri_resolve_path(a, b, destination);
	}
	
	directory = malloc(strlen(a) + strlen(PATHSEP) + 1);
	
	if (directory == NULL) {
		return M3U8ERR_MEMORY_ALLOCATE_FAILURE;
	}
	
	strcpy(directory, a);
	strcat(directory, PATHSEP);
	
	status = m3u8uri_resolve_path(directory, b, destination);
	
	free(directory);
	
	return status;
	
}

int m3u8uri_resolve(
	const struct M3U8BaseURI* const base,
	const char* const source,
	char** destination
) {
	
	int err = M3U8ERR_SUCCESS;
	
	switch (base->type) {
		case M3U8_BASE_URI_TYPE_URL:
			err = m3u8uri_resolve_url(base->uri, source, destination);
			break;
		case M3U8_BASE_URI_TYPE_LOCAL_FILE:
			err = m3u8uri_resolve_file(base->uri, source, destination);
			break;
		case M3U8_BASE_URI_TYPE_LOCAL_DIRECTORY:
			err = m3u8uri_resolve_directory(base->uri, source, destination);
			break;
		default:
			err = M3U8ERR_LOAD_UNSUPPORTED_URI;
			break;
	}
	
	return err;
	
}