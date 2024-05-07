from .base import Dict

class M3U8Resolution(Dict):
	
	def __init__(self, width, height):
		self.width = width
		self.height = height

class M3U8ByteRange(Dict):
	
	def __init__(self, length, offset):
		self.length = length
		self.offset = offset
