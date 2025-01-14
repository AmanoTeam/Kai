#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "m3u8.h"
#include "m3u8stream.h"
#include "errors.h"
#include "m3u8parser.h"
#include "m3u8sizeof.h"
#include "biggestint.h"
#include "m3u8utils.h"

#define M3U8_MAX_KFV_VALUE_LEN (8 + 1)

static const char M3U8K_CC1[] = "CC1";
static const char M3U8K_CC2[] = "CC2";
static const char M3U8K_CC3[] = "CC3";
static const char M3U8K_CC4[] = "CC4";
static const char M3U8K_SERVICE1[] = "SERVICE1";
static const char M3U8K_SERVICE2[] = "SERVICE2";
static const char M3U8K_SERVICE3[] = "SERVICE3";
static const char M3U8K_SERVICE4[] = "SERVICE4";
static const char M3U8K_SERVICE5[] = "SERVICE5";
static const char M3U8K_SERVICE6[] = "SERVICE6";
static const char M3U8K_SERVICE7[] = "SERVICE7";
static const char M3U8K_SERVICE8[] = "SERVICE8";
static const char M3U8K_SERVICE9[] = "SERVICE9";
static const char M3U8K_SERVICE10[] = "SERVICE10";
static const char M3U8K_SERVICE11[] = "SERVICE11";
static const char M3U8K_SERVICE12[] = "SERVICE12";
static const char M3U8K_SERVICE13[] = "SERVICE13";
static const char M3U8K_SERVICE14[] = "SERVICE14";
static const char M3U8K_SERVICE15[] = "SERVICE15";
static const char M3U8K_SERVICE16[] = "SERVICE16";
static const char M3U8K_SERVICE17[] = "SERVICE17";
static const char M3U8K_SERVICE18[] = "SERVICE18";
static const char M3U8K_SERVICE19[] = "SERVICE19";
static const char M3U8K_SERVICE20[] = "SERVICE20";
static const char M3U8K_SERVICE21[] = "SERVICE21";
static const char M3U8K_SERVICE22[] = "SERVICE22";
static const char M3U8K_SERVICE23[] = "SERVICE23";
static const char M3U8K_SERVICE24[] = "SERVICE24";
static const char M3U8K_SERVICE25[] = "SERVICE25";
static const char M3U8K_SERVICE26[] = "SERVICE26";
static const char M3U8K_SERVICE27[] = "SERVICE27";
static const char M3U8K_SERVICE28[] = "SERVICE28";
static const char M3U8K_SERVICE29[] = "SERVICE29";
static const char M3U8K_SERVICE30[] = "SERVICE30";
static const char M3U8K_SERVICE31[] = "SERVICE31";
static const char M3U8K_SERVICE32[] = "SERVICE32";
static const char M3U8K_SERVICE33[] = "SERVICE33";
static const char M3U8K_SERVICE34[] = "SERVICE34";
static const char M3U8K_SERVICE35[] = "SERVICE35";
static const char M3U8K_SERVICE36[] = "SERVICE36";
static const char M3U8K_SERVICE37[] = "SERVICE37";
static const char M3U8K_SERVICE38[] = "SERVICE38";
static const char M3U8K_SERVICE39[] = "SERVICE39";
static const char M3U8K_SERVICE40[] = "SERVICE40";
static const char M3U8K_SERVICE41[] = "SERVICE41";
static const char M3U8K_SERVICE42[] = "SERVICE42";
static const char M3U8K_SERVICE43[] = "SERVICE43";
static const char M3U8K_SERVICE44[] = "SERVICE44";
static const char M3U8K_SERVICE45[] = "SERVICE45";
static const char M3U8K_SERVICE46[] = "SERVICE46";
static const char M3U8K_SERVICE47[] = "SERVICE47";
static const char M3U8K_SERVICE48[] = "SERVICE48";
static const char M3U8K_SERVICE49[] = "SERVICE49";
static const char M3U8K_SERVICE50[] = "SERVICE50";
static const char M3U8K_SERVICE51[] = "SERVICE51";
static const char M3U8K_SERVICE52[] = "SERVICE52";
static const char M3U8K_SERVICE53[] = "SERVICE53";
static const char M3U8K_SERVICE54[] = "SERVICE54";
static const char M3U8K_SERVICE55[] = "SERVICE55";
static const char M3U8K_SERVICE56[] = "SERVICE56";
static const char M3U8K_SERVICE57[] = "SERVICE57";
static const char M3U8K_SERVICE58[] = "SERVICE58";
static const char M3U8K_SERVICE59[] = "SERVICE59";
static const char M3U8K_SERVICE60[] = "SERVICE60";
static const char M3U8K_SERVICE61[] = "SERVICE61";
static const char M3U8K_SERVICE62[] = "SERVICE62";
static const char M3U8K_SERVICE63[] = "SERVICE63";

static const char M3U8K_AUDIO[] = "AUDIO";
static const char M3U8K_VIDEO[] = "VIDEO";
static const char M3U8K_SUBTITLES[] = "SUBTITLES";
static const char M3U8K_CLOSED_CAPTIONS[] = "CLOSED-CAPTIONS";

static const char M3U8K_TYPE_0[] = "TYPE-0";
static const char M3U8K_NONE[] = "NONE";

static const char M3U8K_YES[] = "YES";
static const char M3U8K_NO[] = "NO";

static const char M3U8K_UNKNOWN[] = "UNKNOWN";
static const char M3U8K_AES_128[] = "AES-128";
static const char M3U8K_SAMPLE_AES[] = "SAMPLE-AES";

static const char M3U8K_EVENT[] = "EVENT";
static const char M3U8K_VOD[] = "VOD";

static int m3u8stream_getbool(const void* const value) {
	
	if (strcmp(value, M3U8K_YES) == 0) {
		return 1;
	}
	
	if (strcmp(value, M3U8K_NO) == 0) {
		return 0;
	}
	
	return -1;
	
}

static enum M3U8ClosedCaption m3u8cc_unstringify(const char* const name) {
	
	if (strcmp(name, M3U8K_CC1) == 0) {
		return M3U8_CLOSED_CAPTION_CC1;
	}
	
	if (strcmp(name, M3U8K_CC2) == 0) {
		return M3U8_CLOSED_CAPTION_CC2;
	}
	
	if (strcmp(name, M3U8K_CC3) == 0) {
		return M3U8_CLOSED_CAPTION_CC3;
	}
	
	if (strcmp(name, M3U8K_CC4) == 0) {
		return M3U8_CLOSED_CAPTION_CC4;
	}
	
	if (strcmp(name, M3U8K_SERVICE1) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE1;
	}
	
	if (strcmp(name, M3U8K_SERVICE2) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE2;
	}
	
	if (strcmp(name, M3U8K_SERVICE3) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE3;
	}
	
	if (strcmp(name, M3U8K_SERVICE4) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE4;
	}
	
	if (strcmp(name, M3U8K_SERVICE5) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE5;
	}
	
	if (strcmp(name, M3U8K_SERVICE6) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE6;
	}
	
	if (strcmp(name, M3U8K_SERVICE7) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE7;
	}
	
	if (strcmp(name, M3U8K_SERVICE8) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE8;
	}
	
	if (strcmp(name, M3U8K_SERVICE9) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE9;
	}
	
	if (strcmp(name, M3U8K_SERVICE10) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE10;
	}
	
	if (strcmp(name, M3U8K_SERVICE11) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE11;
	}
	
	if (strcmp(name, M3U8K_SERVICE12) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE12;
	}
	
	if (strcmp(name, M3U8K_SERVICE13) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE13;
	}
	
	if (strcmp(name, M3U8K_SERVICE14) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE14;
	}
	
	if (strcmp(name, M3U8K_SERVICE15) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE15;
	}
	
	if (strcmp(name, M3U8K_SERVICE16) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE16;
	}
	
	if (strcmp(name, M3U8K_SERVICE17) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE17;
	}
	
	if (strcmp(name, M3U8K_SERVICE18) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE18;
	}
	
	if (strcmp(name, M3U8K_SERVICE19) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE19;
	}
	
	if (strcmp(name, M3U8K_SERVICE20) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE20;
	}
	
	if (strcmp(name, M3U8K_SERVICE21) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE21;
	}
	
	if (strcmp(name, M3U8K_SERVICE22) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE22;
	}
	
	if (strcmp(name, M3U8K_SERVICE23) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE23;
	}
	
	if (strcmp(name, M3U8K_SERVICE24) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE24;
	}
	
	if (strcmp(name, M3U8K_SERVICE25) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE25;
	}
	
	if (strcmp(name, M3U8K_SERVICE26) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE26;
	}
	
	if (strcmp(name, M3U8K_SERVICE27) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE27;
	}
	
	if (strcmp(name, M3U8K_SERVICE28) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE28;
	}
	
	if (strcmp(name, M3U8K_SERVICE29) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE29;
	}
	
	if (strcmp(name, M3U8K_SERVICE30) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE30;
	}
	
	if (strcmp(name, M3U8K_SERVICE31) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE31;
	}
	
	if (strcmp(name, M3U8K_SERVICE32) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE32;
	}
	
	if (strcmp(name, M3U8K_SERVICE33) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE33;
	}
	
