from .exceptions import *
from .types import *

__all__ = [
	"KaiError",
	"M3U8AttributeError",
	"MemoryError",
	"CallbackError",
	"CLIError",
	"cURLError",
	"DownloadError",
	"FilesystemError",
	"FFmpegError",
	"M3U8ItemError",
	"UnsupportedURIError",
	"M3U8MediaError",
	"M3U8ParserError",
	"M3U8PlaylistError",
	"PrintfError",
	"M3U8TagError",
	"code2exception",
	"KaiErrorCode",
	"M3U8MediaType",
	"M3U8ClosedCaption",
	"M3U8HDCPLevel",
	"M3U8EncryptionMethod",
	"M3U8MediaPlaylistType",
	"M3U8StreamItemType",
	"M3U8StreamItem",
	"M3U8Version",
	"M3U8StreamPlaylistType",
	"M3U8KeyFormatVersions",
	"M3U8Key",
	"M3U8Segment",
	"M3U8Segments",
	"M3U8Map",
	"M3U8AllowCache",
	"M3U8DateRange",
	"M3U8TargetDuration",
	"M3U8MediaSequence",
	"M3U8DiscontinuitySequence",
	"M3U8SessionData",
	"M3U8Start",
	"M3U8Media",
	"M3U8VariantStream",
	"M3U8Resolution",
	"M3U8ByteRange",
	"Dict",
	"List"
]

__locals = locals()

for __name in __all__:
	if __name.startswith("__"):
		continue
	
	try:
		setattr(__locals[__name], "__module__", "kai")
	except AttributeError:
		pass
