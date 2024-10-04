import uuid

from common import (
    CONTENT_SIZE_BYTE_SIZE,
    MESSAGE_TYPE_BYTE_SIZE,
    MESSAGE_ID_BYTE_SIZE,
    CLIENT_ID_BYTE_SIZE,
    MAX_PULLED_MESSAGES_DATA,
)


class MessageUManager:
    def __init__(self, storage_handler):
        self.storage_handler = storage_handler

    def register_client(self, user_name, public_key):
        if self.storage_handler.is_user_name_exist(user_name=user_name):
            raise Exception(f"user name: {user_name} already exist")
        # generating random client id (16 bytes)
        client_id = uuid.uuid4()
        self.storage_handler.add_client(
            client_id=client_id, user_name=user_name, public_key=public_key
        )
        return client_id

    def list_clients(self, client_id):
        self._handle_pre_request(client_id)
        return self.storage_handler.list_clients(client_id_to_exclude=client_id)

    def get_client_public_key(self, client_id, dest_client_id):
        self._handle_pre_request(client_id)
        public_key = self.storage_handler.get_client_public_key(
            client_id=dest_client_id
        )
        if not public_key:
            raise Exception(
                f"Can't get public key for client id: {dest_client_id}. the client doesn't exist"
            )
        return public_key

    def send_message(self, client_id, to_client_id, message_type, message_content):
        self._handle_pre_request(client_id)
        if not self.storage_handler.is_client_id_exist(client_id=to_client_id):
            raise Exception(f"dest client isn't registered. client_id: {to_client_id}")

        return self.storage_handler.save_message(
            to_client_id=to_client_id,
            from_client_id=client_id,
            message_type=message_type,
            message_content=message_content,
        )

    def pull_messages(self, client_id):
        self._handle_pre_request(client_id=client_id)

        messages = self.storage_handler.pull_client_messages(client_id=client_id)
        result_messages = []
        size_of_returned_messages = 0
        const_size_of_message = (
            CLIENT_ID_BYTE_SIZE
            + MESSAGE_ID_BYTE_SIZE
            + MESSAGE_TYPE_BYTE_SIZE
            + CONTENT_SIZE_BYTE_SIZE
        )
        for message in messages:
            size_of_message = len(message.message_content) + const_size_of_message
            if size_of_message <= (
                MAX_PULLED_MESSAGES_DATA - size_of_returned_messages
            ):
                result_messages.append(message)
                size_of_returned_messages += size_of_message
            else:
                break

        return result_messages, size_of_returned_messages

    def delete_messages(self, messages_ids_list_to_delete):
        self.storage_handler.delete_client_messages(
            messages_ids_list_to_delete=messages_ids_list_to_delete
        )

    # need to be done before handling a request (except register)
    def _handle_pre_request(self, client_id):
        if not self.storage_handler.is_client_id_exist(client_id=client_id):
            raise Exception(f"client isn't registered. client_id: {client_id}")
        self.storage_handler.update_client_last_seen(client_id=client_id)