	if (strcmp(name, M3U8K_SERVICE34) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE34;
	}
	
	if (strcmp(name, M3U8K_SERVICE35) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE35;
	}
	
	if (strcmp(name, M3U8K_SERVICE36) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE36;
	}
	
	if (strcmp(name, M3U8K_SERVICE37) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE37;
	}
	
	if (strcmp(name, M3U8K_SERVICE38) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE38;
	}
	
	if (strcmp(name, M3U8K_SERVICE39) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE39;
	}
	
	if (strcmp(name, M3U8K_SERVICE40) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE40;
	}
	
	if (strcmp(name, M3U8K_SERVICE41) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE41;
	}
	
	if (strcmp(name, M3U8K_SERVICE42) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE42;
	}
	
	if (strcmp(name, M3U8K_SERVICE43) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE43;
	}
	
	if (strcmp(name, M3U8K_SERVICE44) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE44;
	}
	
	if (strcmp(name, M3U8K_SERVICE45) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE45;
	}
	
	if (strcmp(name, M3U8K_SERVICE46) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE46;
	}
	
	if (strcmp(name, M3U8K_SERVICE47) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE47;
	}
	
	if (strcmp(name, M3U8K_SERVICE48) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE48;
	}
	
	if (strcmp(name, M3U8K_SERVICE49) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE49;
	}
	
	if (strcmp(name, M3U8K_SERVICE50) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE50;
	}
	
	if (strcmp(name, M3U8K_SERVICE51) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE51;
	}
	
	if (strcmp(name, M3U8K_SERVICE52) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE52;
	}
	
	if (strcmp(name, M3U8K_SERVICE53) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE53;
	}
	
	if (strcmp(name, M3U8K_SERVICE54) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE54;
	}
	
	if (strcmp(name, M3U8K_SERVICE55) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE55;
	}
	
	if (strcmp(name, M3U8K_SERVICE56) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE56;
	}
	
	if (strcmp(name, M3U8K_SERVICE57) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE57;
	}
	
	if (strcmp(name, M3U8K_SERVICE58) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE58;
	}
	
	if (strcmp(name, M3U8K_SERVICE59) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE59;
	}
	
	if (strcmp(name, M3U8K_SERVICE60) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE60;
	}
	
	if (strcmp(name, M3U8K_SERVICE61) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE61;
	}
	
	if (strcmp(name, M3U8K_SERVICE62) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE62;
	}
	
	if (strcmp(name, M3U8K_SERVICE63) == 0) {
		return M3U8_CLOSED_CAPTION_SERVICE63;
	}
	
	return M3U8_CLOSED_CAPTION_UNKNOWN;
	
}

const char* m3u8cc_stringify(const enum M3U8ClosedCaption value) {
	
	switch (value) {
		case M3U8_CLOSED_CAPTION_CC1:
			return M3U8K_CC1;
		case M3U8_CLOSED_CAPTION_CC2:
			return M3U8K_CC2;
		case M3U8_CLOSED_CAPTION_CC3:
			return M3U8K_CC3;
		case M3U8_CLOSED_CAPTION_CC4:
			return M3U8K_CC4;
		case M3U8_CLOSED_CAPTION_SERVICE1:
			return M3U8K_SERVICE1;
		case M3U8_CLOSED_CAPTION_SERVICE2:
			return M3U8K_SERVICE2;
		case M3U8_CLOSED_CAPTION_SERVICE3:
			return M3U8K_SERVICE3;
		case M3U8_CLOSED_CAPTION_SERVICE4:
			return M3U8K_SERVICE4;
		case M3U8_CLOSED_CAPTION_SERVICE5:
			return M3U8K_SERVICE5;
		case M3U8_CLOSED_CAPTION_SERVICE6:
			return M3U8K_SERVICE6;
		case M3U8_CLOSED_CAPTION_SERVICE7:
			return M3U8K_SERVICE7;
		case M3U8_CLOSED_CAPTION_SERVICE8:
			return M3U8K_SERVICE8;
		case M3U8_CLOSED_CAPTION_SERVICE9:
			return M3U8K_SERVICE9;
		case M3U8_CLOSED_CAPTION_SERVICE10:
			return M3U8K_SERVICE10;
		case M3U8_CLOSED_CAPTION_SERVICE11:
			return M3U8K_SERVICE11;
		case M3U8_CLOSED_CAPTION_SERVICE12:
			return M3U8K_SERVICE12;
		case M3U8_CLOSED_CAPTION_SERVICE13:
			return M3U8K_SERVICE13;
		case M3U8_CLOSED_CAPTION_SERVICE14:
			return M3U8K_SERVICE14;
		case M3U8_CLOSED_CAPTION_SERVICE15:
			return M3U8K_SERVICE15;
		case M3U8_CLOSED_CAPTION_SERVICE16:
			return M3U8K_SERVICE16;
		case M3U8_CLOSED_CAPTION_SERVICE17:
			return M3U8K_SERVICE17;
		case M3U8_CLOSED_CAPTION_SERVICE18:
			return M3U8K_SERVICE18;
		case M3U8_CLOSED_CAPTION_SERVICE19:
			return M3U8K_SERVICE19;
		case M3U8_CLOSED_CAPTION_SERVICE20:
			return M3U8K_SERVICE20;
		case M3U8_CLOSED_CAPTION_SERVICE21:
			return M3U8K_SERVICE21;
		case M3U8_CLOSED_CAPTION_SERVICE22:
			return M3U8K_SERVICE22;
		case M3U8_CLOSED_CAPTION_SERVICE23:
			return M3U8K_SERVICE23;
		case M3U8_CLOSED_CAPTION_SERVICE24:
			return M3U8K_SERVICE24;
		case M3U8_CLOSED_CAPTION_SERVICE25:
			return M3U8K_SERVICE25;
		case M3U8_CLOSED_CAPTION_SERVICE26:
			return M3U8K_SERVICE26;
		case M3U8_CLOSED_CAPTION_SERVICE27:
			return M3U8K_SERVICE27;
		case M3U8_CLOSED_CAPTION_SERVICE28:
			return M3U8K_SERVICE28;
		case M3U8_CLOSED_CAPTION_SERVICE29:
			return M3U8K_SERVICE29;
		case M3U8_CLOSED_CAPTION_SERVICE30:
			return M3U8K_SERVICE30;
		case M3U8_CLOSED_CAPTION_SERVICE31:
			return M3U8K_SERVICE31;
		case M3U8_CLOSED_CAPTION_SERVICE32:
			return M3U8K_SERVICE32;
		case M3U8_CLOSED_CAPTION_SERVICE33:
			return M3U8K_SERVICE33;
		case M3U8_CLOSED_CAPTION_SERVICE34:
			return M3U8K_SERVICE34;
		case M3U8_CLOSED_CAPTION_SERVICE35:
			return M3U8K_SERVICE35;
		case M3U8_CLOSED_CAPTION_SERVICE36:
			return M3U8K_SERVICE36;
		case M3U8_CLOSED_CAPTION_SERVICE37:
			return M3U8K_SERVICE37;
		case M3U8_CLOSED_CAPTION_SERVICE38:
			return M3U8K_SERVICE38;
		case M3U8_CLOSED_CAPTION_SERVICE39:
			return M3U8K_SERVICE39;
		case M3U8_CLOSED_CAPTION_SERVICE40:
			return M3U8K_SERVICE40;
		case M3U8_CLOSED_CAPTION_SERVICE41:
			return M3U8K_SERVICE41;
		case M3U8_CLOSED_CAPTION_SERVICE42:
			return M3U8K_SERVICE42;
		case M3U8_CLOSED_CAPTION_SERVICE43:
			return M3U8K_SERVICE43;
		case M3U8_CLOSED_CAPTION_SERVICE44:
			return M3U8K_SERVICE44;
		case M3U8_CLOSED_CAPTION_SERVICE45:
			return M3U8K_SERVICE45;
		case M3U8_CLOSED_CAPTION_SERVICE46:
			return M3U8K_SERVICE46;
		case M3U8_CLOSED_CAPTION_SERVICE47:
			return M3U8K_SERVICE47;
		case M3U8_CLOSED_CAPTION_SERVICE48:
			return M3U8K_SERVICE48;
		case M3U8_CLOSED_CAPTION_SERVICE49:
			return M3U8K_SERVICE49;
		case M3U8_CLOSED_CAPTION_SERVICE50:
			return M3U8K_SERVICE50;
		case M3U8_CLOSED_CAPTION_SERVICE51:
			return M3U8K_SERVICE51;
		case M3U8_CLOSED_CAPTION_SERVICE52:
			return M3U8K_SERVICE52;
		case M3U8_CLOSED_CAPTION_SERVICE53:
			return M3U8K_SERVICE53;
		case M3U8_CLOSED_CAPTION_SERVICE54:
			return M3U8K_SERVICE54;
		case M3U8_CLOSED_CAPTION_SERVICE55:
			return M3U8K_SERVICE55;
		case M3U8_CLOSED_CAPTION_SERVICE56:
			return M3U8K_SERVICE56;
		case M3U8_CLOSED_CAPTION_SERVICE57:
			return M3U8K_SERVICE57;
		case M3U8_CLOSED_CAPTION_SERVICE58:
			return M3U8K_SERVICE58;
		case M3U8_CLOSED_CAPTION_SERVICE59:
			return M3U8K_SERVICE59;
		case M3U8_CLOSED_CAPTION_SERVICE60:
			return M3U8K_SERVICE60;
		case M3U8_CLOSED_CAPTION_SERVICE61:
			return M3U8K_SERVICE61;
		case M3U8_CLOSED_CAPTION_SERVICE62:
			return M3U8K_SERVICE62;
		case M3U8_CLOSED_CAPTION_SERVICE63:
			return M3U8K_SERVICE63;
		default:
			return M3U8K_UNKNOWN;
	}
	
}

