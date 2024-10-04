class Request:
    def __init__(self, client_id, client_version):
        self.client_id = client_id
        self.client_version = client_version

    def __str__(self):
        return f"client id: {self.client_id}, client version: {self.client_version}"


class RegisterRequest(Request):
    def __init__(self, client_id, client_version, user_name, public_key):
        super().__init__(client_id, client_version)
        self.user_name = user_name.decode().rstrip("\x00")
        self.public_key = public_key

    def __str__(self):
        return (
            f"{type(self).__name__}, "
            + super().__str__()
            + f", user name: {self.user_name}, public key: {self.public_key}"
        )


class ListClientsRequest(Request):
    def __init__(self, client_id, client_version):
        super().__init__(client_id, client_version)

    def __str__(self):
        return f"{type(self).__name__}, " + super().__str__()


class GetPublicKeyRequest(Request):
    def __init__(self, client_id, client_version, dest_client_id):
        super().__init__(client_id, client_version)
        self.dest_client_id = dest_client_id

    def __str__(self):
        return (
            f"{type(self).__name__}, "
            + super().__str__()
            + f", destination client id: {self.dest_client_id}"
        )


class SendMessageRequest(Request):
    def __init__(
        self, client_id, client_version, dest_client_id, message_type, content_size
    ):
        super().__init__(client_id, client_version)
        self.dest_client_id = dest_client_id
        self.message_type = message_type
        buffer = "\0" * content_size
        self.payload = bytearray(buffer, "utf-8")
        self.payload_index = 0
        self.content_size = content_size

    def __str__(self):
        return (
            f"{type(self).__name__}, "
            + super().__str__()
            + f", destination client id: {self.dest_client_id}, message type: {self.message_type}, content size: {self.content_size}"
        )


class PullMessagesRequest(Request):
    def __init__(self, client_id, client_version):
        super().__init__(client_id, client_version)

    def __str__(self):
        return f"{type(self).__name__}, " + super().__str__()
