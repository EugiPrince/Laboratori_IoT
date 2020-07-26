import cherrypy
import json

def CtoK (x):
	return round(x + 273.15,2)

def KtoC(x):
	return round(x - 273.15,2)

def CtoF(x):
	return round(x * 9/5 + 32,2)

def FtoC(x):
	return round((x - 32) * 5/9,2)

def FtoK(x):
	return round((x - 32) * 5/9 + 273.15,2)

def KtoF(x):
	return round((x -273.15) * 9/5 + 32,2)

class tempConverter(object):
	
	exposed = True	

	def GET (self, *uri, **params):

		if (len(uri) != 3):
			return "Il numero di parametri non è corretto"
		
		if(uri[1] != 'C' and uri[1] != 'K' and uri[1] != 'F'):
			return "Unità di misura errata"

		if(uri[2] != 'C' and uri[2] != 'K' and uri[2] != 'F'):
			return "Unità di misura errata"

		if(uri[1] == 'C'):
			if(uri[2] == 'K'):
				newValue = CtoK(int(uri[0]))
			else:
				newValue = CtoF(int(uri[0]))


		if(uri[1] == 'K'):
			if(uri[2] == 'C'):
				newValue = KtoC(int(uri[0]))
			else:
				newValue = KtoF(int(uri[0]))


		if(uri[1] == 'F'):
			if(uri[2] == 'C'):
				newValue = FtoC(int(uri[0]))
			else:
				newValue = FtoK(int(uri[0]))

		conversion = { uri[0] : uri[1], str(newValue) : uri[2] }
		return json.dumps(conversion)

if __name__=="__main__":
	#Standard configuration to serve the url "localhost:8080"
	conf={
		'/':{
				'request.dispatch':cherrypy.dispatch.MethodDispatcher(),
				'tool.session.on':True
		}
	}
	cherrypy.quickstart(tempConverter(),'/converter',conf)

