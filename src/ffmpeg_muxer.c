#include <stdlib.h>

#include <libavformat/avformat.h>
#include <libavutil/timestamp.h>

#include "ffmpeg_muxer.h"

#define FFMPEG_MAX_STREAMS 101

static int stream_params_copy(AVStream* const destination, const AVStream* const source) {
	
	int code = 0;
	
	destination->id = source->id;
	destination->time_base = source->time_base;
	destination->start_time = source->start_time;
	destination->duration = source->duration;
	destination->nb_frames = source->nb_frames;
	destination->disposition = source->disposition;
	destination->discard = source->discard;
	destination->sample_aspect_ratio = source->sample_aspect_ratio;
	destination->avg_frame_rate = source->avg_frame_rate;
	destination->event_flags = source->event_flags;
	destination->r_frame_rate = source->r_frame_rate;
	destination->pts_wrap_bits = source->pts_wrap_bits;
	
	av_dict_free(&destination->metadata);
	
	code = av_dict_copy(&destination->metadata, source->metadata, 0);
	
	if (code < 0) {
		return code;
	}
	
	code = avcodec_parameters_copy(destination->codecpar, source->codecpar);
	
	if (code < 0) {
		return code;
	}
	
	destination->codecpar->codec_tag = 0;
	
	return code;
	
}

const char* ffmpeg_guess_extension(char* const* const sources) {
	
	const char* extension = NULL;
	const char* source = NULL;
	
	int code = 0;
	
	size_t index = 0;
	
	AVStream* input_stream = NULL;
	
	AVFormatContext* input_format_context = NULL;
	
	AVDictionary* options = NULL;
	
	if (av_log_get_level() != AV_LOG_ERROR) {
		av_log_set_level(AV_LOG_ERROR);
	}
	
	for (index = 0; 1; index++) {
		source = sources[index];
		
		if (source == NULL) {
			break;
		}
	}
	
	if (index > 1) {
		extension = "mkv";
		goto end;
	}
	
	source = sources[0];
	
	code = av_dict_set(&options, "allowed_extensions", "bin", 0);
	
	if (code < 0) {
		goto end;
	}
	
	input_format_context = avformat_alloc_context();
	
	if (input_format_context == NULL) {
		code = AVERROR_UNKNOWN;
		goto end;
	}
	
	code = avformat_open_input(&input_format_context, source, NULL, &options);
	
	av_dict_free(&options);
	options = NULL;
	
	if (code != 0) {
		goto end;
	}
	
	code = avformat_find_stream_info(input_format_context, NULL);
	
	if (code < 0) {
		goto end;
	}
	
	input_stream = input_format_context->streams[0];
	
	switch (input_stream->codecpar->codec_id) {
		case AV_CODEC_ID_H264:
			extension = "ts";
			break;
		case AV_CODEC_ID_AAC:
			extension = "aac";
			break;
		case AV_CODEC_ID_WEBVTT:
			extension = "vtt";
			break;
		case AV_CODEC_ID_MP3:
			extension = "mp3";
			break;
		default:
			break;
	}
	
	end:;
	
	avformat_close_input(&input_format_context);
	
	return extension;
	
}

