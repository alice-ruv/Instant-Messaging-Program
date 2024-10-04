import struct
from uuid import UUID

from common import (
    RequestCode,
    USER_NAME_BYTE_SIZE,
    PUBLIC_KEY_BYTE_SIZE,
    CLIENT_ID_BYTE_SIZE,
    VERSION_BYTE_SIZE,
    REQUEST_CODE_BYTE_SIZE,
    PAYLOAD_SIZE_BYTE_SIZE,
    RequestParseStatus,
    BUFFER_PARTITION_SIZE,
    MESSAGE_TYPE_BYTE_SIZE,
    CONTENT_SIZE_BYTE_SIZE,
)

from request import (
    RegisterRequest,
    ListClientsRequest,
    GetPublicKeyRequest,
    SendMessageRequest,
    PullMessagesRequest,
)


class IRequestParser:
    def parse_request(self, request_buffer, request=None):
        raise NotImplementedError


class RequestParserBase(IRequestParser):
    def __init__(self, client_id, client_version, payload_size):
        self.client_id = client_id
        self.client_version = client_version
        self.payload_size = payload_size

    def __str__(self):
        return f"{type(self).__name__}, client id: {self.client_id}, client version: {self.client_version}, payload size: {self.payload_size}"

    def parse_request(self, request_buffer, request=None):
        raise NotImplementedError

    @staticmethod
    def parse_header(request_buffer):
        fmt = str(CLIENT_ID_BYTE_SIZE) + "s"

        (client_id,) = struct.unpack_from(fmt, request_buffer, offset=0)
        client_id = UUID(bytes=client_id)

        fmt = "<B H I"
        (client_version, request_code, payload_size) = struct.unpack_from(
            fmt, request_buffer, offset=CLIENT_ID_BYTE_SIZE
        )

        return (client_id, client_version, request_code, payload_size)


class RegisterRequestParser(RequestParserBase):
    def __init__(self, client_id, client_version, payload_size):
        super().__init__(client_id, client_version, payload_size)

    def __str__(self):
        f"{type(self).__name__}, " + super().__str__()

    def parse_request(self, request_buffer, request=None):
        if self.payload_size != (USER_NAME_BYTE_SIZE + PUBLIC_KEY_BYTE_SIZE):
            raise Exception("invalid register request payload size")

        fmt = str(USER_NAME_BYTE_SIZE) + "s " + str(PUBLIC_KEY_BYTE_SIZE) + "s"
        offset = (
            CLIENT_ID_BYTE_SIZE
            + VERSION_BYTE_SIZE
            + REQUEST_CODE_BYTE_SIZE
            + PAYLOAD_SIZE_BYTE_SIZE
        )

        (user_name, public_key) = struct.unpack_from(fmt, request_buffer, offset=offset)
        return RequestParseStatus.DONE.value, RegisterRequest(
            self.client_id,
            client_version=self.client_version,
            user_name=user_name,
            public_key=public_key,
        )


class ListClientsRequestParser(RequestParserBase):
    def __init__(self, client_id, client_version, payload_size):
        super().__init__(client_id, client_version, payload_size)

    def __str__(self):
        f"{type(self).__name__}, " + super().__str__()

    def parse_request(self, request_buffer, request=None):
        if self.payload_size != 0:
            raise Exception("invalid list clients request payload size")
        return RequestParseStatus.DONE.value, ListClientsRequest(
            self.client_id, client_version=self.client_version
        )


class GetPublicKeyRequestParser(RequestParserBase):
    def __init__(self, client_id, client_version, payload_size):
        super().__init__(client_id, client_version, payload_size)

    def __str__(self):
        f"{type(self).__name__}, " + super().__str__()

    def parse_request(self, request_buffer, request=None):
        if self.payload_size != CLIENT_ID_BYTE_SIZE:
            raise Exception("invalid get public key payload size")
        fmt = str(CLIENT_ID_BYTE_SIZE) + "s"
        offset = (
            CLIENT_ID_BYTE_SIZE
            + VERSION_BYTE_SIZE
            + REQUEST_CODE_BYTE_SIZE
            + PAYLOAD_SIZE_BYTE_SIZE
        )
        (dest_client_id,) = struct.unpack_from(fmt, request_buffer, offset=offset)
        dest_client_id = UUID(bytes=dest_client_id)
        return RequestParseStatus.DONE.value, GetPublicKeyRequest(
            client_id=self.client_id,
            client_version=self.client_version,
            dest_client_id=dest_client_id,
        )


