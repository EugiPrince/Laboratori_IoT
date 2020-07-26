import paho.mqtt.client as PahoMQTT
import json


class myMQTTSubscriber():

    def __init__(self, clientID, topic, broker, catalog):
        self.catalog = catalog
        self.clientID = clientID
        # create an instance of paho.mqtt.client
        self._paho_mqtt = PahoMQTT.Client(clientID, False)

        # register the callback
        self._paho_mqtt.on_connect = self.myOnConnect
        self._paho_mqtt.on_message = self.myOnMessageReceived
        # l'ip del catalog corrisponde al broker utilizzato per MQTT
        self.messageBroker = catalog.ip
        self.port = catalog.port
        # recupera i topic del device arduino precedentemente registrato, contenuti in un vettore dichiarato
        # al momento della registrazione
        self.topic = []
        for var in catalog.getDevice('Yun').get('endpoints'):
            self.topic.append(var)

        self.temperature = 0

        #registrazione come nuovo servizio del catalog
        service_content = {
            "service_id": "MQTT_temp_subscriber",
            "description": "temperature subscriber mqtt service",
            "endpoints": "/tiot/9/temperature"
        }
        self.catalog.addService(service_content)


    def start(self):
        # manage connection to broker
        self._paho_mqtt.connect(self.messageBroker, 1883)
        # subscribe ai topic del device
        self._paho_mqtt.loop_start()
        for var in self.topic:
            self._paho_mqtt.subscribe(var)
            print(var)


    def stop(self):
        self._paho_mqtt.unsubscribe(self.topic)
        self._paho_mqtt.loop_stop()
        self._paho_mqtt.disconnect()

    def myOnConnect(self, paho_mqtt, userdata, flags, rc):
        print("Connected to %s with result code: %d" % (self.messageBroker, rc))

    def myOnMessageReceived(self, paho_mqtt, userdata, msg):
        # A new message is received
        #print('received', paho_mqtt, userdata, msg)
        print("ricevuto")
        device = json.loads(msg.payload)
        print(device)
        self.catalog.addDevice(device)
