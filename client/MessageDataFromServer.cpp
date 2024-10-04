#include "MessageDataFromServer.h"


MessageDataFromServer::MessageDataFromServer(std::string& in_clientId, unsigned int in_messageId, MessageType in_messageType, unsigned int in_messageSize, unsigned char* in_messageToCopy)
    : clientId(in_clientId)
    , messageId(in_messageId)
    , messageType(in_messageType)
    , messageContent(std::string(in_messageToCopy, in_messageToCopy + in_messageSize)) {}


MessageDataFromServer::~MessageDataFromServer() {}


const std::string MessageDataFromServer::getClientId() const
{
    return clientId;
}


const unsigned int MessageDataFromServer::getMessageId() const
{
    return messageId;
}


const MessageType MessageDataFromServer::getMessageType() const
{
    return messageType;
}


const std::string MessageDataFromServer::getMessageContent() const
{
    return messageContent;
}
