class KaiError(Exception):
	
	def __init__(
		self,
		code: int,
		message: str
	):
		self.code = code
		self.message = message
		
		super().__init__(message)
