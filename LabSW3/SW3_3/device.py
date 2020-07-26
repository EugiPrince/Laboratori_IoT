import time

class Device():

    def __init__(self, content):
    	
        self.device_id = content["device_id"]
        self.resources = content["resources"]
        self.endpoints = content["endpoints"]
        self.timestamp = int(time.time())