static enum M3U8MediaType m3u8media_unstringify(const char* const name) {
	
	if (strcmp(name, M3U8K_AUDIO) == 0) {
		return M3U8_MEDIA_TYPE_AUDIO;
	}
	
	if (strcmp(name, M3U8K_VIDEO) == 0) {
		return M3U8_MEDIA_TYPE_VIDEO;
	}
	
	if (strcmp(name, M3U8K_SUBTITLES) == 0) {
		return M3U8_MEDIA_TYPE_SUBTITLES;
	}
	
	if (strcmp(name, M3U8K_CLOSED_CAPTIONS) == 0) {
		return M3U8_MEDIA_TYPE_CLOSED_CAPTIONS;
	}
	
	return M3U8_MEDIA_TYPE_UNKNOWN;
	
}

const char* m3u8media_stringify(const enum M3U8MediaType value) {
	
	switch (value) {
		case M3U8_MEDIA_TYPE_AUDIO:
			return M3U8K_AUDIO;
		case M3U8_MEDIA_TYPE_VIDEO:
			return M3U8K_VIDEO;
		case M3U8_MEDIA_TYPE_SUBTITLES:
			return M3U8K_SUBTITLES;
		case M3U8_MEDIA_TYPE_CLOSED_CAPTIONS:
			return M3U8K_CLOSED_CAPTIONS;
		default:
			return M3U8K_UNKNOWN;
	}
	
}

static enum M3U8HDCPLevel m3u8hdcplvl_unstringify(const char* const name) {
	
	if (strcmp(name, M3U8K_NONE) == 0) {
		return M3U8_HDCP_LEVEL_NONE;
	}
	
	if (strcmp(name, M3U8K_TYPE_0) == 0) {
		return M3U8_HDCP_LEVEL_TYPE_0;
	}
	
	return M3U8_HDCP_LEVEL_UNKNOWN;
	
}

static enum M3U8EncryptionMethod m3u8em_unstringify(const char* const name) {
	
	if (strcmp(name, M3U8K_NONE) == 0) {
		return M3U8_ENCRYPTION_METHOD_NONE;
	}
	
	if (strcmp(name, M3U8K_AES_128) == 0) {
		return M3U8_ENCRYPTION_METHOD_AES_128;
	}
	
	if (strcmp(name, M3U8K_SAMPLE_AES) == 0) {
		return M3U8_ENCRYPTION_METHOD_SAMPLE_AES;
	}
	
	return M3U8_ENCRYPTION_METHOD_UNKNOWN;
	
}

const char* m3u8em_stringify(const enum M3U8EncryptionMethod value) {
	
	switch (value) {
		case M3U8_ENCRYPTION_METHOD_NONE:
			return M3U8K_NONE;
		case M3U8_ENCRYPTION_METHOD_AES_128:
			return M3U8K_AES_128;
		case M3U8_ENCRYPTION_METHOD_SAMPLE_AES:
			return M3U8K_SAMPLE_AES;
		default:
			return M3U8K_UNKNOWN;
	}
	
}

static enum M3U8MediaPlaylistType m3u8mpt_unstringify(const char* const name) {
	
	if (strcmp(name, M3U8K_EVENT) == 0) {
		return M3U8_MEDIA_PLAYLIST_TYPE_EVENT;
	}
	
	if (strcmp(name, M3U8K_VOD) == 0) {
		return M3U8_MEDIA_PLAYLIST_TYPE_VOD;
	}
	
	return M3U8_MEDIA_PLAYLIST_TYPE_UNKNOWN;
	
}

bigfloat_t m3u8stream_getduration(const struct M3U8Stream* const stream) {
	
	size_t index = 0;
	bigfloat_t duration = 0;
	
	for (index = 0; index < stream->offset; index++) {
		const struct M3U8StreamItem* const item = &stream->items[index];
		const struct M3U8Segment* const  segment = (struct M3U8Segment*) item->item;
		
		if (item->type != M3U8_STREAM_SEGMENT) {
			continue;
		}
		
		duration += segment->duration;
	}
	
	return duration;
	
}

static biguint_t m3u8stream_getsize(const bigfloat_t duration, const biguint_t bandwidth) {
	
	const biguint_t size = (duration * bandwidth) / 8;
	return size;
	
}

biguint_t m3u8stream_getsegments(const struct M3U8Stream* const stream) {
	
	size_t index = 0;
	biguint_t segments = 0;
	
	for (index = 0; index < stream->offset; index++) {
		const struct M3U8StreamItem* const item = &stream->items[index];
		
		if (item->type != M3U8_STREAM_SEGMENT) {
			continue;
		}
		
		segments++;
	}
	
	return segments;
	
}

static biguint_t m3u8datetime_getepoch(const struct M3U8DateTime* const date_time) {
	
	biguint_t epoch = 0;
	
	biguint_t month = 0;
	biguint_t year = 0;
	
	const int month_days[] = {
		0, 31, 28, 31, 30, 31,
		30, 31, 31, 30, 31, 30, 31
	};
	
	epoch += date_time->sec;
	epoch += date_time->min * 60;
	epoch += date_time->hour * 3600;
	epoch += (date_time->mday - (date_time->mday > 1)) * 86400;
	
	for (month = 1; month < date_time->mon; month++) {
		epoch += (month_days[month] + (month == 2 && isleap(date_time->year))) * 86400;
	}
	
	for (year = 1970; year < date_time->year; year++) {
		epoch += (365 + isleap(year)) * 86400;
	}
	
	return epoch;
	
}

static int m3u8vs_compare(const void* a, const void* b) {
	
	biguint_t ab = 0;
	biguint_t bb = 0;
	
	struct M3U8StreamItem* ia = (struct M3U8StreamItem*) a;
	struct M3U8StreamItem* ib = (struct M3U8StreamItem*) b;
	
	if (ia->type == M3U8_STREAM_VARIANT_STREAM) {
		ab = ((struct M3U8VariantStream*) ia->item)->bandwidth;
	}
	
	if (ib->type == M3U8_STREAM_VARIANT_STREAM) {
		bb = ((struct M3U8VariantStream*) ib->item)->bandwidth;
	}
	
	return ab - bb;
}

