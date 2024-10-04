#include "ServerRequest.h"


ServerRequestBase::~ServerRequestBase() {}


ServerRequestBase::ServerRequestBase(std::string& in_clientId, unsigned int in_version, RequestCode in_requestCode, int in_payloadSize)
        : clientId(in_clientId)
        , version(in_version)
        , op(in_requestCode)
        , payloadSize(in_payloadSize) {}


RegisterServerRequest::RegisterServerRequest(std::string& in_clientId, unsigned int in_version, std::string& in_userName, std::string& in_publicKey) 
        : ServerRequestBase(in_clientId, in_version, RequestCode::REGISTER, CommonConsts::USER_NAME_BYTE_SIZE + CommonConsts::PUBLIC_KEY_BYTE_SIZE)
        , userName(in_userName)
        , publicKey(in_publicKey) {}


RegisterServerRequest::~RegisterServerRequest() {}


ListClientsServerRequest::ListClientsServerRequest(std::string& in_clientId, unsigned int in_version) 
    : ServerRequestBase(in_clientId, in_version, RequestCode::LIST_CLIENTS, 0) {}


ListClientsServerRequest::~ListClientsServerRequest() {}


GetPublicKeyServerRequest::GetPublicKeyServerRequest(std::string& in_clientId, unsigned int in_version, std::string& in_destClientId) 
    : ServerRequestBase(in_clientId, in_version, RequestCode::GET_PUBLIC_KEY, CommonConsts::CLIENT_ID_BYTE_SIZE)
    , destClientId(in_destClientId) {}


GetPublicKeyServerRequest::~GetPublicKeyServerRequest() {}


SendMessageServerRequest::SendMessageServerRequest(std::string& in_clientId, unsigned int in_version, std::string& in_destClientId, MessageType in_messageType, std::string& in_messageContent)
    : ServerRequestBase(in_clientId, in_version, RequestCode::SEND_MESSAGE
        , CommonConsts::CLIENT_ID_BYTE_SIZE + CommonConsts::MESSAGE_TYPE_BYTE_SIZE + CommonConsts::CONTENT_SIZE_BYTE_SIZE + (unsigned int)in_messageContent.size())
    , destClientId(in_destClientId)
    , messageType(in_messageType)
    , sentMessageContentPayload(0)
    , messageContent(in_messageContent.begin(), in_messageContent.end()) {}


SendMessageServerRequest::~SendMessageServerRequest()
{
    messageContent.clear();
}


PullMessagesServerRequest::PullMessagesServerRequest(std::string& in_clientId, unsigned int in_version) 
    : ServerRequestBase(in_clientId, in_version, RequestCode::PULL_MESSAGES, 0) {}


PullMessagesServerRequest::~PullMessagesServerRequest() {}


void ServerRequestBase::parseClientId(unsigned int offset, const std::string& clientId, unsigned char* buffer)
{
    for (unsigned long long i = 0; i < CommonConsts::CLIENT_ID_BYTE_SIZE; i++)
    {
        std::string num = clientId.substr(i * 2, 2);
        buffer[offset + i] = std::stoi(num, nullptr, 16);
    }
}


void ServerRequestBase::parseNumIntoBuffer(unsigned char* buffer, unsigned int num, int numOfBytes)
{
    for (int i = 0; i < numOfBytes; i++)
    {
        buffer[i] = (num >> i * 8) & 0xff;
    }
}


void ServerRequestBase::parseHeaderIntoBuffer(unsigned char* buffer)
{
    //client id 
    parseClientId(CommonConsts::CLIENT_ID_OFFSET_IN_REQUEST_BUFFER, clientId, buffer);

    //version
    parseNumIntoBuffer(buffer + CommonConsts::VERSION_OFFSET_IN_REQUEST_BUFFER, (unsigned int)version, CommonConsts::VERSION_BYTE_SIZE);

    //requestCode
    parseNumIntoBuffer(buffer + CommonConsts::REQUEST_CODE_OFFSET_IN_REQUEST_BUFFER, (unsigned int)op, CommonConsts::REQUEST_CODE_BYTE_SIZE);

    //payload size
    parseNumIntoBuffer(buffer + CommonConsts::PAYLOAD_SIZE_OFFSET_IN_REQUEST_BUFFER, (unsigned int)payloadSize, CommonConsts::PAYLOAD_SIZE_BYTE_SIZE);
}


