#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) && defined(_UNICODE)
	#include <windows.h>
#endif

#include "argparser.h"
#include "errors.h"

static const struct Argument* argparser_next(struct ArgumentParser* const argparser, struct Argument* const argument) {
	
	const char* astart = NULL;
	const char* aend = NULL;
	
	const char* kstart = NULL;
	const char* kend = NULL;
	
	const char* vstart = NULL;
	const char* vend = NULL;
	
	size_t ksize = 0;
	size_t vsize = 0;
	
	char* item = NULL;
	
	int vonly = 0;
	
	#if defined(_WIN32) && defined(_UNICODE)
		const wchar_t* witem = NULL;
		int items = 0;
	#endif
	
	argument->key = NULL;
	argument->value = NULL;
	
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
	
	argument->type = ARGUMENT_TYPE_NON_VALUE;
	
	while (*kstart == '-') {
		kstart++;
	}
	
	vonly = (kstart == astart);
	
	if (!vonly) {
		ksize = (size_t) (kend - kstart);
		
		argument->key = malloc(ksize + 1);
		
		if (argument->key == NULL) {
			#if defined(_WIN32) && defined(_UNICODE)
				free(item);
			#endif
			
			return NULL;
		}
		
		memcpy(argument->key, kstart, ksize);
		argument->key[ksize] = '\0';
	}
	
	vstart = vonly ? kstart : kend;
	
	if (!vonly && vstart != aend) {
		vstart += 1;
	}
	
	vend = aend;
	
	vsize = (size_t) (vend - vstart);
	
	if (vsize > 0) {
		argument->value = malloc(vsize + 1);
		
		if (argument->value == NULL) {
			#if defined(_WIN32) && defined(_UNICODE)
				free(item);
			#endif
			
			return NULL;
		}
		
		memcpy(argument->value, vstart, vsize);
		argument->value[vsize] = '\0';
		
		argument->type = (vonly) ? ARGUMENT_TYPE_VALUE_ONLY : ARGUMENT_TYPE_VALUE;
	}
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(item);
	#endif
	
	return argument;
	
}

int argparser_init(struct ArgumentParser* const argparser, const int argc, argv_t** const argv) {
	
	int err = M3U8ERR_SUCCESS;
	size_t index = 0;
	
	struct Argument argument = {0};
	
	argparser->index = 1;
	argparser->argc = (size_t) argc;
	argparser->argv = argv;
	
	argparser->arguments.size = sizeof(argument) * argparser->argc;
	argparser->arguments.items = malloc(argparser->arguments.size);
	
	if (argparser->arguments.items == NULL) {
		err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	while (argparser_next(argparser, &argument) != NULL) {
		argparser->arguments.items[argparser->arguments.offset++] = argument;
		
		if (argument.key == NULL && argument.value == NULL) {
			err = M3U8ERR_CLI_ARGUMENT_EMPTY;
			goto end;
		}
	}
	
	for (index = 0; index < argparser->arguments.offset; index++) {
		struct Argument* const prev = &argparser->arguments.items[index - 1];
		struct Argument* const cur = &argparser->arguments.items[index];
		struct Argument* const next = &argparser->arguments.items[index + 1];
		
		if (cur->type != ARGUMENT_TYPE_VALUE_ONLY) {
			continue;
		}
		
		if (index == 0) {
			err = M3U8ERR_CLI_VALUE_UNEXPECTED;
			goto end;
		}
		
		if (prev->type != ARGUMENT_TYPE_NON_VALUE) {
			err = M3U8ERR_CLI_VALUE_UNEXPECTED;
			goto end;
		}
		
		prev->type = ARGUMENT_TYPE_VALUE;
		prev->value = cur->value;
		
		cur->value = NULL;
		
		memmove(cur, next, (argparser->arguments.offset - (index + 1)) * sizeof(argument));
		
		argparser->arguments.offset--;
		index--;
	}
	
	end:;
	
	argparser->index = 0;
	
	if (err != M3U8ERR_SUCCESS) {
		argparser_free(argparser);
	}
	
	return err;
	
}

const struct Argument* argparser_getnext(struct ArgumentParser* const argparser) {
	
	if (argparser->index >= argparser->arguments.offset) {
		return NULL;
	}
	
	return &argparser->arguments.items[argparser->index++];
	
}

static void argument_free(struct Argument* const argument) {
	
	free(argument->key);
	argument->key = NULL;
	
	free(argument->value);
	argument->value = NULL;
	
}

void argparser_free(struct ArgumentParser* const argparser) {
	
	size_t index = 0;
	
	argparser->index = 0;
	argparser->argc = 0;
	argparser->argv = NULL;
	
	for (index = 0; index < argparser->arguments.offset; index++) {
		struct Argument* const argument = &argparser->arguments.items[index];
		argument_free(argument);
	}
	
	argparser->arguments.offset = 0;
	argparser->arguments.size = 0;
	
	free(argparser->arguments.items);
	argparser->arguments.items = NULL;
	
}