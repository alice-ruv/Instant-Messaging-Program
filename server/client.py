class Client:
    def __init__(self, client_id, user_name, public_key, last_seen):
        self.client_id = client_id
        self.user_name = user_name
        self.public_key = public_key
        self.last_seen = last_seen

    def __str__(self):
        return f"Client id:{self.client_id}, name: {self.user_name}, public key: {self.public_key}, last seen: {self.last_seen}"
