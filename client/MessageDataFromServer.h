#pragma once
#include "common.h"

class MessageDataFromServer
{
private:
    std::string clientId;
    unsigned int messageId;
    MessageType messageType;
    std::string messageContent;

public:
    MessageDataFromServer(std::string& in_clientId, unsigned int in_messageId, MessageType in_messageType, unsigned int in_messageSize, unsigned char* in_messageToCopy);
    ~MessageDataFromServer();

    const std::string getClientId() const;
    const unsigned int getMessageId() const;
    const MessageType getMessageType() const;
    const std::string getMessageContent() const;
};