ParseStatus RegisterServerRequest::prepareRequestBuffer(unsigned char* buffer)
{
    parseHeaderIntoBuffer(buffer);
    //user name
    std::copy(userName.begin(), userName.end(), &buffer[CommonConsts::USER_NAME_OFFSET_IN_REQUEST_BUFFER]);

    //public key
    std::copy(publicKey.begin(), publicKey.end(), &buffer[CommonConsts::PUBLIC_KEY_OFFSET_IN_REQUEST_BUFFER]);
    return ParseStatus::DONE;
}


ParseStatus GetPublicKeyServerRequest::prepareRequestBuffer(unsigned char* buffer)
{
    parseHeaderIntoBuffer(buffer);
    //dest client id
    parseClientId(CommonConsts::PAYLOAD_OFFSET_IN_REQUEST_BUFFER, destClientId, buffer);
    return ParseStatus::DONE;
}


ParseStatus SendMessageServerRequest::prepareRequestBuffer(unsigned char* buffer)
{
    if (sentMessageContentPayload > 0)
    {
        return parseMessageContentIntoBuffer(buffer);
    }

    parseHeaderIntoBuffer(buffer);
 
    //dest client id
    parseClientId(CommonConsts::PAYLOAD_OFFSET_IN_REQUEST_BUFFER, destClientId, buffer);

    //message type
    parseNumIntoBuffer(buffer + CommonConsts::MESSAGE_TYPE_OFFSET_IN_REQUEST_BUFFER, (unsigned int)messageType, CommonConsts::MESSAGE_TYPE_BYTE_SIZE);

    //content size
    parseNumIntoBuffer(buffer + CommonConsts::CONTENT_SIZE_OFFSET_IN_REQUEST_BUFFER, (unsigned int)messageContent.size(), CommonConsts::CONTENT_SIZE_BYTE_SIZE);
    return parseMessageContentIntoBuffer(buffer);
}


ParseStatus ListClientsServerRequest::prepareRequestBuffer(unsigned char* buffer)
{
    parseHeaderIntoBuffer(buffer);
    return ParseStatus::DONE;
}

ParseStatus PullMessagesServerRequest::prepareRequestBuffer(unsigned char* buffer)
{
    parseHeaderIntoBuffer(buffer);
    return ParseStatus::DONE;
}


ParseStatus SendMessageServerRequest::parseMessageContentIntoBuffer(unsigned char* buffer)
{
 
    auto messageContentSize = messageContent.size();

    auto bufferStartIndex = (sentMessageContentPayload != 0) ? 0 : CommonConsts::MESSAGE_CONTENT_OFFSET_IN_REQUEST_BUFFER;

    ParseStatus parseStatus;

    auto numOfBytesToWrite = 0;

    if ((messageContentSize - sentMessageContentPayload) > (CommonConsts::BUFFER_PARTITION_SIZE - bufferStartIndex))
    {
        parseStatus = ParseStatus::UNDONE;
        numOfBytesToWrite = CommonConsts::BUFFER_PARTITION_SIZE - (int)bufferStartIndex;
    }
    else
    {
        parseStatus = ParseStatus::DONE;
        numOfBytesToWrite = (unsigned int)messageContentSize - sentMessageContentPayload;
    }

    std::copy(messageContent.begin() + sentMessageContentPayload, messageContent.begin() + sentMessageContentPayload + numOfBytesToWrite, buffer + bufferStartIndex);
    sentMessageContentPayload = sentMessageContentPayload + numOfBytesToWrite;

    return parseStatus;
}