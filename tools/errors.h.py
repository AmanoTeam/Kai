#!/usr/bin/env python3

import argparse
import os
import io

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

for index, error in enumerate(errors):
	error.code = -index if index != 0 else index
	
	if error.group != group:
		group = error.group
		body += "\n"
	
	body += "#define %s %i %s" % (error.name, error.code, error.message)

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
