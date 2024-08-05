#!/usr/bin/env python3

import argparse
import os
import io

parser = argparse.ArgumentParser(
	prog = "kai",
	description = "A command-line utility to download contents from M3U8 playlists.",
	allow_abbrev = False,
	add_help = False,
	epilog = "Note, options that take an argument require a equal sign. E.g. --url=URL"
)

parser.add_argument(
	"-h",
	"--help",
	required = False,
	action = "store_true",
	help = "Show this help message and exit."
)

parser.add_argument(
	"-v",
	"--version",
	action = "store_true",
	help = "Display the Kai version and exit."
)

parser.add_argument(
	"-u",
	"--url",
	required = True,
	help = "Specify the URL of M3U8 playlist to work with."
)

parser.add_argument(
	"-k",
	"--insecure",
	required = False,
	action = "store_true",
	help = "Disable SSL/TLS certificate validation."
)

parser.add_argument(
	"-A",
	"--user-agent",
	required = False,
	help = "Send a custom User-Agent header to server."
)

parser.add_argument(
	"-x",
	"--proxy",
	metavar = "URI",
	required = False,
	help = "Proxy all network traffic through the specified proxy. Supports SOCKS(4,5) and also HTTP(S) proxies."
)

parser.add_argument(
	"--doh-url",
	metavar = "URL",
	required = False,
	help = "Forward all DNS requests to a custom DoH (DNS-over-HTTPS) server."
)

parser.add_argument(
	"-e",
	"--referer",
	metavar = "URL",
	required = False,
	help = "Send a custom Referer header to server."
)

parser.add_argument(
	"-r",
	"--retry",
	metavar = "COUNT",
	required = False,
	help = "Specify how many times a failed HTTP request should be retried. Defaults to 0 (no retries)."
)

parser.add_argument(
	"-H",
	"--header",
	metavar = "HEADER",
	required = False,
	help = "Send a custom header to server. This argument can be specified multiple times."
)

parser.add_argument(
	"--disable-cookies",
	required = False,
	action = "store_true",
	help = "Disable HTTP cookie handling."
)

parser.add_argument(
	"--debug",
	required = False,
	action = "store_true",
	help = "Enable verbose logging of network requests for debugging purposes."
)

parser.add_argument(
	"-S",
	"--show-streams",
	required = False,
	action = "store_true",
	help = "List all available streams of the M3U8 playlist."
)

parser.add_argument(
	"--select-media",
	metavar = "MEDIA",
	required = False,
	help = "Select which media stream to download. By default, no additional media streams are downloaded."
)

parser.add_argument(
	"--select-stream",
	metavar = "VARIANT_STREAM",
	required = False,
	help = "Select which variant stream to download. Defaults to the variant stream with the highest bandwidth (bits per second)."
)

parser.add_argument(
	"--disable-autoselection",
	required = False,
	action = "store_true",
	help = "Avoid autoselection of streams based on predefined preferences set by the master playlist."
)

parser.add_argument(
	"--disable-progress-meter",
	required = False,
	action = "store_true",
	help = "Disable showing download progress meter."
)

parser.add_argument(
	"--prefer-ffmpegc",
	required = False,
	action = "store_true",
	help = "Prefer using the FFmpeg CLI tool instead of the builtin implementation when muxing media streams."
)

parser.add_argument(
	"-c",
	"--concurrency",
	required = False,
	help = "Specify how many media segments should be downloaded simultaneously. Defaults to 1."
)

parser.add_argument(
	"-o",
	"--output",
	metavar = "FILENAME",
	required = True,
	help = "Specify the output file."
)

os.environ["LINES"] = "1000"
os.environ["COLUMNS"] = "1000"

file = io.StringIO()
parser.print_help(file = file)
file.seek(0, io.SEEK_SET)

text = file.read()

header = """/*
This file is auto-generated. Use the tool at ../tools/program_help.h.py to regenerate.
*/

#if !defined(PROGRAM_HELP_H)
#define PROGRAM_HELP_H

#define PROGRAM_HELP \\\n\
"""

for line in text.splitlines():
	header += '\t"%s\\n" \\\n' % line

header += "\n#endif\n"

destination = os.path.join(
	os.path.dirname(
		p = os.path.dirname(
			p = os.path.realpath(
				filename = __file__
			)
		)
	),
	"src/program_help.h"
)
	
print("Saving to '%s'" % (destination))

with open(file = destination, mode = "w") as file:
	file.write(header)
