import enum

from .base import Dict, List

class M3U8MediaType(enum.Enum):
	M3U8_MEDIA_TYPE_UNKNOWN = 0x00000000
	M3U8_MEDIA_TYPE_VIDEO = enum.auto()
	M3U8_MEDIA_TYPE_AUDIO = enum.auto()
	M3U8_MEDIA_TYPE_SUBTITLES = enum.auto()
	M3U8_MEDIA_TYPE_CLOSED_CAPTIONS = enum.auto()

class M3U8ClosedCaption(enum.Enum):
	M3U8_CLOSED_CAPTION_UNKNOWN = 0x00000000
	M3U8_CLOSED_CAPTION_CC1 = enum.auto()
	M3U8_CLOSED_CAPTION_CC2 = enum.auto()
	M3U8_CLOSED_CAPTION_CC3 = enum.auto()
	M3U8_CLOSED_CAPTION_CC4 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE1 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE2 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE3 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE4 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE5 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE6 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE7 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE8 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE9 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE10 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE11 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE12 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE13 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE14 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE15 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE16 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE17 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE18 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE19 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE20 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE21 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE22 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE23 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE24 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE25 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE26 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE27 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE28 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE29 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE30 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE31 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE32 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE33 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE34 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE35 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE36 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE37 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE38 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE39 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE40 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE41 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE42 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE43 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE44 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE45 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE46 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE47 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE48 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE49 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE50 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE51 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE52 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE53 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE54 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE55 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE56 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE57 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE58 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE59 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE60 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE61 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE62 = enum.auto()
	M3U8_CLOSED_CAPTION_SERVICE63 = enum.auto()

class M3U8HDCPLevel(enum.Enum):
	M3U8_ENCRYPTION_METHOD_UNKNOWN = 0x00000000
	M3U8_HDCP_LEVEL_NONE = enum.auto()
	M3U8_HDCP_LEVEL_TYPE_0 = enum.auto()

class M3U8EncryptionMethod(enum.Enum):
	M3U8_ENCRYPTION_METHOD_UNKNOWN = 0x00000000
	M3U8_ENCRYPTION_METHOD_NONE = enum.auto()
	M3U8_ENCRYPTION_METHOD_AES_128 = enum.auto()
	M3U8_ENCRYPTION_METHOD_SAMPLE_AES = enum.auto()

class M3U8MediaPlaylistType(enum.Enum):
	M3U8_MEDIA_PLAYLIST_TYPE_UNKNOWN = 0x00000000
	M3U8_MEDIA_PLAYLIST_TYPE_EVENT = enum.auto()
	M3U8_MEDIA_PLAYLIST_TYPE_VOD = enum.auto()

class M3U8StreamItemType(enum.Enum):
	M3U8_STREAM_UNKNOWN = 0x00000000
	M3U8_STREAM_VERSION = enum.auto()
	M3U8_STREAM_KEY = enum.auto()
	M3U8_STREAM_SEGMENT = enum.auto()
	M3U8_STREAM_MAP = enum.auto()
	M3U8_STREAM_ALLOW_CACHE = enum.auto()
	M3U8_STREAM_DATE_RANGE = enum.auto()
	M3U8_STREAM_TARGET_DURATION = enum.auto()
	M3U8_STREAM_MEDIA_SEQUENCE = enum.auto()
	M3U8_STREAM_DISCONTINUITY_SEQUENCE = enum.auto()
	M3U8_STREAM_PLAYLIST_TYPE = enum.auto()
	M3U8_STREAM_SESSION_DATA = enum.auto()
	M3U8_STREAM_START = enum.auto()
	M3U8_STREAM_MEDIA = enum.auto()
	M3U8_STREAM_VARIANT_STREAM = enum.auto()
	M3U8_STREAM_PROGRAM_DATE_TIME = enum.auto()
	M3U8_STREAM_END_LIST = enum.auto()
	M3U8_STREAM_IFRAMES_ONLY = enum.auto()
	M3U8_STREAM_DISCONTINUITY = enum.auto()
	M3U8_STREAM_INDEPENDENT_SEGMENTS = enum.auto()