class PullMessagesRequestParser(RequestParserBase):
    def __init__(self, client_id, client_version, payload_size):
        super().__init__(client_id, client_version, payload_size)

    def __str__(self):
        f"{type(self).__name__}, " + super().__str__()

    def parse_request(self, request_buffer, request=None):
        if self.payload_size != 0:
            raise Exception("invalid pull messages request payload size")
        return RequestParseStatus.DONE.value, PullMessagesRequest(
            self.client_id, client_version=self.client_version
        )


class SendMessageRequestParser(RequestParserBase):
    def __init__(self, client_id, client_version, payload_size):
        super().__init__(client_id, client_version, payload_size)

    def __str__(self):
        f"{type(self).__name__}, " + super().__str__()

    @staticmethod
    def _parse_content(request_buffer, request):

        request_buffer_start_index = (
            0
            if request.payload_index != 0
            else CLIENT_ID_BYTE_SIZE
            + VERSION_BYTE_SIZE
            + REQUEST_CODE_BYTE_SIZE
            + PAYLOAD_SIZE_BYTE_SIZE
            + CLIENT_ID_BYTE_SIZE
            + MESSAGE_TYPE_BYTE_SIZE
            + CONTENT_SIZE_BYTE_SIZE
        )
        payload_index = request.payload_index

        if (request.content_size - payload_index) > (
            BUFFER_PARTITION_SIZE - request_buffer_start_index
        ):
            parse_status = RequestParseStatus.UNDONE.value
            number_of_bytes_to_read = BUFFER_PARTITION_SIZE - request_buffer_start_index
        else:
            parse_status = RequestParseStatus.DONE.value
            number_of_bytes_to_read = request.content_size - payload_index

        request.payload[
            payload_index : payload_index + number_of_bytes_to_read
        ] = request_buffer[
            request_buffer_start_index : request_buffer_start_index
            + number_of_bytes_to_read
        ]
        request.payload_index = payload_index + number_of_bytes_to_read

        return parse_status, request

    def parse_request(self, request_buffer, request=None):
        if request:
            return self._parse_content(request_buffer=request_buffer, request=request)
        fmt = str(CLIENT_ID_BYTE_SIZE) + "s"
        offset = (
            CLIENT_ID_BYTE_SIZE
            + VERSION_BYTE_SIZE
            + REQUEST_CODE_BYTE_SIZE
            + PAYLOAD_SIZE_BYTE_SIZE
        )
        (dest_client_id,) = struct.unpack_from(fmt, request_buffer, offset=offset)
        dest_client_id = UUID(bytes=dest_client_id)
        fmt = "<B I"
        offset = offset + CLIENT_ID_BYTE_SIZE
        message_type, content_size = struct.unpack_from(
            fmt, request_buffer, offset=offset
        )
        request = SendMessageRequest(
            client_id=self.client_id,
            client_version=self.client_version,
            dest_client_id=dest_client_id,
            message_type=message_type,
            content_size=content_size,
        )
        return self._parse_content(request_buffer=request_buffer, request=request)


Request_Parser_CLS_MAP = {
    RequestCode.REGISTER.value: RegisterRequestParser,
    RequestCode.LIST_CLIENTS.value: ListClientsRequestParser,
    RequestCode.GET_PUBLIC_KEY.value: GetPublicKeyRequestParser,
    RequestCode.SEND_MESSAGE.value: SendMessageRequestParser,
    RequestCode.PULL_MESSAGES.value: PullMessagesRequestParser,
}


class RequestParserFactory:
    @staticmethod
    def get_request_parser(request_buffer):
        if len(request_buffer) == 0:
            raise Exception("no data")
        (
            client_id,
            client_version,
            request_code,
            payload_size,
        ) = RequestParserBase.parse_header(request_buffer=request_buffer)
        parser_cls = Request_Parser_CLS_MAP.get(request_code)
        if not parser_cls:
            raise Exception("invalid request type")
        return parser_cls(
            client_id=client_id,
            client_version=client_version,
            payload_size=payload_size,
        )
