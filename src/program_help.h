/*
This file is auto-generated. Use the tool at ../tools/program_help.h.py to regenerate.
*/

#if !defined(PROGRAM_HELP_H)
#define PROGRAM_HELP_H

#define PROGRAM_HELP \
	"usage: kai [-h] [-v] -u URL [-k] [-A USER_AGENT] [-x URI] [--doh-url URL] [-e URL] [-r COUNT] [-H HEADER] [--disable-cookies] [--http1.0] [--http1.1] [--http2] [--debug] [-S] [--select-media MEDIA] [--select-stream VARIANT_STREAM] [--disable-autoselection] [--disable-progress-meter] [--prefer-ffmpegc] [-c CONCURRENCY] -o FILENAME [--return-error-code]\n" \
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
	"  -x URI, --proxy URI   Proxy all network traffic through the specified proxy. Supports SOCKS(4,5) and also HTTP(S) proxies.\n" \
	"  --doh-url URL         Forward all DNS requests to a custom DoH (DNS-over-HTTPS) server.\n" \
	"  -e URL, --referer URL\n" \
	"                        Send a custom Referer header to server.\n" \
	"  -r COUNT, --retry COUNT\n" \
	"                        Specify how many times a failed HTTP request should be retried. Defaults to 0 (no retries).\n" \
	"  -H HEADER, --header HEADER\n" \
	"                        Send a custom header to server. This argument can be specified multiple times.\n" \
	"  --disable-cookies     Disable HTTP cookie handling.\n" \
	"  --http1.0             Enforce HTTP/1.0 for network requests.\n" \
	"  --http1.1             Enforce HTTP/1.1 for network requests.\n" \
	"  --http2               Enforce HTTP/2 for network requests.\n" \
	"  --debug               Enable verbose logging of network requests for debugging purposes.\n" \
	"  -S, --show-streams    List all available streams of the M3U8 playlist.\n" \
	"  --select-media MEDIA  Select which media stream to download. By default, no additional media streams are downloaded.\n" \
	"  --select-stream VARIANT_STREAM\n" \
	"                        Select which variant stream to download. Defaults to the variant stream with the highest bandwidth (bits per second).\n" \
	"  --disable-autoselection\n" \
	"                        Avoid autoselection of streams based on predefined preferences set by the master playlist.\n" \
	"  --disable-progress-meter\n" \
	"                        Disable showing download progress meter.\n" \
	"  --prefer-ffmpegc      Prefer using the FFmpeg CLI tool instead of the builtin implementation when muxing media streams.\n" \
	"  -c CONCURRENCY, --concurrency CONCURRENCY\n" \
	"                        Specify how many media segments should be downloaded simultaneously. Defaults to 1.\n" \
	"  -o FILENAME, --output FILENAME\n" \
	"                        Specify the output file.\n" \
	"  --return-error-code   In case of errors, prefer returning Kai's internal error codes instead of the standard platform error codes.\n" \
	"\n" \
	"Note, options that take an argument require a equal sign. E.g. --url=URL\n" \

#endif
