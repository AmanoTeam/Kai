#if !defined(FFMPEGC_MUXER_H)
#define FFMPEGC_MUXER_H

int ffmpegc_mux_streams(
	char* const* const sources,
	const char* const destination,
	const int verbose
);

#endif
