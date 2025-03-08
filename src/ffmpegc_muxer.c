#include <stdlib.h>
#include <string.h>

#include "errors.h"
#include "ffmpegc_muxer.h"
#include "os/env.h"
#include "os/shell.h"

static const char FFMPEG_LOGLEVEL_ERROR[] = "error";
static const char FFMPEG_LOGLEVEL_VERBOSE[] = "verbose";

static const char* const FFMPEG_DEFAULT_INPUT_FLAGS[] = {
	"-y",
	"-nostdin",
	"-nostats",
	"-hide_banner",
	"-loglevel", FFMPEG_LOGLEVEL_ERROR
};

static const char* const FFMPEG_DEFAULT_OUTPUT_FLAGS[] = {
	"-c", "copy",
	"-movflags", "+faststart",
	"-max_interleave_delta", "0",
	"-fflags", "+bitexact+autobsf"
};

static const char* const FFMPEG_INPUT_RANGE[] = {
	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13",
	"14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25",
	"26", "27", "28", "29", "30", "31", "32", "33", "34", "35", "36", "37",
	"38", "39", "40", "41", "42", "43", "44", "45", "46", "47", "48", "49",
	"50", "51", "52", "53", "54", "55", "56", "57", "58", "59", "60", "61",
	"62", "63", "64", "65", "66", "67", "68", "69", "70", "71", "72", "73",
	"74", "75", "76", "77", "78", "79", "80", "81", "82", "83", "84", "85",
	"86", "87", "88", "89", "90", "91", "92", "93", "94", "95", "96", "97",
	"98", "99", "100"
};

int ffmpegc_mux_streams(
	char* const* const sources,
	const char* const destination,
	const int verbose
) {
	
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
	argc += index * 6; /* -allowed_extensions <extension> -i <input> -map <position>*/
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
		
		if (strcmp(item, "-loglevel") == 0 && verbose) {
			argv[argvpos++] = (char*) FFMPEG_LOGLEVEL_VERBOSE;
			index++;
		}
	}
	
	for (index = 0; 1; index++) {
		const char* const item = sources[index];
		
		if (item == NULL) {
			break;
		}
		
		argv[argvpos++] = "-allowed_extensions";
		argv[argvpos++] = "bin";
		
		argv[argvpos++] = "-i";
		argv[argvpos++] = (char*) item;
	}
	
	argc = index;
	
	for (index = 0; index < argc; index++) {
		const char* const position = FFMPEG_INPUT_RANGE[index];
		
		argv[argvpos++] = "-map";
		argv[argvpos++] = (char*) position;
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
