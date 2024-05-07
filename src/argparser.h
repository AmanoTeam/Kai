#include <stdlib.h>

#if defined(_WIN32) && defined(_UNICODE)
	#define argv_t wchar_t
#else
	#define argv_t char
#endif

enum ArgumentType {
	ARGUMENT_TYPE_NON_VALUE,
	ARGUMENT_TYPE_VALUE,
	ARGUMENT_TYPE_VALUE_ONLY
};

struct Argument {
	enum ArgumentType type;
	char* key;
	char* value;
};

struct Arguments {
	size_t offset;
	size_t size;
	struct Argument* items;
};

struct ArgumentParser {
	size_t index;
	size_t argc;
	argv_t** argv;
	struct Arguments arguments;
};

int argparser_init(struct ArgumentParser* const argparser, const int argc, argv_t** const argv);
const struct Argument* argparser_getnext(struct ArgumentParser* const argparser);
void argparser_free(struct ArgumentParser* const argparser);

#pragma once
