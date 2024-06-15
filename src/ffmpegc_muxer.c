#include <stdlib.h>

#include "os.h"
#include "m3u8errors.h"
#include "ffmpegc_muxer.h"

static const char* const FFMPEG_DEFAULT_INPUT_FLAGS[] = {
	"-y", "-nostdin", "-nostats", "-loglevel", "error"
};

static const char* const FFMPEG_DEFAULT_OUTPUT_FLAGS[] = {
	"-map", "a",
	"-c", "copy",
	"-movflags", "+faststart",
	"-max_interleave_delta", "0",
	"-fflags", "+bitexact+autobsf"
};

int ffmpegc_mux_streams(char* const* const sources, const char* const destination) {
	
	int err = M3U8ERR_SUCCESS;
	
	char* executable = NULL;
	char* command = NULL;
	char**argv = NULL;
	
	size_t index = 0;
	size_t argvpos = 0;
	size_t size = 0;
	size_t argc = 0;
	
	argc += sizeof(FFMPEG_DEFAULT_INPUT_FLAGS) / sizeof(*FFMPEG_DEFAULT_INPUT_FLAGS);
	argc += sizeof(FFMPEG_DEFAULT_OUTPUT_FLAGS) / sizeof(*FFMPEG_DEFAULT_OUTPUT_FLAGS);
	
	for (index = 0; 1; index++) {
		const char* const item = sources[index];
		
		if (item == NULL) {
			break;
		}
	}
	
	argc += 1; /* <command> */
	argc += index * 2; /* -i <input> */
	argc += 1; /* <output> */
	
	executable = find_exe("ffmpeg");
	
	if (executable == NULL) {
		err = M3U8ERR_FFMPEG_COMMAND_NOT_FOUND;
		goto end;
	}
	
	argv = malloc(sizeof(*argv) * argc);
	
	if (argv == NULL) {
		err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	argv[argvpos++] = executable;
	
	for (index = 0; index < sizeof(FFMPEG_DEFAULT_INPUT_FLAGS) / sizeof(*FFMPEG_DEFAULT_INPUT_FLAGS); index++) {
		const char* const item = FFMPEG_DEFAULT_INPUT_FLAGS[index];
		argv[argvpos++] = (char*) item;
	}
	
	for (index = 0; 1; index++) {
		const char* const item = sources[index];
		
		if (item == NULL) {
			break;
		}
		
		argv[argvpos++] = "-i";
		argv[argvpos++] = (char*) item;
	}
	
	for (index = 0; index < sizeof(FFMPEG_DEFAULT_OUTPUT_FLAGS) / sizeof(*FFMPEG_DEFAULT_OUTPUT_FLAGS); index++) {
		const char* const item = FFMPEG_DEFAULT_OUTPUT_FLAGS[index];
		argv[argvpos++] = (char*) item;
	}
	
	argv[argvpos++] = (char*) destination;
	
	for (index = 0; index < argvpos; index++) {
		const char* const arg = argv[index];
		size += strlen(arg) + 1;
	}
	
	size += 1; /* \0 */
	
	command = malloc(size);
	
	if (command == NULL) {
		err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	command[0] = '\0';
	
	for (index = 0; index < argvpos; index++) {
		const char* const arg = argv[index];
		strcat(command, arg);
		strcat(command, " ");
	}
	puts(command);
	
	if (execute_shell_command(command) != 0) {
		err = M3U8ERR_FFMPEG_MUXING_FAILURE;
		goto end;
	}
	
	end:;
	
	free(executable);
	free(argv);
	free(command);
	
	return err;
	
}
