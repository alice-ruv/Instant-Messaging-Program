#pragma once
#include "IServerResponse.h" 
#include "MessageDataFromServer.h"

enum ResponseCode 
{
    REGISTER_SUCCESS = 2100,
    LIST_CLIENTS_SUCCESS = 2101,
    PUBLIC_KEY_SUCCESS = 2102,
    MESSAGE_SENT_SUCCESS = 2103,
    MESSAGES_PULLED_SUCCESS = 2104,
    GENERAL_SERVER_ERROR = 9000,
};


class ServerResponseBase : public IServerResponse
{
protected:
    unsigned int version; //1 byte
    unsigned int payloadSize; //4 byte
    std::string responseText;

public:
    virtual ~ServerResponseBase();

    static unsigned long long int getNumFromBuffer(const unsigned char* buffer, int numOfBytes);
    virtual bool isResponseRegisterSuccess() const;
    virtual const std::string getResponseText() const;

protected:
    ServerResponseBase(unsigned int in_version, unsigned int in_payloadSize);
    static void assertPayloadSize(unsigned int expectedPayloadSize, unsigned int payloadSize);
    static std::string getClientIdFromBuffer(const unsigned char* buffer);
};


class GeneralErrorResponse : public ServerResponseBase
{
public:
    GeneralErrorResponse(unsigned int in_version, unsigned int in_payloadSize);
    virtual ParseStatus parsePayload(const unsigned char* buffer);
    virtual void handleResponse(IClientDataManager* clientDataManager);
    virtual ~GeneralErrorResponse();
};


//for responses that it's easier to parse from a full payload
class BigResponse : public ServerResponseBase
{
protected:
    unsigned int payloadCopiedSize;
    unsigned char* payload; //dynamic

    BigResponse(unsigned int in_version, unsigned int in_payloadSize);

public:
    virtual ~BigResponse();
    virtual ParseStatus parsePayload(const unsigned char* buffer);

private:
    virtual void parseDataList() = 0;
};


class RegisterSuccessResponse : public ServerResponseBase
{  
private:
    std::string clientId;
    bool isCompletedSuccess;

public:
    RegisterSuccessResponse(unsigned int in_version, unsigned int in_payloadSize);
    virtual ~RegisterSuccessResponse();
    virtual ParseStatus parsePayload(const unsigned char* buffer);
    virtual void handleResponse(IClientDataManager* clientDataManager);
    virtual bool isResponseRegisterSuccess() const;
};


class ListClientsSuccessResponse : public BigResponse
{
private:
    unsigned int numOfClients;
    std::vector <ClientDataFromServer*> clientDataFromServerList;

public:
    ListClientsSuccessResponse(unsigned int in_version, unsigned int in_payloadSize);
    virtual ~ListClientsSuccessResponse();

    void insertClientDataFromServer(ClientDataFromServer* clientDataFromServer);
    unsigned int getNumOfClients() const;
    virtual void handleResponse(IClientDataManager* clientDataManager);

private:
    virtual void parseDataList();
};


class PublicKeySuccessResponse : public ServerResponseBase
{
private:
    std::string clientId;
    std::string publicKey;

public:
    PublicKeySuccessResponse(unsigned int in_version, unsigned int in_payloadSize);
    virtual ParseStatus parsePayload(const unsigned char* buffer);
    virtual void handleResponse(IClientDataManager* clientDataManager);
    virtual ~PublicKeySuccessResponse();
};


class MessageSentSuccessResponse : public ServerResponseBase
{
private:
    std::string clientId;
    unsigned int messageId;

public:
    MessageSentSuccessResponse(unsigned int in_version, unsigned int in_payloadSize);
    virtual ParseStatus parsePayload(const unsigned char* buffer);
    virtual void handleResponse(IClientDataManager* clientDataManager);
    virtual ~MessageSentSuccessResponse();
};


class PullMessagesSuccessResponse : public BigResponse
{
private:
    std::vector <MessageDataFromServer*> messageDataFromServerList;

public:
    PullMessagesSuccessResponse(unsigned int in_version, unsigned int in_payloadSize);
    virtual ~PullMessagesSuccessResponse();

    void insertMessageDataFromServer(MessageDataFromServer* messageDataFromServer);
    virtual void handleResponse(IClientDataManager* clientDataManager);  

private:
    virtual void parseDataList();
    std::string decrypteMessage(const MessageDataFromServer* messageDataFromServer, IClientDataManager* clientDataManager);
};