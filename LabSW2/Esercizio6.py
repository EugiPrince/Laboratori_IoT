import paho.mqtt.client as PahoMQTT
import json
import time


class myPublisher():

    def __init__(self):
        # create an instance of paho.mqtt.client
        self._paho_mqtt = PahoMQTT.Client()
        # register the callback
        self._paho_mqtt.on_connect = self.myOnConnect
        self._paho_mqtt.on_publish = self.onPublish
        self.messageBroker = 'mqtt.eclipse.org'

    def start(self):
        # manage connection to broker
        self._paho_mqtt.connect(self.messageBroker, 1883, 60)
        self._paho_mqtt.loop_start()

    def stop(self):
        self._paho_mqtt.loop_stop()
        self._paho_mqtt.disconnect()

    def myPublish(self, topic, message):
        # publish a message with a certain topic
        self._paho_mqtt.publish(topic, message)

    def onPublish(mosq, obj, mid):
        print("Comando inviato con message id: " + str(mid))

    def myOnConnect(self, paho_mqtt, userdata, flags, rc):
        print("Connected to %s with result code: %d" % (self.messageBroker, rc))


if __name__ == '__main__':
    infoDevice = {"device_id": "ArduinoYun",
                  "resources": ["sensore temperatura", "sensore pressione"],
                  "endpoints": ["http://192.168.1.2:8080/getTemperature",
                                "http://192.168.1.2:8080/getBar"]}

    p = myPublisher()
    p.start()
    while True:
        p.myPublish('test/topic', json.dumps(infoDevice))
        time.sleep(60)