int m3u8stream_parse(struct M3U8Stream* const stream) {
	
	int err = 0;
	int status = 0;
	
	size_t index = 0;
	size_t subindex = 0;
	size_t subsubindex = 0;
	size_t subsubsubindex = 0;
	size_t size = 0;
	
	struct M3U8Item* item = NULL;
	
	const struct M3U8Attribute* attribute = NULL;
	const struct M3U8Attribute* subattribute = NULL;
	
	void* value = NULL;
	
	stream->size = sizeof(*stream->items) * stream->playlist.tags.offset;
	stream->items = malloc(stream->size);
	
	if (stream->items == NULL) {
		err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	struct M3U8StreamItem stream_item = {0};
	
	for (index = 0; index < stream->playlist.tags.offset; index++) {
		struct M3U8Tag* const tag = &stream->playlist.tags.items[index];
		
		stream_item.type = M3U8_STREAM_UNKNOWN;
		stream_item.item = NULL;
		
		switch (tag->type) {
			case M3U8_TAG_EXT_X_VERSION: {
				struct M3U8Version version = {0};
				
				version.version = *((biguint_t*) tag->value.value);
				version.tag = tag;
				
				stream_item.type = M3U8_STREAM_VERSION;
				stream_item.item = malloc(sizeof(version));
				
				if (stream_item.item == NULL) {
					err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
					goto end;
				}
				
				memcpy(stream_item.item, &version, sizeof(version));
				
				break;
			}
			case M3U8_TAG_EXTINF: {
				struct M3U8Segment segment = {0};
				
				/*
				DURATION: This value is REQUIRED.
				*/
				item = m3u8tag_getitem(tag, 0);
				
				if (item == NULL) {
					err = M3U8ERR_ITEM_MISSING;
					goto end;
				}
				
				if (item->vtype == M3U8_VITEM_TYPE_UINT) {
					segment.duration = (bigfloat_t) *((biguint_t*) item->value);
				} else {
					segment.duration = *((bigfloat_t*) item->value);
				}
				
				/*
				TITLE: This value is OPTIONAL.
				*/
				item = m3u8tag_getitem(tag, 1);
				
				if (item != NULL) {
					segment.title = item->value;
				}
				
				if ((index + 1) < stream->playlist.tags.offset) {
					struct M3U8Tag* const subtag = &stream->playlist.tags.items[index + 1];
					
					switch (subtag->type) {
						case M3U8_TAG_EXT_X_BYTERANGE: {
							/*
							BYTERANGE: This value is OPTIONAL.
							*/
							item = m3u8tag_getitem(subtag, 0);
							
							if (item == NULL) {
								err = M3U8ERR_ITEM_MISSING;
								goto end;
							}
							
							segment.byterange = *((struct M3U8ByteRange*) item->value);
							index++;
							
							break;
						}
						case M3U8_TAG_EXT_X_BITRATE: {
							/*
							BITRATE: This value is OPTIONAL.
							*/
							item = m3u8tag_getitem(subtag, 0);
							
							if (item == NULL) {
								err = M3U8ERR_ITEM_MISSING;
								goto end;
							}
							
							segment.bitrate = *((biguint_t*) item->value);
							index++;
							
							break;
						}
						default: {
							break;
						}
					}
				}
				
				/*
				URI: This value is REQUIRED.
				*/
				segment.uri = tag->uri;
				
				if (stream->offset > 0) {
					struct M3U8StreamItem* const subitem = &stream->items[stream->offset - 1];
					
					if (subitem->type == M3U8_STREAM_KEY) {
						memcpy(&segment.key, subitem->item, sizeof(segment.key));
						stream->offset--;
						
						free(subitem->item);
					}
				}
				
				segment.tag = tag;
				
				stream_item.type = M3U8_STREAM_SEGMENT;
				stream_item.item = malloc(sizeof(segment));
				
				if (stream_item.item == NULL) {
					err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
					goto end;
				}
				
				memcpy(stream_item.item, &segment, sizeof(segment));
				
				break;
			}
			case M3U8_TAG_EXT_X_MAP: {
				struct M3U8Map map = {0};
				
				/*
				URI: This attribute is REQUIRED.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_URI);
				
				if (attribute == NULL) {
					err = M3U8ERR_ATTRIBUTE_MISSING;
					goto end;
				}
				
				map.uri = attribute->value;
				
				/*
				BYTERANGE: This attribute is OPTIONAL.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_BYTERANGE);
				
				if (attribute != NULL) {
					void* value = NULL;
					const int status = m3u8parser_getbrange(attribute->value, &value);
					
					switch (status) {
						case M3U8ERR_SUCCESS:
							break;
						case M3U8ERR_PARSER_INVALID_BRANGE:
							err = M3U8ERR_ATTRIBUTE_INVALID_BRANGE;
							goto end;
						default:
							err = status;
							goto end;
					}
					
					map.byterange = *((struct M3U8ByteRange*) value);
					free(value);
				}
				
				map.tag = tag;
				
				stream_item.type = M3U8_STREAM_MAP;
				stream_item.item = malloc(sizeof(map));
				
				if (stream_item.item == NULL) {
					err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
					goto end;
				}
				
				memcpy(stream_item.item, &map, sizeof(map));
				
				break;
			}
			case M3U8_TAG_EXT_X_BYTERANGE:
			case M3U8_TAG_EXT_X_BITRATE: {
				/*
				The value of these tags was merged with the M3U8Segment structure.
				*/
				err = M3U8ERR_PLAYLIST_WRONG_TAG_POSITION;
				goto end;
			}
			case M3U8_TAG_EXT_X_PROGRAM_DATE_TIME: {
				struct M3U8DateTime date_time = {0};
				
				/*
				DATE-TIME: This value is REQUIRED.
				*/
				item = m3u8tag_getitem(tag, 0);
				
				if (item == NULL) {
					err = M3U8ERR_ITEM_MISSING;
					goto end;
				}
				
				date_time = *((struct M3U8DateTime*) item->value);
				
				stream_item.type = M3U8_STREAM_PROGRAM_DATE_TIME;
				stream_item.item = malloc(sizeof(date_time));
				
				if (stream_item.item == NULL) {
					err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
					goto end;
				}
				
				memcpy(stream_item.item, &date_time, sizeof(date_time));
				
				break;
			}
			case M3U8_TAG_EXT_X_ALLOW_CACHE: {
				struct M3U8AllowCache allow_cache = {0};
				
				/*
				ALLOW-CACHE: This value is REQUIRED.
				*/
				item = m3u8tag_getitem(tag, 0);
				
				if (item == NULL) {
					err = M3U8ERR_ITEM_MISSING;
					goto end;
				}
				
				allow_cache.allow_cache = m3u8stream_getbool(item->value);
				
				if (allow_cache.allow_cache == -1) {
					err = M3U8ERR_ATTRIBUTE_INVALID_ESTRING;
					goto end;
				}
				
				allow_cache.tag = tag;
				
				stream_item.type = M3U8_STREAM_ALLOW_CACHE;
				stream_item.item = malloc(sizeof(allow_cache));
				
				if (stream_item.item == NULL) {
					err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
					goto end;
				}
				
				memcpy(stream_item.item, &allow_cache, sizeof(allow_cache));
				
				break;
			}
			case M3U8_TAG_EXT_X_DATERANGE: {
				struct M3U8DateRange range = {0};
				
				/*
				ID: This attribute is REQUIRED.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_ID);
				
				if (attribute == NULL) {
					err = M3U8ERR_ATTRIBUTE_MISSING;
					goto end;
				}
				
				range.id = attribute->value;
				
				/*
				CLASS: This attribute is OPTIONAL.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_CLASS);
				
				if (attribute != NULL) {
					range.class = attribute->value;
				}
				
				/*
				START-DATE: This attribute is REQUIRED.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_START_DATE);
				
				if (attribute == NULL) {
					err = M3U8ERR_ATTRIBUTE_MISSING;
					goto end;
				}
				
				status = m3u8parser_getdtime(attribute->value, &value);
				
				switch (status) {
					case M3U8ERR_SUCCESS:
						break;
					case M3U8ERR_PARSER_INVALID_DTIME:
						err = M3U8ERR_ATTRIBUTE_INVALID_DTIME;
						goto end;
					default:
						err = status;
						goto end;
				}
				
				range.start_date = *((struct M3U8DateTime*) value);
				free(value);
				
				/*
				END-DATE: This attribute is OPTIONAL.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_END_DATE);
				
				if (attribute != NULL) {
					status = m3u8parser_getdtime(attribute->value, &value);
					
					switch (status) {
						case M3U8ERR_SUCCESS:
							break;
						case M3U8ERR_PARSER_INVALID_DTIME:
							err = M3U8ERR_ATTRIBUTE_INVALID_DTIME;
							goto end;
						default:
							err = status;
							goto end;
					}
					
					range.end_date = *((struct M3U8DateTime*) value);
					free(value);
				}
				
				/*
				DURATION: This attribute is OPTIONAL.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_DURATION);
				
				if (attribute != NULL) {
					range.duration = *((bigfloat_t*) attribute->value);
				}
				
				/*
				PLANNED-DURATION: This attribute is OPTIONAL.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_PLANNED_DURATION);
				
				if (attribute != NULL) {
					range.planned_duration = *((bigfloat_t*) attribute->value);
				}
				
				/*
				END-ON-NEXT: This attribute is OPTIONAL.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_END_ON_NEXT);
				
				if (attribute != NULL) {
					range.end_on_next = m3u8stream_getbool(attribute->value);
					
					if (!range.end_on_next) {
						err = M3U8ERR_ATTRIBUTE_INVALID_ESTRING;
						goto end;
					}
					
					/*
					An EXT-X-DATERANGE tag with an END-ON-NEXT=YES attribute MUST have a
					CLASS attribute.
					*/
					if (range.class == NULL) {
						err = M3U8ERR_ATTRIBUTE_MISSING;
						goto end;
					}
					
					/*
					An EXT-X-DATERANGE tag with an END-ON-NEXT=YES attribute MUST NOT
					contain DURATION or END-DATE attributes.
					*/
					if (range.duration || range.end_date.year) {
						err = M3U8ERR_ATTRIBUTE_UNEXPECTED;
						goto end;
					}
				}
				
				/*
				If a Playlist contains an EXT-X-DATERANGE tag, it MUST also contain
				at least one EXT-X-PROGRAM-DATE-TIME tag.
				*/
				status = m3u8playlist_tagexists(&stream->playlist, M3U8_TAG_EXT_X_PROGRAM_DATE_TIME);
				
				if (!status) {
					err = M3U8ERR_PLAYLIST_MISSING_TAG;
					goto end;
				}
				
				/*
				If a Playlist contains two EXT-X-DATERANGE tags with the same ID
				attribute value, then any AttributeName that appears in both tags
				MUST have the same AttributeValue.
				*/
				for (subindex = 0; subindex < stream->playlist.tags.offset; subindex++) {
					const struct M3U8Tag* const a = &stream->playlist.tags.items[subindex];
					
					if (a->type != M3U8_TAG_EXT_X_DATERANGE) {
						continue;
					}
					
					attribute = m3u8tag_igetattr(a, M3U8_ATTRIBUTE_ID);
					
					if (attribute == NULL) {
						continue;
					}
					
					for (subsubindex = 0; subsubindex < stream->playlist.tags.offset; subsubindex++) {
						const struct M3U8Tag* const b = &stream->playlist.tags.items[subsubindex];
						
						if (b->type != M3U8_TAG_EXT_X_DATERANGE) {
							continue;
						}
						
						if (subsubindex == subindex) {
							continue;
						}
						
						subattribute = m3u8tag_igetattr(b, M3U8_ATTRIBUTE_ID);
						
						if (subattribute == NULL) {
							continue;
						}
						
						if (strcmp(attribute->value, subattribute->value) == 0) {
							for (subsubsubindex = 0; subsubsubindex < a->attributes.offset; subsubsubindex++) {
								attribute = &a->attributes.items[subsubsubindex];
								subattribute = m3u8tag_igetattr(b, attribute->type);
								
								if (subattribute == NULL) {
									continue;
								}
								
								size = m3u8attribute_getvsize(attribute);
								
								status = memcmp(attribute->value, subattribute->value, size) != 0;
								
								if (status) {
									err = M3U8ERR_TAG_NON_MATCHING_ATTRIBUTES;
									goto end;
								}
							}
						}
					}
				}
				
				/*
				If a Date Range contains both a DURATION attribute and an END-DATE
				attribute, the value of the END-DATE attribute MUST be equal to the
				value of the START-DATE attribute plus the value of the DURATION
				attribute.
				*/
				if (range.duration && range.end_date.year) {
					const biguint_t a = (
						m3u8datetime_getepoch(&range.start_date) + range.duration
					);
					
					const biguint_t b = (
						m3u8datetime_getepoch(&range.end_date)
					);
					
					if (a != b) {
						err = M3U8ERR_ATTRIBUTE_WRONG_END_DATE;
						goto end;
					}
				}
				
				range.tag = tag;
				
				stream_item.type = M3U8_STREAM_DATE_RANGE;
				stream_item.item = malloc(sizeof(range));
				
				if (stream_item.item == NULL) {
					err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
					goto end;
				}
				
				memcpy(stream_item.item, &range, sizeof(range));
				
				break;
			}
			case M3U8_TAG_EXT_X_TARGETDURATION: {
				struct M3U8TargetDuration duration = {0};
				
				/*
				DURATION: This value is REQUIRED.
				*/
				item = m3u8tag_getitem(tag, 0);
				
				if (item == NULL) {
					err = M3U8ERR_ITEM_MISSING;
					goto end;
				}
				
				duration.duration = *((biguint_t*) item->value);
				duration.tag = tag;
				
				stream_item.type = M3U8_STREAM_TARGET_DURATION;
				stream_item.item = malloc(sizeof(duration));
				
				if (stream_item.item == NULL) {
					err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
					goto end;
				}
				
				memcpy(stream_item.item, &duration, sizeof(duration));
				
				break;
			}
			case M3U8_TAG_EXT_X_MEDIA_SEQUENCE: {
				struct M3U8MediaSequence sequence = {0};
				
				/*
				NUMBER: This value is REQUIRED.
				*/
				item = m3u8tag_getitem(tag, 0);
				
				if (item == NULL) {
					err = M3U8ERR_ITEM_MISSING;
					goto end;
				}
				
				sequence.number = *((biguint_t*) item->value);
				
				/*
				The EXT-X-MEDIA-SEQUENCE tag MUST appear before the first Media
				Segment in the Playlist.
				*/
				for (subindex = 0; subindex < stream->playlist.tags.offset; subindex++) {
					const struct M3U8Tag* const subtag = &stream->playlist.tags.items[subindex];
					
					if (subtag->type != M3U8_TAG_EXTINF) {
						continue;
					}
					
					if (subindex > index) {
						break;
					}
					
					err = M3U8ERR_PLAYLIST_WRONG_TAG_POSITION;
					goto end;
				}
				
				sequence.tag = tag;
				
				stream_item.type = M3U8_STREAM_MEDIA_SEQUENCE;
				stream_item.item = malloc(sizeof(sequence));
				
				if (stream_item.item == NULL) {
					err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
					goto end;
				}
				
				memcpy(stream_item.item, &sequence, sizeof(sequence));
				
				break;
			}
			case M3U8_TAG_EXT_X_DISCONTINUITY_SEQUENCE: {
				struct M3U8DiscontinuitySequence sequence = {0};
				
				/*
				NUMBER: This value is REQUIRED.
				*/
				item = m3u8tag_getitem(tag, 0);
				
				if (item == NULL) {
					err = M3U8ERR_ITEM_MISSING;
					goto end;
				}
				
				sequence.number = *((biguint_t*) item->value);
				
				/*
				The EXT-X-DISCONTINUITY-SEQUENCE tag MUST appear before the first
				Media Segment in the Playlist.
				
				The EXT-X-DISCONTINUITY-SEQUENCE tag MUST appear before any EXT-
				X-DISCONTINUITY tag.
				*/
				for (subindex = 0; subindex < stream->playlist.tags.offset; subindex++) {
					const struct M3U8Tag* const subtag = &stream->playlist.tags.items[subindex];
					
					if (!(subtag->type == M3U8_TAG_EXTINF || subtag->type == M3U8_TAG_EXT_X_DISCONTINUITY)) {
						continue;
					}
					
					if (subindex > index) {
						break;
					}
					
					err = M3U8ERR_PLAYLIST_WRONG_TAG_POSITION;
					goto end;
				}
				
				sequence.tag = tag;
				
				stream_item.type = M3U8_STREAM_DISCONTINUITY_SEQUENCE;
				stream_item.item = malloc(sizeof(sequence));
				
				if (stream_item.item == NULL) {
					err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
					goto end;
				}
				
				memcpy(stream_item.item, &sequence, sizeof(sequence));
				
				break;
			}
			case M3U8_TAG_EXT_X_ENDLIST: {
				stream_item.type = M3U8_STREAM_END_LIST;
				break;
			}
			case M3U8_TAG_EXT_X_PLAYLIST_TYPE: {
				struct M3U8StreamPlaylistType type = {0};
				
				/*
				TYPE: This value is REQUIRED.
				*/
				item = m3u8tag_getitem(tag, 0);
				
				if (item == NULL) {
					err = M3U8ERR_ITEM_MISSING;
					goto end;
				}
				
				type.type = m3u8mpt_unstringify(item->value);
				
				if (type.type == M3U8_MEDIA_PLAYLIST_TYPE_UNKNOWN) {
					err = M3U8ERR_ATTRIBUTE_INVALID_ESTRING;
					goto end;
				}
				
				type.tag = tag;
				
				stream_item.type = M3U8_STREAM_PLAYLIST_TYPE;
				stream_item.item = malloc(sizeof(type));
				
				if (stream_item.item == NULL) {
					err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
					goto end;
				}
				
				memcpy(stream_item.item, &type, sizeof(type));
				
				break;
			}
			case M3U8_TAG_EXT_X_I_FRAMES_ONLY: {
				stream_item.type = M3U8_STREAM_IFRAMES_ONLY;
				break;
			}
			case M3U8_TAG_EXT_X_DISCONTINUITY: {
				stream_item.type = M3U8_STREAM_DISCONTINUITY;
				break;
			}
			case M3U8_TAG_EXT_X_KEY:
			case M3U8_TAG_EXT_X_SESSION_KEY: {
				struct M3U8Key key = {0};
				
				/*
				METHOD: This attribute is REQUIRED.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_METHOD);
				
				if (attribute == NULL) {
					err = M3U8ERR_ATTRIBUTE_MISSING;
					goto end;
				}
				
				key.method = m3u8em_unstringify(attribute->value);
					
				if (key.method == M3U8_ENCRYPTION_METHOD_UNKNOWN) {
					err = M3U8ERR_ATTRIBUTE_INVALID_ESTRING;
					goto end;
				}
				
				/*
				The value of the METHOD attribute MUST NOT be NONE for the EXT-X-SESSION-KEY
				tag.
				*/
				if (tag->type == M3U8_TAG_EXT_X_SESSION_KEY && key.method == M3U8_ENCRYPTION_METHOD_NONE) {
					err = M3U8ERR_ATTRIBUTE_INVALID_ESTRING;
					goto end;
				}
				
				if (key.method != M3U8_ENCRYPTION_METHOD_NONE) {
					/*
					The URI attribute is REQUIRED if the encryption method is not NONE.
					*/
					attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_URI);
					
					if (attribute == NULL) {
						err = M3U8ERR_ATTRIBUTE_MISSING;
						goto end;
					}
					
					key.uri = attribute->value;
				}
				
				/*
				IV: This attribute is OPTIONAL.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_IV);
				
				if (attribute != NULL) {
					key.iv = *((struct M3U8Bytes*) attribute->value);
				}
				
				/*
				KEYFORMAT: This attribute is OPTIONAL.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_KEYFORMAT);
				
				if (attribute != NULL) {
					key.keyformat = attribute->value;
				}
				
				/*
				KEYFORMATVERSIONS: This attribute is OPTIONAL.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_KEYFORMATVERSIONS);
				
				if (attribute != NULL) {
					char version[M3U8_MAX_KFV_VALUE_LEN];
					biguint_t number = 0;
					
					biguint_t* items = NULL;
					
					const char* start = attribute->value;
					const char* end = strstr(start, "/");
					
					const char* vend = strchr(start, '\0');
					
					while (1) {
						if (end == NULL) {
							end = vend;
						}
						
						size = (size_t) (end - start);
						
						if (size < 1) {
							err = M3U8ERR_ATTRIBUTE_KEYFORMATVERSIONS_INVALID;
							goto end;
						}
						
						if (size > (M3U8_MAX_KFV_VALUE_LEN - 1)) {
							err = M3U8ERR_ATTRIBUTE_KEYFORMATVERSIONS_INVALID;
							goto end;
						}
						
						memcpy(version, start, size);
						version[size] = '\0';
						
						number = strtobui(version, NULL, 10);
						
						if (errno == ERANGE) {
							err = M3U8ERR_ATTRIBUTE_KEYFORMATVERSIONS_INVALID;
							goto end;
						}
						
						size = key.keyformatversions.size + sizeof(*key.keyformatversions.items) * 1;
						items = realloc(key.keyformatversions.items, size);
						
						if (items == NULL) {
							err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
							goto end;
						}
						
						key.keyformatversions.items = items;
						key.keyformatversions.size = size;
						
						key.keyformatversions.items[key.keyformatversions.offset++] = number;
						
						if (end == vend) {
							break;
						}
						
						start = end;
						start++;
						
						if (*start == '/') {
							start++;
						}
						
						end = strstr(start, "/");
					}
				}
				
				if (tag->type == M3U8_TAG_EXT_X_SESSION_KEY && key.uri != NULL) {
					/*
					If an EXT-X-SESSION-KEY is used, the values of the METHOD, KEYFORMAT, and
					KEYFORMATVERSIONS attributes MUST match any EXT-X-KEY with the same URI value.
					*/
					for (subindex = 0; subindex < stream->playlist.tags.offset; subindex++) {
						const struct M3U8Tag* const subtag = &stream->playlist.tags.items[subindex];
						
						if (subtag->type != M3U8_TAG_EXT_X_KEY) {
							continue;
						}
						
						attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_URI);
						
						if (attribute == NULL) {
							continue;
						}
						
						if (strcmp(key.uri, attribute->value) != 0) {
							continue;
						}
						
						/*
						METHOD attribute.
						*/
						attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_METHOD);
						subattribute = m3u8tag_igetattr(subtag, M3U8_ATTRIBUTE_METHOD);
						
						if (attribute == NULL || subattribute == NULL) {
							continue;
						}
						
						if (strcmp(attribute->value, subattribute->value) != 0) {
							err = M3U8ERR_TAG_NON_MATCHING_ATTRIBUTES;
							goto end;
						}
						
						/*
						KEYFORMAT attribute.
						*/
						attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_KEYFORMAT);
						subattribute = m3u8tag_igetattr(subtag, M3U8_ATTRIBUTE_KEYFORMAT);
						
						if (attribute == NULL || subattribute == NULL) {
							continue;
						}
						
						if (strcmp(attribute->value, subattribute->value) != 0) {
							err = M3U8ERR_TAG_NON_MATCHING_ATTRIBUTES;
							goto end;
						}
						
						/*
						KEYFORMATVERSIONS attribute.
						*/
						attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_KEYFORMATVERSIONS);
						subattribute = m3u8tag_igetattr(subtag, M3U8_ATTRIBUTE_KEYFORMATVERSIONS);
						
						if (attribute == NULL || subattribute == NULL) {
							continue;
						}
						
						if (strcmp(attribute->value, subattribute->value) != 0) {
							err = M3U8ERR_TAG_NON_MATCHING_ATTRIBUTES;
							goto end;
						}
					}
				}
				
				key.tag = tag;
				
				stream_item.type = M3U8_STREAM_KEY;
				stream_item.item = malloc(sizeof(key));
				
				if (stream_item.item == NULL) {
					err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
					goto end;
				}
				
				memcpy(stream_item.item, &key, sizeof(key));
				
				break;
			}
			case M3U8_TAG_EXT_X_INDEPENDENT_SEGMENTS: {
				stream_item.type = M3U8_STREAM_INDEPENDENT_SEGMENTS;
				break;
			}
			case M3U8_TAG_EXT_X_START: {
				struct M3U8Start start = {0};
				
				/*
				TIME-OFFSET: This attribute is REQUIRED.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_TIME_OFFSET);
				
				if (attribute == NULL) {
					err = M3U8ERR_ATTRIBUTE_MISSING;
					goto end;
				}
				
				start.time_offset = *((bigfloat_t*) attribute->value);
				
				/*
				PRECISE: This attribute is OPTIONAL.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_PRECISE);
				
				if (attribute != NULL) {
					start.precise = m3u8stream_getbool(attribute->value);
					
					if (start.precise == -1) {
						err = M3U8ERR_ATTRIBUTE_INVALID_ESTRING;
						goto end;
					}
				}
				
				start.tag = tag;
				
				stream_item.type = M3U8_STREAM_START;
				stream_item.item = malloc(sizeof(start));
				
				if (stream_item.item == NULL) {
					err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
					goto end;
				}
				
				memcpy(stream_item.item, &start, sizeof(start));
				
				break;
			}
			case M3U8_TAG_EXT_X_MEDIA: {
				struct M3U8Media media = {0};
				
				/*
				TYPE: This attribute is REQUIRED.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_TYPE);
				
				if (attribute == NULL) {
					err = M3U8ERR_ATTRIBUTE_MISSING;
					goto end;
				}
				
				media.type = m3u8media_unstringify(attribute->value);
				
				if (media.type == M3U8_MEDIA_TYPE_UNKNOWN) {
					err = M3U8ERR_ATTRIBUTE_INVALID_ESTRING;
					goto end;
				}
				
				/*
				URI: This attribute is OPTIONAL.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_URI);
				
				if (attribute != NULL) {
					if (media.type == M3U8_MEDIA_TYPE_CLOSED_CAPTIONS) {
						err = M3U8ERR_ATTRIBUTE_UNEXPECTED;
						goto end;
					}
					
					media.uri = attribute->value;
				}
				
				if (media.uri != NULL) {
					err = m3u8stream_load_subresource(stream, &media.stream, media.uri);
					
					if (err != M3U8ERR_SUCCESS) {
						goto end;
					}
					
					media.duration = m3u8stream_getduration(&media.stream);
					media.segments = m3u8stream_getsegments(&media.stream);
					media.average_duration = (media.duration / media.segments);
				}
				
				/*
				GROUP-ID: This attribute is REQUIRED.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_GROUP_ID);
				
				if (attribute == NULL) {
					err = M3U8ERR_ATTRIBUTE_MISSING;
					goto end;
				}
				
				media.group_id = attribute->value;
				
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_LANGUAGE);
				
				if (attribute != NULL) {
					media.language = attribute->value;
				}
				
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_ASSOC_LANGUAGE);
				
				if (attribute != NULL) {
					media.assoc_language = attribute->value;
				}
				
				/*
				NAME: This attribute is REQUIRED.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_NAME);
				
				if (attribute == NULL) {
					err = M3U8ERR_ATTRIBUTE_MISSING;
					goto end;
				}
				
				media.name = attribute->value;
				
				/*
				DEFAULT: This attribute is OPTIONAL.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_DEFAULT);
				
				if (attribute != NULL) {
					media.default_ = m3u8stream_getbool(attribute->value);
					
					if (media.default_ == -1) {
						err = M3U8ERR_ATTRIBUTE_INVALID_ESTRING;
						goto end;
					}
				}
				
				/*
				AUTOSELECT: This attribute is OPTIONAL.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_AUTOSELECT);
				
				if (attribute != NULL) {
					media.autoselect = m3u8stream_getbool(attribute->value);
					
					if (media.autoselect == -1) {
						err = M3U8ERR_ATTRIBUTE_INVALID_ESTRING;
						goto end;
					}
				}
				
				/*
				FORCED: This attribute is OPTIONAL.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_FORCED);
				
				if (attribute != NULL) {
					if (media.type != M3U8_MEDIA_TYPE_SUBTITLES) {
						err = M3U8ERR_ATTRIBUTE_UNEXPECTED;
						goto end;
					}
					
					media.forced = m3u8stream_getbool(attribute->value);
					
					if (media.forced == -1) {
						err = M3U8ERR_ATTRIBUTE_INVALID_ESTRING;
						goto end;
					}
				}
				
				/*
				INSTREAM-ID: This attribute is OPTIONAL.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_INSTREAM_ID);
				
				if (attribute != NULL) {
					if (media.type != M3U8_MEDIA_TYPE_CLOSED_CAPTIONS) {
						err = M3U8ERR_ATTRIBUTE_UNEXPECTED;
						goto end;
					}
					
					media.instream_id = m3u8cc_unstringify(attribute->value);
					
					if (media.instream_id == M3U8_CLOSED_CAPTION_UNKNOWN) {
						err = M3U8ERR_ATTRIBUTE_INVALID_QSTRING;
						goto end;
					}
				}
				
				/*
				CHARACTERISTICS: This attribute is OPTIONAL.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_CHARACTERISTICS);
				
				if (attribute != NULL) {
					media.characteristics = attribute->value;
				}
				
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_CHANNELS);
				
				if (attribute != NULL) {
					media.channels = attribute->value;
				}
				
				media.tag = tag;
				
				stream_item.type = M3U8_STREAM_MEDIA;
				stream_item.item = malloc(sizeof(media));
				
				if (stream_item.item == NULL) {
					err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
					goto end;
				}
				
				memcpy(stream_item.item, &media, sizeof(media));
				
				break;
			}
			case M3U8_TAG_EXT_X_STREAM_INF:
			case M3U8_TAG_EXT_X_I_FRAME_STREAM_INF: {
				struct M3U8VariantStream variant_stream = {0};
				
				/*
				BANDWIDTH: This attribute is REQUIRED.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_BANDWIDTH);
				
				if (attribute == NULL) {
					err = M3U8ERR_ATTRIBUTE_MISSING;
					goto end;
				}
				
				variant_stream.bandwidth = *((biguint_t*) attribute->value);
				
				/*
				AVERAGE-BANDWIDTH: This attribute is OPTIONAL.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_AVERAGE_BANDWIDTH);
				
				if (attribute != NULL) {
					variant_stream.average_bandwidth = *((biguint_t*) attribute->value);
				}
				
				/*
				PROGRAM-ID: This attribute is OPTIONAL.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_PROGRAM_ID);
				
				if (attribute != NULL) {
					variant_stream.program_id = *((biguint_t*) attribute->value);
				}
				
				/*
				CODECS: This attribute is OPTIONAL.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_CODECS);
				
				if (attribute != NULL) {
					variant_stream.codecs = attribute->value;
				}
				
				/*
				RESOLUTION: This attribute is OPTIONAL.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_RESOLUTION);
				
				if (attribute != NULL) {
					variant_stream.resolution = *((struct M3U8Resolution*) attribute->value);
				}
				
				/*
				FRAME-RATE: This attribute is OPTIONAL.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_FRAME_RATE);
				
				if (attribute != NULL) {
					variant_stream.frame_rate = *((bigfloat_t*) attribute->value);
				}
				
				/*
				HDCP-LEVEL: This attribute is OPTIONAL.
				*/
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_HDCP_LEVEL);
				
				if (attribute != NULL) {
					variant_stream.hdcp_level = m3u8hdcplvl_unstringify(attribute->value);
					
					if (variant_stream.hdcp_level == M3U8_HDCP_LEVEL_UNKNOWN) {
						err = M3U8ERR_ATTRIBUTE_INVALID_ESTRING;
						goto end;
					}
				}
				
				/*
				URI: This attribute is REQUIRED.
				*/
				if (tag->type == M3U8_TAG_EXT_X_STREAM_INF) {
					variant_stream.uri = tag->uri;
				} else {
					attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_URI);
					
					if (attribute == NULL) {
						err = M3U8ERR_ATTRIBUTE_MISSING;
						goto end;
					}
					
					variant_stream.uri = attribute->value;
				}
				
				err = m3u8stream_load_subresource(stream, &variant_stream.stream, variant_stream.uri);
				
				if (err != M3U8ERR_SUCCESS) {
					goto end;
				}
				
				variant_stream.duration = m3u8stream_getduration(&variant_stream.stream);
				variant_stream.size = m3u8stream_getsize(variant_stream.duration, variant_stream.bandwidth);
				variant_stream.segments = m3u8stream_getsegments(&variant_stream.stream);
				variant_stream.average_duration = (variant_stream.duration / variant_stream.segments);
				
				variant_stream.tag = tag;
				
				stream_item.type = M3U8_STREAM_VARIANT_STREAM;
				stream_item.item = malloc(sizeof(variant_stream));
				
				if (stream_item.item == NULL) {
					err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
					goto end;
				}
				
				memcpy(stream_item.item, &variant_stream, sizeof(variant_stream));
				
				break;
			}
			case M3U8_TAG_EXT_X_SESSION_DATA: {
				struct M3U8SessionData session = {0};
				
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_DATA_ID);
				
				if (attribute == NULL) {
					err = M3U8ERR_ATTRIBUTE_MISSING;
					goto end;
				}
				
				session.data_id = attribute->value;
				
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_VALUE);
				
				if (attribute != NULL) {
					session.value = attribute->value;
				}
				
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_URI);
				
				if (attribute != NULL) {
					session.uri = attribute->value;
				}
				
				attribute = m3u8tag_igetattr(tag, M3U8_ATTRIBUTE_LANGUAGE);
				
				if (attribute != NULL) {
					session.language = attribute->value;
				}
				
				/*
				Each EXT-X-SESSION-DATA tag MUST contain either a VALUE or URI
				attribute, but not both.
				*/
				if (session.value != NULL && session.uri != NULL) {
					err = M3U8ERR_ATTRIBUTE_UNEXPECTED;
					goto end;
				}
				
				if (session.value == NULL && session.uri == NULL) {
					err = M3U8ERR_ATTRIBUTE_MISSING;
					goto end;
				}
				
				/*
				A Playlist MUST NOT contain more than one EXT-X-SESSION-DATA tag
				with the same DATA-ID attribute and the same LANGUAGE attribute.
				*/
				for (subindex = 0; subindex < stream->playlist.tags.offset; subindex++) {
					const struct M3U8Tag* const a = &stream->playlist.tags.items[subindex];
					
					const char* data_id = NULL;
					const char* language = NULL;
					
					if (a->type != M3U8_TAG_EXT_X_SESSION_DATA) {
						continue;
					}
					
					attribute = m3u8tag_igetattr(a, M3U8_ATTRIBUTE_DATA_ID);
					subattribute = m3u8tag_igetattr(a, M3U8_ATTRIBUTE_LANGUAGE);
					
					if (attribute == NULL || subattribute == NULL) {
						continue;
					}
					
					data_id = attribute->value;
					language = subattribute->value;
					
					for (subsubindex = 0; subsubindex < stream->playlist.tags.offset; subsubindex++) {
						const struct M3U8Tag* const b = &stream->playlist.tags.items[subsubindex];
						
						const char* data_id2 = NULL;
						const char* language2 = NULL;
					
						if (b->type != M3U8_TAG_EXT_X_SESSION_DATA) {
							continue;
						}
						
						if (subsubindex == subindex) {
							continue;
						}
						
						attribute = m3u8tag_igetattr(b, M3U8_ATTRIBUTE_DATA_ID);
						subattribute = m3u8tag_igetattr(b, M3U8_ATTRIBUTE_LANGUAGE);
						
						if (attribute == NULL || subattribute == NULL) {
							continue;
						}
						
						data_id2 = attribute->value;
						language2 = subattribute->value;
					
						if (strcmp(data_id, data_id2) == 0 && strcmp(language, language2) == 0) {
							err = M3U8ERR_TAG_DUPLICATE;
							goto end;
						}
					}
				}
				
				session.tag = tag;
				
				stream_item.type = M3U8_STREAM_SESSION_DATA;
				stream_item.item = malloc(sizeof(session));
				
				if (stream_item.item == NULL) {
					err = M3U8ERR_MEMORY_ALLOCATE_FAILURE;
					goto end;
				}
				
				memcpy(stream_item.item, &session, sizeof(session));
				
				break;
			}
			default: {
				break;
			}
		}
		
		stream->items[stream->offset++] = stream_item;
		
	}
	
	for (index = 0; index < stream->offset; index++) {
		const struct M3U8StreamItem* const item = &stream->items[index];
		struct M3U8VariantStream* const variant_stream = item->item;
		
		if (item->type != M3U8_STREAM_VARIANT_STREAM) {
			continue;
		}
		
		/*
		AUDIO: This attribute is OPTIONAL.
		*/
		attribute = m3u8tag_igetattr(variant_stream->tag, M3U8_ATTRIBUTE_AUDIO);
		
		if (attribute != NULL) {
			/*
			If a Variant Stream has an AUDIO attribute, then there must be a matching
			audio stream within the media section (#EXT-X-MEDIA).
			*/
			for (subindex = 0; subindex < stream->offset; subindex++) {
				const struct M3U8StreamItem* const item = &stream->items[subindex];
				struct M3U8Media* const media = item->item;
				
				if (!(item->type == M3U8_STREAM_MEDIA && media->type == M3U8_MEDIA_TYPE_AUDIO)) {
					continue;
				}
				
				if (strcmp(attribute->value, media->group_id) != 0) {
					continue;
				}
				
				variant_stream->audio = media;
				
				break;
			}
			
			if (variant_stream->audio == NULL) {
				err = M3U8ERR_MEDIA_NO_MATCHING_AUDIO;
				goto end;
			}
		}
		
		/*
		VIDEO: This attribute is OPTIONAL.
		*/
		attribute = m3u8tag_igetattr(variant_stream->tag, M3U8_ATTRIBUTE_VIDEO);
		
		if (attribute != NULL) {
			/*
			If a Variant Stream has a VIDEO attribute, then there must be a matching
			video stream within the media section (#EXT-X-MEDIA).
			*/
			for (subindex = 0; subindex < stream->offset; subindex++) {
				const struct M3U8StreamItem* const item = &stream->items[subindex];
				struct M3U8Media* const media = item->item;
				
				if (!(item->type == M3U8_STREAM_MEDIA && media->type == M3U8_MEDIA_TYPE_VIDEO)) {
					continue;
				}
				
				if (strcmp(attribute->value, media->group_id) != 0) {
					continue;
				}
				
				variant_stream->video = media;
				
				break;
			}
			
			if (variant_stream->video == NULL) {
				err = M3U8ERR_MEDIA_NO_MATCHING_VIDEO;
				goto end;
			}
		}
		
		/*
		SUBTITLES: This attribute is OPTIONAL.
		*/
		attribute = m3u8tag_igetattr(variant_stream->tag, M3U8_ATTRIBUTE_SUBTITLES);
		
		if (attribute != NULL) {
			/*
			If a Variant Stream has a SUBTITLES attribute, then there must be a matching
			subtitles stream within the media section (#EXT-X-MEDIA).
			*/
			for (subindex = 0; subindex < stream->offset; subindex++) {
				const struct M3U8StreamItem* const item = &stream->items[subindex];
				struct M3U8Media* const media = item->item;
				
				if (!(item->type == M3U8_STREAM_MEDIA && media->type == M3U8_MEDIA_TYPE_SUBTITLES)) {
					continue;
				}
				
				if (strcmp(attribute->value, media->group_id) != 0) {
					continue;
				}
				
				variant_stream->subtitles = media;
				
				break;
			}
			
			if (variant_stream->subtitles == NULL) {
				err = M3U8ERR_MEDIA_NO_MATCHING_SUBTITLES;
				goto end;
			}
		}
		
		/*
		CLOSED-CAPTIONS: This attribute is OPTIONAL.
		*/
		attribute = m3u8tag_igetattr(variant_stream->tag, M3U8_ATTRIBUTE_CLOSED_CAPTIONS);
		
		if (attribute != NULL) {
			switch (attribute->vtype) {
				case M3U8_VATTR_TYPE_QSTRING: {
					/*
					If a Variant Stream has a CLOSED-CAPTIONS attribute, then there must be a matching
					closed-captions stream within the media section (#EXT-X-MEDIA).
					*/
					for (subindex = 0; subindex < stream->offset; subindex++) {
						const struct M3U8StreamItem* const item = &stream->items[subindex];
						struct M3U8Media* const media = item->item;
						
						if (!(item->type == M3U8_STREAM_MEDIA && media->type == M3U8_MEDIA_TYPE_CLOSED_CAPTIONS)) {
							continue;
						}
						
						if (strcmp(attribute->value, media->group_id) != 0) {
							continue;
						}
						
						variant_stream->closed_captions = media;
						
						break;
					}
					
					if (variant_stream->closed_captions == NULL) {
						err = M3U8ERR_MEDIA_NO_MATCHING_CLOSED_CAPTIONS;
						goto end;
					}
					
					break;
				}
				case M3U8_VATTR_TYPE_ESTRING: {
					if (strcmp(attribute->value, M3U8K_NONE) != 0) {
						err = M3U8ERR_ATTRIBUTE_INVALID_ESTRING;
						goto end;
					}
					
					/*
					If a Variant Stream sets its CLOSED-CAPTIONS attribute to NONE, then
					all other variant streams must also have this same value.
					*/
					for (subindex = 0; subindex < stream->playlist.tags.offset; subindex++) {
						const struct M3U8Tag* const subtag = &stream->playlist.tags.items[index];
						
						if (subtag->type != M3U8_TAG_EXT_X_STREAM_INF) {
							continue;
						}
						
						attribute = m3u8tag_igetattr(subtag, M3U8_ATTRIBUTE_CLOSED_CAPTIONS);
						
						if (attribute == NULL) {
							err = M3U8ERR_ATTRIBUTE_MISSING;
							goto end;
						}
						
						if (strcmp(attribute->value, M3U8K_NONE) != 0) {
							err = M3U8ERR_MEDIA_UNEXPECTED_CC;
							goto end;
						}
					}
					
					break;
				}
				default: {
					break;
				}
			}
		}
	}
	
	/*
	Sort variant streams.
	*/
	qsort(stream->items, stream->offset, sizeof(*stream->items), m3u8vs_compare);
	
	stream->livestream = stream->playlist.livestream;
	
	end:;
	
	if (err != M3U8ERR_SUCCESS) {
		m3u8stream_free(stream);
	}
	
	return err;
	
}

