#pragma once
#include <string>

class CommonConsts
{
public:
    static const unsigned int CLIENT_VERSION = 2;
    static const unsigned int BUFFER_PARTITION_SIZE = 1024;
    static const unsigned int CLIENT_ID_BYTE_SIZE = 16;
    static const unsigned int VERSION_BYTE_SIZE = 1;
    static const unsigned int REQUEST_CODE_BYTE_SIZE = 2;
    static const unsigned int PAYLOAD_SIZE_BYTE_SIZE = 4;
    static const unsigned int RESPONSE_CODE_BYTE_SIZE = 2;
    static const unsigned int USER_NAME_BYTE_SIZE = 255;
    static const unsigned int PUBLIC_KEY_BYTE_SIZE = 160;
    static const unsigned int MESSAGE_TYPE_BYTE_SIZE = 1;
    static const unsigned int CONTENT_SIZE_BYTE_SIZE = 4;
    static const unsigned int MESSAGE_ID_BYTE_SIZE = 4;
    
    //only request consts
    static const unsigned int CLIENT_ID_OFFSET_IN_REQUEST_BUFFER = 0;
    static const unsigned int VERSION_OFFSET_IN_REQUEST_BUFFER = CLIENT_ID_BYTE_SIZE;
    static const unsigned int REQUEST_CODE_OFFSET_IN_REQUEST_BUFFER = VERSION_OFFSET_IN_REQUEST_BUFFER + VERSION_BYTE_SIZE;
    static const unsigned int PAYLOAD_SIZE_OFFSET_IN_REQUEST_BUFFER = REQUEST_CODE_OFFSET_IN_REQUEST_BUFFER + REQUEST_CODE_BYTE_SIZE;
    static const unsigned int PAYLOAD_OFFSET_IN_REQUEST_BUFFER = PAYLOAD_SIZE_OFFSET_IN_REQUEST_BUFFER + PAYLOAD_SIZE_BYTE_SIZE;
    static const unsigned int USER_NAME_OFFSET_IN_REQUEST_BUFFER = PAYLOAD_OFFSET_IN_REQUEST_BUFFER;
    static const unsigned int PUBLIC_KEY_OFFSET_IN_REQUEST_BUFFER = USER_NAME_OFFSET_IN_REQUEST_BUFFER + USER_NAME_BYTE_SIZE;
    static const unsigned int DEST_CLIENT_ID_OFFSET_IN_REQUEST_BUFFER = PAYLOAD_OFFSET_IN_REQUEST_BUFFER;
    static const unsigned int MESSAGE_TYPE_OFFSET_IN_REQUEST_BUFFER = DEST_CLIENT_ID_OFFSET_IN_REQUEST_BUFFER + CLIENT_ID_BYTE_SIZE;
    static const unsigned int CONTENT_SIZE_OFFSET_IN_REQUEST_BUFFER = MESSAGE_TYPE_OFFSET_IN_REQUEST_BUFFER + MESSAGE_TYPE_BYTE_SIZE;
    static const unsigned int MESSAGE_CONTENT_OFFSET_IN_REQUEST_BUFFER = CONTENT_SIZE_OFFSET_IN_REQUEST_BUFFER + CONTENT_SIZE_BYTE_SIZE;

    //only response consts
    static const unsigned int VERSION_OFFSET_IN_RESPONSE_BUFFER = 0;
    static const unsigned int RESPONSE_CODE_OFFSET_IN_RESPONSE_BUFFER = VERSION_OFFSET_IN_RESPONSE_BUFFER + VERSION_BYTE_SIZE;
    static const unsigned int PAYLOAD_SIZE_OFFSET_IN_RESPONSE_BUFFER = RESPONSE_CODE_OFFSET_IN_RESPONSE_BUFFER + RESPONSE_CODE_BYTE_SIZE;
    static const unsigned int PAYLOAD_OFFSET_IN_RESPONSE_BUFFER = PAYLOAD_SIZE_OFFSET_IN_RESPONSE_BUFFER + PAYLOAD_SIZE_BYTE_SIZE;
    static const unsigned int CLIENT_ID_OFFSET_IN_RESPONSE_BUFFER = PAYLOAD_OFFSET_IN_RESPONSE_BUFFER;
    static const unsigned int PUBLIC_KEY_OFFSET_IN_RESPONSE_BUFFER = CLIENT_ID_OFFSET_IN_RESPONSE_BUFFER + CLIENT_ID_BYTE_SIZE;
    static const unsigned int MESSAGE_ID_OFFSET_IN_RESPONSE_BUFFER = CLIENT_ID_OFFSET_IN_RESPONSE_BUFFER + CLIENT_ID_BYTE_SIZE;

    //ClientDataFromServer consts
    static const unsigned int CLIENT_DATA_FROM_SERVER_SIZE_IN_RESPONSE_BUFFER = CLIENT_ID_BYTE_SIZE + USER_NAME_BYTE_SIZE;

    //server related consts
    static const unsigned int MIN_VALID_PORT_NUMBER = 1024;
    static const unsigned int MAX_VALID_PORT_NUMBER = 65535;

    inline static const std::string SERVER_INFO_FILE_NAME = "server.info";
    inline static const std::string USER_INFO_FILE_NAME = "my.info";
    inline static const std::string ERROR_MESSAGE = "server responded with an error\n\n";
};

enum MessageType {
    REQUEST_SYMMETRIC_KEY = 0,
    SEND_SYMMETRIC_KEY = 1,
    SEND_TEXT_MESSAGE = 2,
    SEND_FILE = 3,
};

enum ParseStatus
{
    UNDONE = 0,
    DONE = 1
};