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
	return round((x - 32) * 5/9 + 273.15)

def KtoF(x):
	return round((x -273.15) * 9/5 + 32)

class tempConverter(object):
	
	exposed = True	

	def GET (self, *uri, **params):

		if (len(params.keys()) != 3):
			return "Il numero di parametri non è corretto. Inserire 3 parametri"
		
		x = 'originalUnit'
		if(params[x] != 'C' and params[x] != 'K' and params[x] != 'F'):
			return "Unità di misura errata"

		x = 'targetUnit'
		if(params[x] != 'C' and params[x] != 'K' and params[x] != 'F'):
			return "Unità di misura errata"

		newValue = 0
		
		values = []
		for x in params.keys():
			values.append(params[x])


		if(values[1] == 'C'):
			if(values[2] == 'K'):
				newValue = CtoK(int(values[0]))
			else:
				newValue = CtoF(int(values[0]))


		if(values[1] == 'K'):
			if(values[2] == 'C'):
				newValue = KtoC(int(values[0]))
			else:
				newValue = KtoF(int(values[0]))


		if(values[1] == 'F'):
			if(values[2] == 'C'):
				newValue = FtoC(int(values[0]))
			else:
				newValue = FtoK(int(values[0]))

		conversion = { values[0] : values[1], str(newValue) : values[2] }
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

	