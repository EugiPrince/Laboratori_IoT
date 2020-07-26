import cherrypy
from catalog import Catalog
import json

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

if __name__ == "__main__":
    catalog = Catalog('mqtt.eclipse.org', '1883')
    # Standard configuration to serve the url "localhost:8080"
    conf = {
        '/': {
            'request.dispatch': cherrypy.dispatch.MethodDispatcher(),
            'tool.session.on': True
        }
    }

    cherrypy.tree.mount(Rest(catalog), "/", conf)
    cherrypy.config.update({'server.socket_host':'0.0.0.0'})
    cherrypy.config.update({'server.socket_port':8080})
    cherrypy.engine.start()
    cherrypy.engine.block()
