import math
import struct

from common import (
    ResponseCode,
    BUFFER_PARTITION_SIZE,
    CLIENT_ID_BYTE_SIZE,
    VERSION_BYTE_SIZE,
    RESPONSE_CODE_BYTE_SIZE,
    PAYLOAD_SIZE_BYTE_SIZE,
    USER_NAME_BYTE_SIZE,
    PUBLIC_KEY_BYTE_SIZE,
    MESSAGE_ID_BYTE_SIZE,
)


class ResponseHandler:
    def __init__(self, server_version, response_code, payload_size):
        self.server_version = server_version
        self.response_code = response_code
        self.payload_size = payload_size

    def __str__(self):
        return f" server version: {self.server_version}, response code: {self.response_code}, payload size: {self.payload_size}"

    @staticmethod
    def _get_header_formatter():
        return "<B H I"

    @staticmethod
    def _prepare_client_id(client_id):
        max_int64 = 0xFFFFFFFFFFFFFFFF
        return client_id.bytes
        return (client_id.int >> 64) & max_int64, client_id.int & max_int64

    def _get_request_size(self):
        return (
            VERSION_BYTE_SIZE
            + RESPONSE_CODE_BYTE_SIZE
            + PAYLOAD_SIZE_BYTE_SIZE
            + self.payload_size
        )

    def _get_pack_header_data(self):
        return self.server_version, self.response_code, self.payload_size

    def _create_buffer(self):
        request_size = self._get_request_size()
        buffer_size = (
            math.ceil(request_size / BUFFER_PARTITION_SIZE) * BUFFER_PARTITION_SIZE
        )
        buffer = "\0" * buffer_size
        return bytearray(buffer, "utf-8")

    def create_response(self):
        return self._format_response()

    def _format_response(self):
        buffer = self._create_buffer()
        fmt = self._get_parse_formatter()
        s = struct.Struct(fmt)
        data = self._get_pack_data()
        s.pack_into(buffer, 0, *data)
        return buffer

    def _get_parse_formatter(self):
        raise NotImplementedError

    def _get_pack_data(self):
        raise NotImplementedError


class RegisterSuccessResponseHandler(ResponseHandler):
    def __init__(self, server_version, client_id):
        super().__init__(
            server_version,
            ResponseCode.REGISTER_SUCCESS.value,
            payload_size=CLIENT_ID_BYTE_SIZE,
        )
        self.client_id = client_id

    def __str__(self):
        return (
            f"{type(self).__name__}, "
            + super().__str__()
            + f", client id: {self.client_id}"
        )

    def _get_parse_formatter(self):
        return self._get_header_formatter() + " " + str(CLIENT_ID_BYTE_SIZE) + "s"

    def _get_pack_data(self):
        return *self._get_pack_header_data(), self._prepare_client_id(
            client_id=self.client_id
        )


class ListClientsSuccessResponseHandler(ResponseHandler):
    def __init__(self, server_version, clients_list):
        super().__init__(
            server_version,
            ResponseCode.LIST_CLIENTS_SUCCESS.value,
            payload_size=len(clients_list)
            * (CLIENT_ID_BYTE_SIZE + USER_NAME_BYTE_SIZE),
        )
        self.clients_list = clients_list

    def __str__(self):
        return (
            f"{type(self).__name__}, "
            + super().__str__()
            + f", clients list: {self.clients_list}"
        )

    def _get_parse_formatter(self):
        return self._get_header_formatter() + (
            " " + str(CLIENT_ID_BYTE_SIZE) + "s " + str(USER_NAME_BYTE_SIZE) + "s"
        ) * len(self.clients_list)

    def _get_pack_data(self):
        flat_client_list = []
        for client_id, user_name in self.clients_list:
            client_id_in_bytes = self._prepare_client_id(client_id)
            user_name = user_name
            flat_client_list.append(client_id_in_bytes)
            flat_client_list.append(user_name)

        return *self._get_pack_header_data(), *flat_client_list


class PublicKeySuccessResponseHandler(ResponseHandler):
    def __init__(self, server_version, client_id, public_key):
        super().__init__(
            server_version,
            ResponseCode.PUBLIC_KEY_SUCCESS.value,
            payload_size=CLIENT_ID_BYTE_SIZE + PUBLIC_KEY_BYTE_SIZE,
        )
        self.client_id = client_id
        self.public_key = public_key

    def __str__(self):
        return (
            f"{type(self).__name__}, "
            + super().__str__()
            + f", client id: {self.client_id}, public key: {self.public_key}"
        )

    def _get_parse_formatter(self):
        return (
            self._get_header_formatter()
            + " "
            + str(CLIENT_ID_BYTE_SIZE)
            + "s "
            + str(PUBLIC_KEY_BYTE_SIZE)
            + "s"
        )

    def _get_pack_data(self):
        return (
            *self._get_pack_header_data(),
            self._prepare_client_id(client_id=self.client_id),
            self.public_key,
        )


class MessageSentSuccessResponseHandler(ResponseHandler):
    def __init__(self, server_version, client_id, message_id):
        super().__init__(
            server_version,
            ResponseCode.MESSAGE_SENT_SUCCESS.value,
            payload_size=CLIENT_ID_BYTE_SIZE + MESSAGE_ID_BYTE_SIZE,
        )
        self.client_id = client_id
        self.message_id = message_id

    def __str__(self):
        return (
            f"{type(self).__name__}, "
            + super().__str__()
            + f", client id: {self.client_id}, message id: {self.message_id}"
        )

    def _get_parse_formatter(self):
        return self._get_header_formatter() + " " + str(CLIENT_ID_BYTE_SIZE) + "s I"

    def _get_pack_data(self):
        return (
            *self._get_pack_header_data(),
            self._prepare_client_id(client_id=self.client_id),
            self.message_id,
        )


class PullMessagesSuccessResponseHandler(ResponseHandler):
    def __init__(self, server_version, messages_list, size_of_returned_messages):
        super().__init__(
            server_version,
            ResponseCode.MESSAGES_PULLED_SUCCESS.value,
            payload_size=size_of_returned_messages,
        )
        self.messages_list = messages_list

    def __str__(self):
        return (
            f"{type(self).__name__}, "
            + super().__str__()
            + f", messages list: {self.messages_list}"
        )

    def _get_parse_formatter(self):
        fmt = self._get_header_formatter()

        for message in self.messages_list:
            fmt += (
                " "
                + str(CLIENT_ID_BYTE_SIZE)
                + "s I B I "
                + str(len(message.message_content))
                + "s"
            )
        return fmt

    def _get_pack_data(self):
        flat_client_list = []
        for message in self.messages_list:
            from_client = self._prepare_client_id(message.from_client)
            flat_client_list.append(from_client)
            flat_client_list.append(message.message_id)
            flat_client_list.append(message.message_type)
            flat_client_list.append(len(message.message_content))
            flat_client_list.append(message.message_content)

        return *self._get_pack_header_data(), *flat_client_list


class GeneralServerErrorResponseHandler(ResponseHandler):
    def __init__(self, server_version):
        return super().__init__(
            server_version, ResponseCode.GENERAL_SERVER_ERROR.value, payload_size=0
        )

    def __str__(self):
        return f"{type(self).__name__}, " + super().__str__()

    def _get_parse_formatter(self):
        return self._get_header_formatter()

    def _get_pack_data(self):
        return self._get_pack_header_data()