void m3u8stream_free(struct M3U8Stream* const stream) {
	
	size_t index = 0;
	
	for (index = 0; index < stream->offset; index++) {
		struct M3U8StreamItem* const item = &stream->items[index];
		
		switch (item->type) {
			case M3U8_STREAM_SEGMENT: {
				struct M3U8Key* key = &((struct M3U8Segment*) item->item)->key;
				
				free(key->keyformatversions.items);
				key->keyformatversions.items = NULL;
				
				key->keyformatversions.offset = 0;
				key->keyformatversions.size = 0;
				
				break;
			}
			case M3U8_STREAM_VARIANT_STREAM:
			case M3U8_STREAM_MEDIA: {
				struct M3U8Stream* substream = (
					item->type == M3U8_STREAM_VARIANT_STREAM ?
						&((struct M3U8VariantStream*) item->item)->stream :
						&((struct M3U8Media*) item->item)->stream
				);
				
				substream->playlist.client.curl = NULL;
				substream->playlist.client.error.code = CURLE_OK;
				substream->playlist.client.error.message = NULL;
				
				substream->playlist.multi_client.curl_multi = NULL;
				
				m3u8stream_free(substream);
				
				break;
			}
			default: {
				break;
			}
		}
		
		item->type = M3U8_STREAM_UNKNOWN;
		
		free(item->item);
		item->item = NULL;
	}
	
	free(stream->items);
	stream->items = NULL;
	
	stream->offset = 0;
	stream->size = 0;
	
	stream->livestream = 0;
	
	m3u8playlist_free(&stream->playlist);
	
}

