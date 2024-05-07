#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) && defined(_UNICODE)
	#include <windows.h>
#endif

#include "argparser.h"

void argparser_init(struct ArgumentParser* const argparser, const int argc, argv_t** const argv) {
	
	argparser->index = 1;
	argparser->argc = (size_t) argc;
	argparser->argv = argv;
	
}

const struct Argument* argparser_next(struct ArgumentParser* const argparser) {
	
	const char* astart = NULL;
	const char* aend = NULL;
	
	const char* kstart = NULL;
	const char* kend = NULL;
	
	const char* vstart = NULL;
	const char* vend = NULL;
	
	size_t ksize = 0;
	size_t vsize = 0;
	
	char* item = NULL;
	
	#if defined(_WIN32) && defined(_UNICODE)
		const wchar_t* witem = NULL;
		int items = 0;
	#endif
	
	if (argparser->argument.key != NULL) {
		free(argparser->argument.key);
		argparser->argument.key = NULL;
	}
	
	if (argparser->argument.value != NULL) {
		free(argparser->argument.value);
		argparser->argument.value = NULL;
	}
	
	if (argparser->index >= argparser->argc) {
		return NULL;
	}
	
	#if defined(_WIN32) && defined(_UNICODE)
		witem = argparser->argv[argparser->index++];
		items = WideCharToMultiByte(CP_UTF8, 0, witem, -1, NULL, 0, NULL, NULL);
		
		if (items == 0) {
			return NULL;
		}
		
		item = malloc((size_t) items);
		
		if (item == NULL) {
			return NULL;
		}
		
		if (WideCharToMultiByte(CP_UTF8, 0, witem, -1, item, items, NULL, NULL) == 0) {
			free(item);
			return NULL;
		}
	#else
		item = argparser->argv[argparser->index++];
	#endif
	
	astart = item;
	aend = strchr(item, '\0');
	
	kstart = astart;
	kend = strstr(astart, "=");
	
	if (kend == NULL) {
		kend = aend;
	}
	
	while (*kstart == '-') {
		kstart++;
	}
	
	ksize = (size_t) (kend - kstart);
	
	argparser->argument.key = malloc(ksize + 1);
	
	if (argparser->argument.key == NULL) {
		#if defined(_WIN32) && defined(_UNICODE)
			free(item);
		#endif
		
		return NULL;
	}
	
	memcpy(argparser->argument.key, kstart, ksize);
	argparser->argument.key[ksize] = '\0';
	
	vstart = kend;
	
	if (kend != aend) {
		vstart += 1;
	}
	
	vend = aend;
	
	vsize = (size_t) (vend - vstart);
	
	if (vsize > 0) {
		argparser->argument.value = malloc(vsize + 1);
		
		if (argparser->argument.value == NULL) {
			#if defined(_WIN32) && defined(_UNICODE)
				free(item);
			#endif
			
			return NULL;
		}
		
		memcpy(argparser->argument.value, vstart, vsize);
		argparser->argument.value[vsize] = '\0';
	}
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(item);
	#endif
	
	return &argparser->argument;
	
}

void argparser_free(struct ArgumentParser* const argparser) {
	
	argparser->index = 0;
	argparser->argc = 0;
	argparser->argv = NULL;
	
	free(argparser->argument.key);
	argparser->argument.key = NULL;
	
	free(argparser->argument.value);
	argparser->argument.value = NULL;
	
}