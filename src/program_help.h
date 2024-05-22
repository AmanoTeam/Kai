/*
This file is auto-generated. Use the ../tools/program_help.h.py tool to regenerate.
*/

#if !defined(PROGRAM_HELP_H)
#define PROGRAM_HELP_H

#define PROGRAM_HELP \
	"usage: kai [-h] [-v] -u URL [-k] [-A USER_AGENT] [-x URI] [--doh-url URL] [-e URL] [-S] [--select-media MEDIA] [--select-stream VARIANT_STREAM] [-c CONCURRENCY] -o FILENAME [--debug] [-r COUNT]\n" \
	"\n" \
	"A command-line utility to download contents from M3U8 playlists.\n" \
	"\n" \
	"options:\n" \
	"  -h, --help            Show this help message and exit.\n" \
	"  -v, --version         Display the Kai version and exit.\n" \
	"  -u URL, --url URL     Specify the URL of M3U8 playlist to work with.\n" \
	"  -k, --insecure        Disable SSL/TLS certificate validation.\n" \
	"  -A USER_AGENT, --user-agent USER_AGENT\n" \
	"                        Send a custom User-Agent header to server.\n" \
	"  -x URI, --proxy URI   Proxy all network traffic through the specified proxy.\n" \
	"  --doh-url URL         Forward all DNS requests to a custom DoH (DNS-over-HTTPS) server.\n" \
	"  -e URL, --referer URL\n" \
	"                        Send a custom Referer header to server.\n" \
	"  -S, --list-streams    List all available streams of the M3U8 playlist.\n" \
	"  --select-media MEDIA  Select which media stream to download. By default, no additional media streams are downloaded.\n" \
	"  --select-stream VARIANT_STREAM\n" \
	"                        Select which variant stream to download. Defaults to the variant stream with the highest bandwidth (bits per second).\n" \
	"  -c CONCURRENCY, --concurrency CONCURRENCY\n" \
	"                        Specify how many media segments should be downloaded simultaneously. Defaults to 1.\n" \
	"  -o FILENAME, --output FILENAME\n" \
	"                        Specify the output file.\n" \
	"  --debug               Enable verbose logging of network requests for debugging purposes.\n" \
	"  -r COUNT, --retry COUNT\n" \
	"                        Specify how many times a failed HTTP request should be retried. Defaults to 0 (no retries at all).\n" \
	"\n" \
	"Note, options that take an argument require a equal sign. E.g. --url=URL\n" \

#endif