size_t m3u8stream_getvarsize(struct M3U8Stream* const stream) {
	
	size_t index = 0;
	
	size_t size = 0;
	
	for (index = 0; index < stream->offset; index++) {
		struct M3U8StreamItem* const item = &stream->items[index];
		struct M3U8VariantStream* const variant_stream = item->item;
		
		if (item->type != M3U8_STREAM_VARIANT_STREAM) {
			continue;
		}
		
		if (variant_stream->tag->type != M3U8_TAG_EXT_X_STREAM_INF) {
			continue;
		}
		
		size += 1;
	}
	
	return size;
	
}

struct M3U8VariantStream* m3u8stream_getvariant(
	struct M3U8Stream* const stream,
	const enum M3U8SelectStreamCriteria criteria,
	const ssize_t selection
) {
	
	size_t index = 0;
	
	size_t stream_index = 0;
	const size_t size = m3u8stream_getvarsize(stream);
	
	for (index = 0; index < stream->offset; index++) {
		struct M3U8StreamItem* const item = &stream->items[index];
		struct M3U8VariantStream* const variant_stream = item->item;
		
		if (item->type != M3U8_STREAM_VARIANT_STREAM) {
			continue;
		}
		
		if (variant_stream->tag->type != M3U8_TAG_EXT_X_STREAM_INF) {
			continue;
		}
		
		switch (criteria) {
			case M3U8_SELECT_STREAM_BY_POSITION: {
				const ssize_t position = selection;
				
				if ((position < 0 && position == (ssize_t) (-size + stream_index)) || ((size_t) position == stream_index)) {
					return variant_stream;
				}
				
				break;
			}
			case M3U8_SELECT_STREAM_BY_RESOLUTION: {
				const biguint_t resolution = selection;
				
				if (resolution == variant_stream->resolution.width || resolution == variant_stream->resolution.height) {
					return variant_stream;
				}
				
				break;
			}
		}
		
		stream_index += 1;
	}
	
	return NULL;
	
}

