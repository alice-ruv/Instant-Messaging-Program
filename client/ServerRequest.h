#pragma once
#include "IServerRequest.h"
#include <vector>

enum RequestCode {
    REGISTER = 1100,
    LIST_CLIENTS = 1101,
    GET_PUBLIC_KEY = 1102,
    SEND_MESSAGE = 1103,
    PULL_MESSAGES = 1104,
};

class ServerRequestBase : public IServerRequest
{
private:
    //header
    std::string  clientId; //16 bytes
    unsigned int version; //1 byte
    RequestCode op; //2 bytes
    unsigned int payloadSize; //4 bytes

public:
	virtual ~ServerRequestBase();

protected:
    ServerRequestBase(std::string& in_clientId, unsigned int in_version, RequestCode in_requestCode, int in_payloadSize);
    void parseHeaderIntoBuffer(unsigned char* buffer);
    static void parseNumIntoBuffer(unsigned char* buffer, unsigned int num, int numOfBytes);
    static void parseClientId(unsigned int offset, const std::string& clientId, unsigned char* buffer);
};


class RegisterServerRequest : public ServerRequestBase
{
private:
    std::string userName; //255 bytes
    std::string publicKey; //160 bytes

public:
    RegisterServerRequest(std::string& in_clientId, unsigned int in_version, std::string& in_userName, std::string& in_publicKey);
    virtual ~RegisterServerRequest();

    virtual ParseStatus prepareRequestBuffer(unsigned char* buffer);
};


class ListClientsServerRequest : public ServerRequestBase
{
public:
    ListClientsServerRequest(std::string& in_clientId, unsigned int in_version);
    virtual ~ListClientsServerRequest();

    virtual ParseStatus prepareRequestBuffer(unsigned char* buffer);
};


class GetPublicKeyServerRequest : public ServerRequestBase
{
private:
    std::string destClientId; //16 bytes

public:
    GetPublicKeyServerRequest(std::string& in_clientId, unsigned int in_version, std::string& in_destClientId);
    virtual ~GetPublicKeyServerRequest();

    virtual ParseStatus prepareRequestBuffer(unsigned char* buffer);
};


class SendMessageServerRequest : public ServerRequestBase
{
private:
    std::string destClientId; //16 bytes
    MessageType messageType; //1 byte
    std::vector<unsigned char>  messageContent; //dynamic
    unsigned int sentMessageContentPayload;

public:
    SendMessageServerRequest(std::string& in_clientId, unsigned int in_version, std::string& in_destClientId, MessageType in_messageType, std::string& in_messageContent);
    virtual ~SendMessageServerRequest();

    virtual ParseStatus prepareRequestBuffer(unsigned char* buffer);

private:
    ParseStatus parseMessageContentIntoBuffer(unsigned char* buffer);
};


class PullMessagesServerRequest : public ServerRequestBase
{
public:
    PullMessagesServerRequest(std::string& in_clientId, unsigned int in_version);
    virtual ~PullMessagesServerRequest();

    virtual ParseStatus prepareRequestBuffer(unsigned char* buffer);
};