import threading
import time

import paho.mqtt.client as PahoMQTT
import requests

class myMQTT():

    def __init__(self, clientID, catalog):
        self.catalog = catalog
        self.clientID = clientID

        self._paho_mqtt = PahoMQTT.Client(clientID, False)

        self._paho_mqtt.on_connect = self.myOnConnect
        self._paho_mqtt.on_message = self.myOnMessageReceived
        self._paho_mqtt.on_publish = self.myPublish
        self.messageBroker = catalog.ip
        self.port = catalog.port


        self.topic = []
        for var in catalog.getDevice('Yun').get('endpoints'):
            self.topic.append(var)

        service_content = {
            "service_id": "MQTT_RemoteSmartHome",
            "description": "MQTT Publisher and Subscriber to remote control sensors",
            "endpoints": ["/tiot/9/temperature", "/tiot/9/presence", "/tiot/9/sound"]
        }

        # ora, potendo accedere ai dati del device, aggiungo il nuovo servizio
        self.catalog.addService(service_content)
        #d = Client(service_content, "http://localhost:8080/addService")

    def myOnMessageReceived(self, paho_mqtt, userdata, msg):
        print("Topic:'" + msg.topic + "', QoS: '" + str(msg.qos) + "' Message: '" + str(msg.payload) + "'")

    # pubblicazione modifica dei setpoint di temperatura
    def myPublish(self, topic, message):
        print("publishing '%s' with topic '%s'" % (message, topic))
        self._paho_mqtt.publish(topic, message, 2)

    def start(self):
        self._paho_mqtt.connect("test.mosquitto.org", 1883)
        print("connected?")
        for var in self.topic:
            self._paho_mqtt.subscribe(var)
            print(var)
        # sub ai topic dichiarati nel device come endpoint
        self._paho_mqtt.loop_start()

    def stop(self):
        self._paho_mqtt.unsubscribe(self.topic)
        self._paho_mqtt.loop_stop()
        self._paho_mqtt.disconnect()

    #myOnConnect standard
    def myOnConnect (self, paho_mqtt, userdata, flags, rc):
        print("dai")
        print ("Connected to %s with result code: %d" % (self.messageBroker, rc))