class M3U8StreamItem(Dict):
	
	def __init__(self, type, item):
		self.type = type
		self.item = item

class M3U8Version(Dict):
	
	def __init__(self, version, tag):
		self.version = version
		self.tag = tag

class M3U8StreamPlaylistType(Dict):
	
	def __init__(self, type, tag):
		self.type = type
		self.tag = tag

class M3U8KeyFormatVersions(List):
	pass

class M3U8Key(Dict):
	
	def __init__(self, method, uri, iv, keyformat, keyformatversions, tag):
		self.method = method
		self.uri = uri
		self.iv = iv
		self.keyformat = keyformat
		self.keyformatversions = keyformatversions
		self.tag = tag

class M3U8Segment(Dict):
	
	def __init__(self, key, duration, title, byterange, bitrate, uri, tag):
		self.key = key
		self.duration = duration
		self.title = title
		self.byterange = byterange
		self.bitrate = bitrate
		self.uri = uri
		self.tag = tag

class M3U8Segments(List):
	pass

class M3U8Map(Dict):
	
	def __init__(self, uri, byterange, tag):
		self.uri = uri
		self.byterange = byterange
		self.tag = tag

class M3U8AllowCache(Dict):
	
	def __init__(self, allow_cache, tag):
		self.allow_cache = allow_cache
		self.tag = tag

class M3U8DateRange(Dict):
	
	def __init__(self, id, _class, start_date, end_date, duration, planned_duration, end_on_next, tag):
		self.id = id
		self._class = _class
		self.start_date = start_date
		self.end_date = end_date
		self.duration = duration
		self.planned_duration = planned_duration
		self.end_on_next = end_on_next
		self.tag = tag

class M3U8TargetDuration(Dict):
	
	def __init__(self, duration, tag):
		self.duration = duration
		self.tag = tag

class M3U8MediaSequence(Dict):
	
	def __init__(self, number, tag):
		self.number = number
		self.tag = tag

class M3U8DiscontinuitySequence(Dict):
	
	def __init__(self, number, tag):
		self.number = number
		self.tag = tag

class M3U8SessionData(Dict):
	
	def __init__(self, data_id, value, uri, language, tag):
		self.data_id = data_id
		self.value = value
		self.uri = uri
		self.language = language
		self.tag = tag

class M3U8Start(Dict):
	
	def __init__(self, time_offset, precise, tag):
		self.time_offset = time_offset
		self.precise = precise
		self.tag = tag

class M3U8Media(Dict):
	
	def __init__(
		self,
		type,
		uri,
		group_id,
		language,
		assoc_language,
		name,
		default,
		autoselect,
		forced,
		instream_id,
		characteristics,
		channels,
		stream,
		duration,
		average_duration,
		segments,
		tag
	):
		self.type = type
		self.uri = uri
		self.group_id = group_id
		self.language = language
		self.assoc_language = assoc_language
		self.name = name
		self.default = default
		self.autoselect = autoselect
		self.forced = forced
		self.instream_id = instream_id
		self.characteristics = characteristics
		self.channels = channels
		self.stream = stream
		self.duration = duration
		self.average_duration = average_duration
		self.segments = segments
		self.tag = tag

class M3U8VariantStream(Dict):
	
	def __init__(
		self,
		bandwidth,
		average_bandwidth,
		program_id,
		codecs,
		resolution,
		frame_rate,
		hdcp_level,
		audio,
		video,
		subtitles,
		closed_captions,
		uri,
		stream,
		duration,
		average_duration,
		size,
		segments,
		tag
	):
		self.bandwidth = bandwidth
		self.average_bandwidth = average_bandwidth
		self.program_id = program_id
		self.codecs = codecs
		self.resolution = resolution
		self.frame_rate = frame_rate
		self.hdcp_level = hdcp_level
		self.audio = audio
		self.video = video
		self.subtitles = subtitles
		self.closed_captions = closed_captions
		self.uri = uri
		self.stream = stream
		self.duration = duration
		self.average_duration = average_duration
		self.size = size
		self.segments = segments
		self.tag = tag