int m3u8stream_load_buffer(
	struct M3U8Stream* const stream,
	const char* const buffer
) {
	
	int err = 0;
	
	err = m3u8playlist_load_buffer(&stream->playlist, buffer);
	
	if (err != M3U8ERR_SUCCESS) {
		return err;
	}
	
	err = m3u8stream_parse(stream);
	return err;
	
}

int m3u8stream_load_file(
	struct M3U8Stream* const stream,
	const char* const filename,
	const char* const base_uri
) {
	
	int err = 0;
	
	err = m3u8playlist_load_file(&stream->playlist, filename, base_uri);
	
	if (err != M3U8ERR_SUCCESS) {
		return err;
	}
	
	err = m3u8stream_parse(stream);
	
	return err;
	
}

int m3u8stream_load_url(
	struct M3U8Stream* const stream,
	const char* const url,
	const char* const base_uri
) {
	
	int err = 0;
	
	err = m3u8playlist_load_url(&stream->playlist, url, base_uri);
	
	if (err != M3U8ERR_SUCCESS) {
		return err;
	}
	
	err = m3u8stream_parse(stream);
	
	return err;
	
}

int m3u8stream_load(
	struct M3U8Stream* const stream,
	const char* const something,
	const char* const base_uri
) {
	
	int err = 0;
	
	err = m3u8playlist_load(&stream->playlist, something, base_uri);
	
	if (err != M3U8ERR_SUCCESS) {
		return err;
	}
	
	err = m3u8stream_parse(stream);
	
	return err;
	
}

int m3u8stream_load_subresource(
	const struct M3U8Stream* const root,
	struct M3U8Stream* const resource,
	const char* const something
) {
	
	int err = 0;
	
	err = m3u8playlist_load_subresource(&root->playlist, &resource->playlist, something);
	
	if (err != M3U8ERR_SUCCESS) {
		return err;
	}
	
	err = m3u8stream_parse(resource);
	
	return err;
	
}
