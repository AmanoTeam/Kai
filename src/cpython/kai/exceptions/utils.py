from .mapping import (
	ERROR_MESSAGE_MAPPING,
	ERROR_CODE_MAPPING
)

def code2exception(code):
	message = ERROR_MESSAGE_MAPPING[code]
	exception = None
	
	for (key, value) in ERROR_CODE_MAPPING.items():
		if code not in value:
			continue
		
		exception = key
		
		break
	
	if exception is None:
		raise RuntimeError("Cannot map error code to exception")
	
	return exception(code = code, message = message)
