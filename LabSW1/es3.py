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
	return round((x - 32) * 5/9 + 273.15, 2)

def KtoF(x):
	return round((x -273.15) * 9/5 + 32, 2)

class tempConverter(object):
	
	exposed = True

	def PUT(self, **params):
		data = cherrypy.request.body.read()
		json_data = json.loads(data.decode('utf-8'))

		errore = "Unita' di misura errata"

		x = 'originalUnit'
		if(json_data[x] != 'C' and json_data[x] != 'K' and json_data[x] != 'F'):
			return errore

		y = 'targetUnit'
		if(json_data[y] != 'C' and json_data[y] != 'K' and json_data[y] != 'F'):
			return errore

		newValues = []

		if(json_data[x] == 'C'):
			if(json_data[y] == 'K'):
				for j in json_data["values"]:
					newValues.append(CtoK(int(j)))
			else:
				for j in json_data["values"]:
					newValues.append(CtoF(int(j)))


		if(json_data[x] == 'K'):
			if(json_data[y] == 'C'):
				for j in json_data["values"]:
					newValues.append(KtoC(int(j)))
			else:
				for j in json_data["values"]:
					newValues.append(KtoF(int(j)))


		if(json_data[x] == 'F'):
			if(json_data[y] == 'C'):
				for j in json_data["values"]:
					newValues.append(FtoC(int(j)))
			else:
				for j in json_data["values"]:
					newValues.append(FtoK(int(j)))

		conversion = { json_data[x] : json_data['values'], json_data[y] : newValues }
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

