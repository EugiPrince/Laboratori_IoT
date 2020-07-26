import paho.mqtt.client as PahoMQTT
import json


class myMQTT():

    def __init__(self, clientID, topic, broker, catalog):
        self.catalog = catalog
        self.clientID = clientID
        # create an instance of paho.mqtt.client
        self._paho_mqtt = PahoMQTT.Client(clientID, False)

        # register the callback
        self._paho_mqtt.on_connect = self.myOnConnect
        self._paho_mqtt.on_message = self.myOnMessageReceived

        self.topic = topic
        self.messageBroker = broker
        self.led_status = 0

    def start(self):
        # manage connection to broker
        self._paho_mqtt.connect(self.messageBroker, 1883)
        # subscribe for a topic
        self._paho_mqtt.subscribe(self.topic)
        self._paho_mqtt.loop_forever()

    def stop(self):
        self._paho_mqtt.unsubscribe(self.topic)
        self._paho_mqtt.loop_stop()
        self._paho_mqtt.disconnect()

    def myOnConnect(self, paho_mqtt, userdata, flags, rc):
        print("Connected to %s with result code: %d" % (self.messageBroker, rc))

    def myOnMessageReceived(self, paho_mqtt, userdata, msg):
        # A new message is received
        #print('received', paho_mqtt, userdata, msg)
        device = json.loads(msg.payload)
        print(device)
        self.catalog.addDevice(device)
