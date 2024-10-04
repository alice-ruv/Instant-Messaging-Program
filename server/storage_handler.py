import datetime
from uuid import UUID
from message import Message
import sqlite3


class IStorageHandler:
    def is_client_id_exist(self, client_id):
        raise NotImplementedError

    def is_user_name_exist(self, user_name):
        raise NotImplementedError

    def add_client(self, client_id, user_name, public_key):
        raise NotImplementedError

    def list_clients(self, client_id_to_exclude):
        raise NotImplementedError

    def get_client_public_key(self, client_id):
        raise NotImplementedError

    def update_client_last_seen(self, client_id):
        raise NotImplementedError

    def save_message(
        self, message_id, to_client_id, from_client_id, message_type, message_content
    ):
        raise NotImplementedError

    def pull_client_messages(self, client_id):
        raise NotImplementedError

    def delete_client_messages(self, messages_ids_list_to_delete):
        raise NotImplementedError


class SQLStorageHandler(IStorageHandler):
    def __init__(self):

        conn = sqlite3.connect("server.db")
        conn.text_factory = bytes

        try:
            conn.executescript(
                """
                     CREATE TABLE IF NOT EXISTS clients(ID TEXT NOT NULL PRIMARY KEY,
                     UserName TEXT UNIQUE, PublicKey TEXT, LastSeen datetime default current_timestamp);
                     CREATE TABLE IF NOT EXISTS messages(ID INTEGER PRIMARY KEY AUTOINCREMENT,
                     ToClient TEXT, FromClient TEXT, Type INTEGER, Content BLOB);"""
            )
        except Exception as e:
            a = e

    def is_client_id_exist(self, client_id):
        conn = sqlite3.connect("server.db")
        conn.text_factory = bytes
        cur = conn.cursor()
        cur.execute("SELECT ID from clients WHERE ID = ?", (client_id.bytes,))
        res = True if cur.fetchone() is not None else False
        conn.close()
        return res

    def is_user_name_exist(self, user_name):
        conn = sqlite3.connect("server.db")
        conn.text_factory = bytes
        cur = conn.cursor()
        cur.execute("select UserName from clients WHERE UserName = ?", (user_name,))
        res = True if cur.fetchone() is not None else False
        conn.close()
        return res

    def add_client(self, client_id, user_name, public_key):
        conn = sqlite3.connect("server.db")
        conn.text_factory = bytes
        cur = conn.cursor()
        cur.execute(
            """ INSERT INTO clients (ID,UserName,PublicKey) VALUES(?,?,?)""",
            (client_id.bytes, user_name, public_key),
        )
        conn.commit()
        conn.close()

    def list_clients(self, client_id_to_exclude):
        conn = sqlite3.connect("server.db")
        conn.text_factory = bytes
        cur = conn.cursor()
        cur.execute(
            "select ID, UserName from clients WHERE ID <> ?",
            (client_id_to_exclude.bytes,),
        )
        res = [(UUID(bytes=client_id), user_name) for client_id, user_name in cur]
        conn.close()
        return res

    def get_client_public_key(self, client_id):
        conn = sqlite3.connect("server.db")
        conn.text_factory = bytes
        cur = conn.cursor()
        cur.execute("select PublicKey FROM clients WHERE ID = ?", (client_id.bytes,))
        row = cur.fetchone()
        conn.close()
        if row:
            (public_key,) = row
            return public_key
        return None

    def update_client_last_seen(self, client_id):
        conn = sqlite3.connect("server.db")
        conn.text_factory = bytes
        cur = conn.cursor()
        cur.execute(
            "UPDATE clients set LastSeen = ? WHERE ID = ?",
            (datetime.datetime.utcnow(), client_id.bytes),
        )
        conn.commit()
        conn.close()

    def save_message(self, to_client_id, from_client_id, message_type, message_content):
        conn = sqlite3.connect("server.db")
        conn.text_factory = bytes
        cur = conn.cursor()
        cur.execute(
            "INSERT INTO messages(ToClient,FromClient,Type,Content) VALUES(?,?,?,?)",
            (to_client_id.bytes, from_client_id.bytes, message_type, message_content),
        )
        conn.commit()
        conn.close()
        return cur.lastrowid

    def pull_client_messages(self, client_id):
        conn = sqlite3.connect("server.db")
        conn.text_factory = bytes
        cur = conn.cursor()
        cur.execute("select * from messages WHERE ToClient = ?", (client_id.bytes,))
        messages_list = [
            Message(
                message_id=message_id,
                to_client=UUID(bytes=to_client),
                from_client=UUID(bytes=from_client),
                message_type=message_type,
                message_content=message_content,
            )
            for (
                message_id,
                to_client,
                from_client,
                message_type,
                message_content,
            ) in cur
        ]
        conn.close()
        return messages_list

    def delete_client_messages(self, messages_ids_list_to_delete):
        if len(messages_ids_list_to_delete) == 0:
            return
        conn = sqlite3.connect("server.db")
        conn.text_factory = bytes
        cur = conn.cursor()
        query = (
            "DELETE FROM messages WHERE ID IN (?"
            + (len(messages_ids_list_to_delete) - 1) * ",?"
            + ")"
        )
        cur.execute(query, tuple(messages_ids_list_to_delete))
        conn.commit()
        conn.close()
