class Message:
    def __init__(
        self, message_id, to_client, from_client, message_type, message_content
    ):
        self.message_id = message_id
        self.to_client = to_client
        self.from_client = from_client
        self.message_type = message_type
        self.message_content = message_content

    def __repr__(self):
        return self.__str__()

    def __str__(self):
        return f"Message id: {self.message_id}, type: {self.message_type}, to: {self.to_client}, from: {self.from_client}"
