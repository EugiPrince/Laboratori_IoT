import json
from catalog import Catalog
from device import Device
from pub import myMQTTpub
from service import Service
from user import User
import cherrypy
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


@cherrypy.tools.json_out()
class Rest(object):
    exposed = True

    def __init__(self, catalog):
        self.catalog = catalog
        return

    @cherrypy.expose
    def GET(self, *uri, **params):
        if len(uri) == 1:
            if uri[0] == 'getInfo':
                r = self.catalog.getInfo()
            elif uri[0] == 'getDevices':
                r = self.catalog.getDevices()
            elif uri[0] == 'getUsers':
                r = self.catalog.getUsers()
            elif uri[0] == 'getServices':
                r = self.catalog.getServices()
            else:
                raise cherrypy.HTTPError(404, "unknown URI.")
        elif len(uri) == 2:
            if uri[0] == 'getDevice':
                r = self.catalog.getDevice(uri[1])
            elif uri[0] == 'getUser':
                r = self.catalog.getUser(uri[1])
            elif uri[0] == 'getService':
                r = self.catalog.getService(uri[1])
            else:
                raise cherrypy.HTTPError(404, "unknown URI.")

            if r is None:
                cherrypy.response.status = 404
        else:
            raise cherrypy.HTTPError(404, "unknown URI.")
        if r == "":
            raise cherrypy.HTTPError(404, "Can't satisfy the request.")
        else:
            return r

    @cherrypy.expose
    def POST(self, *uri, **params):
        content = json.loads(cherrypy.request.body.read())
        if uri[0] == 'addDevice':
            r = self.catalog.addDevice(content)
        elif uri[0] == 'addUser':
            r = self.catalog.addUser(content)
        elif uri[0] == 'addService':
            r = self.catalog.addService(content)
        elif uri[0] == 'refreshTemperature':
            r = self.catalog.refreshTemperature(content)
        else:
            raise cherrypy.HTTPError(404, "URI unknown")

        if r is None:
            cherrypy.response.status = 404
        return r


if __name__ == '__main__':
    catalog = Catalog('test.mosquitto.org', '1883')
    
    conf = {
        '/': {
            'request.dispatch': cherrypy.dispatch.MethodDispatcher(),
            'tool.session.on': True
        }
    }

    infoDevice = {"device_id": "Yun",
                  "resources": ["led"],
                  "endpoints": ["/tiot/9/led"]
                  }

    cherrypy.tree.mount(Rest(catalog), "/", conf)
    cherrypy.config.update({'server.socket_host':'0.0.0.0'})
    cherrypy.config.update({'server.socket_port':8080})

    cherrypy.engine.start()
    c = Client(infoDevice, 'http://127.0.0.1:8080/addDevice')
    
    p = myMQTTpub("Pub_Led", catalog)
    p.start()


    while True:
        print("Inserisci 1 per accendere il Led, 0 per spegnere:")
        val = input()
        val = int(val)

        ledCommands = {
            "bn": "Yun",
            "e": [{"n": "led", "t": 'null', "v": val, "u": 'null'}]
        }
        p.myPublish("/tiot/9/led", json.dumps(ledCommands))
        time.sleep(60)

    cherrypy.engine.block()