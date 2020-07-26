from device import Device
from user import User
from service import Service
import time
import threading


class Catalog():

    def __init__(self, ip, port):
        self.devices = {}
        self.users = {}
        self.services = {}
        self.ip = ip
        self.port = port
        self.info = {"ip": self.ip, "port": self.port}
        threading.Thread(target=self.__refreshDevices, daemon=True).start()

    def addDevice(self, content):
        device = Device(content)
        self.devices[content['device_id']] = device
        return device.__dict__

    # def updateDevice(self, device_id, content):
    #    content.timestamp = int(time.time())
    #    self.devices[device_id] = content

    def getDevice(self, device_id):
        return self.devices.get(device_id).__dict__ if self.devices.get(device_id) else None

    def getDevices(self):
        return [d.__dict__ for d in self.devices.values()]

    def addUser(self, content):
        user = User(content)
        self.users[content['user_id']] = user
        return user.__dict__

    def getUser(self, user_id):
        return self.users.get(user_id).__dict__ if self.users.get(user_id) else None

    def getUsers(self):
        return [u.__dict__ for u in self.users.values()]

    def addService(self, content):
        service = Service(content)
        self.services[content['service_id']] = service
        return service.__dict__

    def getService(self, service_id):
        return self.services.get(service_id).__dict__ if self.services.get(service_id) else None

    def getServices(self):
        return [s.__dict__ for s in self.services.values()]

    def getInfo(self):
        return self.info

    def __refreshDevices(self):
        while True:
            self.clearOldDevices()
            time.sleep(60)

    def clearOldDevices(self):
        min_time = int(time.time()) - 120
        for k in list(self.devices.keys()):
            if (self.devices[k]).timestamp < min_time:
                del self.devices[k]
                print('Device %s eliminato.' % k)
