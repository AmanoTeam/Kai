#include <stdlib.h>
#include <string.h>

#include "fstream.h"
#include "query.h"

static char* get_distribution(void) {
	
	int err = 0;
	
	struct HTTPQuery query = {0};
	
	char* ptr = NULL;
	
	char* name = NULL;
	char* value = NULL;
	
	query_init(&query, '\n');
	err = query_load_file(&query, "/etc/os-release");
	
	if (err != 0) {
		err = query_load_file(&query, "/usr/lib/os-release");
	}
	
	if (err != 0) {
		goto end;
	}
	
	value = query_get_string(&query, "NAME");
	
	if (value == NULL || strlen(value) <= 2) {
		err = -1;
		goto end;
	}
	
	ptr = value;
	
	if (*ptr == '"') {
		value++;
	}
	
	ptr = strchr(value, '\0');
	ptr--;
	
	if (*ptr == '"') {
		*ptr = '\0';
	}
	
	name = malloc(strlen(value) + 1);
	
	if (name == NULL) {
		err = -1;
		goto end;
	}
	
	strcpy(name, value);
	
	end:;
	
	query_free(&query);
	
	return name;
	
}

static const char* get_operating_system(void) {
	
	#if defined(_WIN32)
		return "Windows";
	#elif defined(__APPLE__)
		return "Darwin";
	#elif defined(__HAIKU__)
		return "Haiku";
	#elif defined(__ANDROID__)
		return "Android";
	#elif defined(__OpenBSD__)
		return "OpenBSD";
	#elif defined(__MidnightBSD__)
		return "MidnightBSD";
	#elif defined(__FreeBSD__)
		return "FreeBSD";
	#elif defined(__NetBSD__)
		return "NetBSD";
	#elif defined(__DragonFly__)
		return "DragonFly BSD";
	#elif defined(__serenity__)
		return "SerenityOS";
	#elif defined(__linux__)
		return "Linux";
	#else
		return "Unknown";
	#endif
	
}

char* get_platform(void) {
	
	char* value = NULL;
	const char* const operating_system = get_operating_system();
	
	if (strcmp(operating_system, "Linux") == 0 && (value = get_distribution()) != NULL) {
		return value;
	}
	
	value = malloc(strlen(operating_system) + 1);
	
	if (value == NULL) {
		return NULL;
	}
	
	strcpy(value, operating_system);
	
	return value;
	
}
