import requests

class Client:

    def getBrokerInfo(self):
        r = requests.get('http://localhost:8080/getInfo')
        info = r.json()
        return info

    def getDevices(self):
        r = requests.get('http://localhost:8080/getDevices')
        devices = r.json()
        return devices

    def getDevice(self, device_id):
        r = requests.get('http://localhost:8080/getDevice/' + device_id)
        device = r.json()
        return device

    def getUsers(self):
        r = requests.get('http://localhost:8080/getUsers')
        users = r.json()
        return users

    def getUser(self, user_id):
        r = requests.get('http://localhost:8080/getUser/' + user_id)
        device = r.json()
        return device


if __name__ == '__main__':
    c = Client()
    brokerInfo = c.getBrokerInfo()
    print(brokerInfo)

    devices = c.getDevices()
    print(devices)

    device = c.getDevice('sensoreTemperatura')
    print(device)

    users = c.getUsers()
    print(users)

    user = c.getUser('1')
    print(user)
