import cherrypy
import json

class ArduinoTemperature(object):
    exposed = True

    def __init__(self):
        self.list = list()
        self.obj = {"all":[]}


    def POST(self, *uri, **params):
        data = cherrypy.request.body.read()
        json_data = json.loads(data.decode('utf-8'))
        self.list.append(json_data)
        
        self.obj['all'].append(json_data)

    def GET(self, *uri, **params):
        res = json.dumps(self.obj)
        return res

if __name__=="__main__":
	#Standard configuration to serve the url "localhost:8080"
        conf={
            '/':{
                    'request.dispatch':cherrypy.dispatch.MethodDispatcher(),
                    'tool.session.on':True
            }
        }

       	cherrypy.tree.mount(ArduinoTemperature(), '/log', conf)
        cherrypy.config.update({'server.socket_host': '0.0.0.0'})
        cherrypy.config.update({'server.socket_port': 8080})
        cherrypy.engine.start()
        cherrypy.engine.block()