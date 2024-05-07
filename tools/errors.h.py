#!/usr/bin/env python3

import os
import json

errors_c = """/*
This file is auto-generated. Use the tool at ../tools/errors.h.py to regenerate.
*/

#include "errors.h"

const char* m3u8err_getmessage(const int code) {
	
	switch (code) {
%s
	}
	
	return "Unknown error";
	
}
"""

codes_py = """\
# 
# This file is auto-generated. Use the tool at ../tools/errors.h.py to regenerate.
# 

import enum

class KaiErrorCode(enum.Enum):
%s

"""

exceptions_py = """\
# 
# This file is auto-generated. Use the tool at ../tools/errors.h.py to regenerate.
# 

from .exception import KaiError

%s
"""

mapping_py = """\
# 
# This file is auto-generated. Use the tool at ../tools/errors.h.py to regenerate.
# 

from .exceptions import *

ERROR_CODE_MAPPING = %s

ERROR_MESSAGE_MAPPING = %s
"""

class Error:
	
	def __init__(self, name, group, code, message):
		self.name = name
		self.group = group
		self.code = code
		self.message = message
	
	def __lt__(self, other):
		if other.name == "M3U8ERR_SUCCESS":
			return False
		
		return self.name < other.name

directory = os.path.join(
	os.path.dirname(
		p = os.path.dirname(
			p = os.path.realpath(
				filename = __file__
			)
		)
	),
	"src"
)

beginning = True
prefix = ""
suffix = ""

source = os.path.join(directory, "errors.h")

errors = []

with open(file = source, mode = "r") as file:
	for line in file:
		parts = line.split(maxsplit = 3)
		
		if len(parts) < 4 or not parts[1].startswith("M3U8ERR_"):
			if beginning:
				prefix += line
			else:
				suffix += line
			
			continue
		
		(_, name, code, message) = parts
		
		beginning = False
		
		group = (
			name
				.split(sep = "_", maxsplit = 2)
				.pop(1)
				.lower()
		)
		
		error = Error(
			name = name,
			group = group,
			code = code,
			message = message
		)
		errors.append(error)

errors.sort()

body = ""
group = None

for (index, error) in enumerate(errors):
	error.code = -index if index != 0 else index
	
	if error.group != group:
		group = error.group
		body += "\n"
	
	body += "#define %s %i %s" % (
		error.name,
		error.code,
		error.message
	)
	
	error.message = (
		error.message
			.strip()
			.replace("/*", "")
			.replace("*/", "")
			.strip()
	)

header = (
	prefix.strip() +
	"\n" +
	body +
	"\n" +
	suffix.strip() +
	"\n"
)

destination = source

print("Saving to '%s'" % (destination))

with open(file = destination, mode = "w") as file:
	file.write(header)

source = os.path.join(directory, "errors.c")

# errors.c
body = ""

for error in errors:
	body += "%scase %s:\n%sreturn \"%s\";\n" % (
		"\t" * 2,
		error.name,
		"\t" * 3,
		error.message
	)

header = errors_c % body.rstrip()

destination = source

print("Saving to '%s'" % (destination))

with open(file = destination, mode = "w") as file:
	file.write(header)

destination = os.path.join(directory, "cpython/exceptions/codes.py")

# errors.py
body = ""

for error in errors:
	body += "\t%s = %i\n" % (
		error.name,
		error.code
	)

header = codes_py % body.rstrip()

print("Saving to '%s'" % (destination))

with open(file = destination, mode = "w") as file:
	file.write(header)

destination = os.path.join(directory, "cpython/exceptions/exceptions.py")

# errors.py
body = ""

error_code_mapping = {}
error_message_mapping = {}

for error in errors:
	name = (
		error.name
			.replace("M3U8ERR_", "")
	)
	
	if name == "SUCCESS":
		continue
	
	if name.startswith("ATTRIBUTE_"):
		name = "M3U8AttributeError"
	elif name.startswith("CLI_"):
		name = "CLIError"
	elif name.startswith(("CURLE_", "CURLM_", "CURLSH_", "CURLU_", "CURL_")):
		name = "cURLError"
	elif name.startswith("DOWNLOAD_"):
		name = "DownloadError"
	elif name.startswith("FSTREAM_") or name in ("EXPAND_FILENAME_FAILURE", "GET_APP_FILENAME_FAILURE"):
		name = "FilesystemError"
	elif name.startswith("ITEM_"):
		name = "M3U8ItemError"
	elif name.startswith("MEDIA_"):
		name = "M3U8MediaError"
	elif name.startswith("PARSER_"):
		name = "M3U8ParserError"
	elif name.startswith("PLAYLIST_"):
		name = "M3U8PlaylistError"
	elif name.startswith("TAG_"):
		name = "M3U8TagError"
	elif name.startswith("PRINTF_"):
		name = "PrintfError"
	elif name.startswith("CALLBACK_"):
		name = "CallbackError"
	elif name.startswith("FFMPEG_"):
		name = "FFmpegError"
	elif name == "LOAD_UNSUPPORTED_URI":
		name = "UnsupportedURIError"
	elif name in ("MEMORY_ALLOCATE_FAILURE", "BUFFER_OVERFLOW"):
		name = "MemoryError"
	else:
		raise ValueError("Unknown error code: %s" % (name))
	
	error_message_mapping.update(
		{
			error.code: error.message
		}
	)
	
	if name in error_code_mapping.keys():
		error_code_mapping[name].append(error.code)
		continue
	
	error_code_mapping.update(
		{
			name: [
				error.code
			]
		}
	)
	
	body += "class %s(KaiError):\n\tpass\n\n" % (
		name
	)

header = exceptions_py % (
	body.rstrip()
)

print("Saving to '%s'" % (destination))

with open(file = destination, mode = "w") as file:
	file.write(header)

destination = os.path.join(directory, "cpython/exceptions/mapping.py")

# mapping.py
body = ""

header = mapping_py % (
	(
		json.dumps(
			obj = error_code_mapping,
			indent = 4
		)
			.replace("\"", "")
			.replace("    ", "\t")
	),
	(
		json.dumps(
			obj = error_message_mapping,
			indent = 4
		)
			.replace("    ", "\t")
			.replace("\t\"", "\t")
			.replace("\":", ":")
	)
)

print("Saving to '%s'" % (destination))

with open(file = destination, mode = "w") as file:
	file.write(header)
