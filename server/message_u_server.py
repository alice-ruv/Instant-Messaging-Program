import selectors
import socket

from common import (
    BUFFER_PARTITION_SIZE,
    RequestParseStatus,
    ResponseCode,
    SERVER_INFO_FILE_NAME,
    MIN_VALID_PORT_NUMBER,
    MAX_VALID_PORT_NUMBER,
)
from message_u_manager import MessageUManager
from request_parser import RequestParserFactory
from request import (
    RegisterRequest,
    ListClientsRequest,
    GetPublicKeyRequest,
    SendMessageRequest,
    PullMessagesRequest,
)
from response_handler import (
    RegisterSuccessResponseHandler,
    ListClientsSuccessResponseHandler,
    PublicKeySuccessResponseHandler,
    MessageSentSuccessResponseHandler,
    PullMessagesSuccessResponseHandler,
    GeneralServerErrorResponseHandler,
)
from storage_handler import SQLStorageHandler


class MessageUServer:
    server_version = 2

    def __init__(self, port=None):
        self.port = port
        self.sel = selectors.DefaultSelector()
        self.message_u_manager = MessageUManager(SQLStorageHandler())

    def _initialize(self):
        if self.port is None:
            file = open(SERVER_INFO_FILE_NAME)
            port = int(file.read())
            if port < MIN_VALID_PORT_NUMBER or port > MAX_VALID_PORT_NUMBER:
                print(f"{port} is not valid port number")
                return False
            self.port = port
            return True

    def start_server(self):
        if self._initialize() is False:
            print("Failed starting server. closing server")
            return

        sock = socket.socket()
        sock.bind(("localhost", self.port))
        sock.listen(100)
        sock.setblocking(False)
        self.sel.register(sock, selectors.EVENT_READ, self._accept)
        while True:
            events = self.sel.select()
            for key, mask in events:
                callback = key.data
                callback(key.fileobj, mask)

    def _accept(self, sock, mask):
        conn, addr = sock.accept()  # Should be ready
        print(f"accepted {conn} from {addr}")
        conn.setblocking(True)
        self.sel.register(conn, selectors.EVENT_READ, self._handle_session)

    def _handle_request(self, request):
        print(f"handle request: {request}")
        response_parse_handler = None
        if isinstance(request, RegisterRequest):
            client_id = self.message_u_manager.register_client(
                user_name=request.user_name, public_key=request.public_key
            )
            response_parse_handler = RegisterSuccessResponseHandler(
                server_version=self.server_version, client_id=client_id
            )
        elif isinstance(request, ListClientsRequest):
            clients_list = self.message_u_manager.list_clients(
                client_id=request.client_id
            )
            response_parse_handler = ListClientsSuccessResponseHandler(
                server_version=self.server_version, clients_list=clients_list
            )
        elif isinstance(request, GetPublicKeyRequest):
            public_key = self.message_u_manager.get_client_public_key(
                client_id=request.client_id, dest_client_id=request.dest_client_id
            )
            response_parse_handler = PublicKeySuccessResponseHandler(
                server_version=self.server_version,
                client_id=request.dest_client_id,
                public_key=public_key,
            )
        elif isinstance(request, SendMessageRequest):
            message_id = self.message_u_manager.send_message(
                client_id=request.client_id,
                to_client_id=request.dest_client_id,
                message_type=request.message_type,
                message_content=request.payload,
            )
            response_parse_handler = MessageSentSuccessResponseHandler(
                server_version=self.server_version,
                client_id=request.dest_client_id,
                message_id=message_id,
            )
        elif isinstance(request, PullMessagesRequest):
            (
                result_messages,
                size_of_returned_messages,
            ) = self.message_u_manager.pull_messages(client_id=request.client_id)
            response_parse_handler = PullMessagesSuccessResponseHandler(
                server_version=self.server_version,
                messages_list=result_messages,
                size_of_returned_messages=size_of_returned_messages,
            )

        return response_parse_handler

    def _after_send_process(self, response):
        if response.response_code == ResponseCode.MESSAGES_PULLED_SUCCESS.value:
            self.message_u_manager.delete_messages(
                [message.message_id for message in response.messages_list]
            )

    def _handle_session(self, conn, mask):
        try:
            parse_status = RequestParseStatus.UNDONE.value
            request = None
            request_parser = None
            response_parse_handler = None

            try:
                while parse_status == RequestParseStatus.UNDONE.value:
                    data_from_server = conn.recv(BUFFER_PARTITION_SIZE)
                    if request_parser is None:
                        request_parser = RequestParserFactory.get_request_parser(
                            request_buffer=data_from_server
                        )
                    (parse_status, request) = request_parser.parse_request(
                        request_buffer=data_from_server, request=request
                    )

                response_parse_handler = self._handle_request(request)
            except Exception as e:
                response_parse_handler = GeneralServerErrorResponseHandler(
                    server_version=self.server_version
                )
                print(e)
            print(f"parsing response: {response_parse_handler}")
            response_buffer = response_parse_handler.create_response()
            for partition_start_index in range(
                0, len(response_buffer), BUFFER_PARTITION_SIZE
            ):
                conn.sendall(
                    response_buffer[
                        partition_start_index : partition_start_index
                        + BUFFER_PARTITION_SIZE
                    ]
                )

            self._after_send_process(response=response_parse_handler)
        except Exception as e:
            print(e)

        # closing connection
        self.sel.unregister(conn)
        conn.close()
