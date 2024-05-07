import sys
sys.path.append('/storage/emulated/0/Kai/src/cpython')

from _kai import (
	m3u8stream_init,
	m3u8stream_load,
	m3u8stream_load_url,
	m3u8stream_load_file,
	m3u8stream_getstream
)

from kai.types import *


from exceptions import code2exception, KaiErrorCode

class M3U8Stream(List):
	
	def __init__(self):
		self.instance = m3u8stream_init()
	
	def _fetch_objects(self):
		self.base_list = m3u8stream_getstream(self.instance)
	
	def ensure_non_error(self):
		code = KaiErrorCode(self.code)
		
		if code != KaiErrorCode.M3U8ERR_SUCCESS:
			exception = code2exception(code = self.code)
			raise exception
		
		return None
	
	def load(self, something, base_url = None):
		self.code = m3u8stream_load(
			self.instance,
			something,
			base_url
		)
		
		self.ensure_non_error()
		self._fetch_objects()
		
		return self
	
	def load_url(self, url, base_url = None):
		self.code = m3u8stream_load_url(
			self.instance,
			url,
			base_url
		)
		
		self.ensure_non_error()
		self._fetch_objects()
		
		return self
	
	def load_file(self, filename, base_url = None):
		self.code = m3u8stream_load_file(
			self.instance,
			filename,
			base_url
		)
		
		self.ensure_non_error()
		self._fetch_objects()
		
		return self

stream = M3U8Stream()
a=stream.load("/storage/emulated/0/cq3l8ci23aks73akgsug/master.m3u8")
import sys
"""
for item in a.stream.iter():
	pass
	#print(repr(item))

print(sys.getrefcount(a.stream))
"""
print(a)