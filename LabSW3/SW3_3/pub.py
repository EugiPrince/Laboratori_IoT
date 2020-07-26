import paho.mqtt.client as PahoMQTT
import json


class myMQTTpub():

    def __init__(self, clientID, catalog):
        self.catalog = catalog
        self.clientID = clientID
        # create an instance of paho.mqtt.client
        self._paho_mqtt = PahoMQTT.Client(clientID, False)
        self._paho_mqtt.on_connect = self.myOnConnect
        self._paho_mqtt.on_publish = self.myPublish

        self.messageBroker = catalog.ip
        self.port = catalog.port

        service_content = {
            "service_id" : "MQTT_Led_Pub",
            "description" : "MQTT Publisher to send actuation commands to a led managed by the Arduino Yun",
            "endpoints" : "/tiot/9/led"
        }
        self.catalog.addService(service_content)

    def start(self):
        # manage connection to broker
        self._paho_mqtt.connect("test.mosquitto.org", 1883)
        print("started")
        self._paho_mqtt.loop_start()

    def stop (self):
        self._paho_mqtt.loop_stop()
        self._paho_mqtt.disconnect()

    #myOnConnect standard
    def myOnConnect (self, paho_mqtt, userdata, flags, rc):
        print ("Connected to %s with result code: %d" % (self.messageBroker, rc))

    def myPublish(self, topic, message):
        print ("publishing '%s' with topic '%s'" % (message, topic))
        self._paho_mqtt.publish(topic, message, 2)


