import requests
import threading
import time


class Client:

    def __init__(self, info_device, registration_endpoint):
        self.device = info_device
        self.registration_endpoint = registration_endpoint
        threading.Thread(target=self.__refreshRegistration).start()

    def register(self):
        r = requests.post(self.registration_endpoint, json=self.device)

    def __refreshRegistration(self):
        while True:
            self.register()
            time.sleep(60)


if __name__ == '__main__':
    infoDevice = {"device_id": "ArduinoUno",
                  "resources": ["sensore temperatura", "sensore presenza", "sensore pressione"],
                  "endpoints": ["http://192.168.1.1:8080/getTemperature", "http://192.168.1.1:8080/getPresence",
                                "http://192.168.1.1:8080/getBar"]}

    c = Client(infoDevice, 'http://localhost:8080/addDevice')
