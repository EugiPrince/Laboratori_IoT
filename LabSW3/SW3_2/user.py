class User():

	def __init__(self, content):

		self.user_id = content["user_id"]
		self.name = content["name"]
		self.surname = content["surname"]
		self.email = content["email"]
