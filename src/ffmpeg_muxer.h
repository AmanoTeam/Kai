#if !defined(FFMPEG_MUXER_H)
#define FFMPEG_MUXER_H

const char* ffmpeg_guess_extension(char* const* const sources);

int ffmpeg_mux_streams(
	char* const* const sources,
	const char* const destination,
	const int verbose
);

char* ffmpeg_err2str(const int code);

#endif