int ffmpeg_mux_streams(
	char* const* const sources,
	const char* const destination,
	const int verbose
) {
	
	int code = 0;
	int discard = 0;
	
	size_t index = 0;
	size_t subindex = 0;
	
	size_t input_index = 0;
	size_t output_index = 0;
	
	int streams_index[FFMPEG_MAX_STREAMS];
	int stream_index = 0;
	
	AVPacket packet = {0};
	
	AVStream* input_stream = NULL;
	AVStream* output_stream = NULL;
	
	AVFormatContext* input_format_context = NULL;
	AVFormatContext* output_format_context = NULL;
	
	AVDictionary* options = NULL;
	
	AVFormatContext** inputs_format_context = NULL;
	
	const char* source = NULL;
	
	av_log_set_level(verbose ? AV_LOG_VERBOSE : AV_LOG_ERROR);
	
	inputs_format_context = malloc(sizeof(AVFormatContext*) * FFMPEG_MAX_STREAMS);
	
	if (inputs_format_context == NULL) {
		code = AVERROR_UNKNOWN;
		goto end;
	}
	
	for (index = 0; index < FFMPEG_MAX_STREAMS; index++) {
		inputs_format_context[index] = NULL;
	}
	
	code = avformat_alloc_output_context2(&output_format_context, NULL, NULL, destination);
	
	if (code < 0) {
		goto end;
	}
	
	output_format_context->max_interleave_delta = 0;
	/* output_format_context->avoid_negative_ts = AVFMT_AVOID_NEG_TS_MAKE_ZERO; */
	
	output_format_context->flags |= AVFMT_FLAG_AUTO_BSF;
	output_format_context->flags |= AVFMT_FLAG_BITEXACT;
	
	for (index = 0; 1; index++) {
		source = sources[index];
		
		if (source == NULL) {
			break;
		}
		
		code = av_dict_set(&options, "allowed_extensions", "bin", 0);
		
		if (code < 0) {
			goto end;
		}
		
		input_format_context = avformat_alloc_context();
		
		if (input_format_context == NULL) {
			code = AVERROR_UNKNOWN;
			goto end;
		}
		
		code = avformat_open_input(&input_format_context, source, NULL, &options);
		
		av_dict_free(&options);
		options = NULL;
		
		if (code != 0) {
			goto end;
		}
		
		inputs_format_context[index] = input_format_context;
		
		code = avformat_find_stream_info(input_format_context, NULL);
		
		if (code < 0) {
			goto end;
		}
		
		for (subindex = 0; subindex < input_format_context->nb_streams; subindex++) {
			input_stream = input_format_context->streams[subindex];
			output_index = subindex + index;
			
			if (input_stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && input_stream->codecpar->sample_rate < 1) {
				streams_index[output_index] = -1;
				continue;
			}
			
			if (!(input_stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO || input_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO || input_stream->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE)) {
				streams_index[output_index] = -1;
				continue;
			}
			
			streams_index[output_index] = stream_index++;
			
			output_stream = avformat_new_stream(output_format_context, NULL);
			
			if (output_stream == NULL) {
				code = AVERROR_UNKNOWN;
				goto end;
			}
			
			code = stream_params_copy(output_stream, input_stream);
			
			if (code < 0) {
				goto end;
			}
		}
	}
	
	if (!(output_format_context->oformat->flags & AVFMT_NOFILE)) {
		code = avio_open(&output_format_context->pb, destination, AVIO_FLAG_WRITE);
		
		if (code < 0) {
			goto end;
		}
	}
	
	code = avformat_write_header(output_format_context, NULL);
	
	if (code < 0) {
		goto end;
	}
	
	for (index = 0; 1; index++) {
		source = sources[index];
		
		if (source == NULL) {
			break;
		}
		
		input_format_context = inputs_format_context[index];
		
		while (1) {
			code = av_read_frame(input_format_context, &packet);
			
			if (code == AVERROR_EOF) {
				break;
			}
			
			if (code < 0) {
				goto end;
			}
			
			input_index = packet.stream_index;
			output_index = input_index + index;
			
			discard = (packet.pts == AV_NOPTS_VALUE || output_index >= FFMPEG_MAX_STREAMS || streams_index[output_index] == -1);
			
			if (discard) {
				av_packet_unref(&packet);
				continue;
			}
			
			input_stream = input_format_context->streams[input_index];
			output_stream = output_format_context->streams[output_index];
			
			packet.pts = av_rescale_q_rnd(packet.pts, input_stream->time_base, output_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
			packet.dts = av_rescale_q_rnd(packet.dts, input_stream->time_base, output_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
			packet.duration = av_rescale_q(packet.duration, input_stream->time_base, output_stream->time_base);
			packet.pos = -1;
			
			packet.stream_index = output_index;
			
			code = av_interleaved_write_frame(output_format_context, &packet);
			
			av_packet_unref(&packet);
			
			if (code != 0) {
				goto end;
			}
		}
	}
	
	code = av_write_trailer(output_format_context);
	
	if (code != 0) {
		goto end;
	}
	
	end:;
	
	if (inputs_format_context != NULL) {
		for (index = 0; index < FFMPEG_MAX_STREAMS; index++) {
			AVFormatContext** context = &inputs_format_context[index];
			
			if (context == NULL) {
				break;
			}
			
			avformat_close_input(context);
		}
		
		free(inputs_format_context);
	}
	
	if (output_format_context != NULL) {
		if (!(output_format_context->oformat->flags & AVFMT_NOFILE)) {
			avio_closep(&output_format_context->pb);
		}
		
		avformat_free_context(output_format_context);
	}
	
	return code;
	
}

char* ffmpeg_err2str(const int code) {
	
	int status = 0;
	char* message = malloc(AV_ERROR_MAX_STRING_SIZE);
	
	if (message == NULL) {
		return NULL;
	}
	
	status = av_strerror(code, message, AV_ERROR_MAX_STRING_SIZE);
	
	if (status != 0) {
		strcpy(message, "unknown cause");
	}
	
	return message;
	
}
