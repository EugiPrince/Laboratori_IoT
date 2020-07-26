import time

class Service():

	def __init__ (self, content):

		self.service_id = content['service_id']
		self.description = content['description']
		self.endpoints = content['endpoints']
		self.timestamp = int(time.time())

