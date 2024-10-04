#pragma once
#include "IClientDataManager.h"
#include "common.h"

class IServerResponse
{
public:
    virtual void handleResponse(IClientDataManager * clientDataManager) = 0;
    virtual ParseStatus parsePayload(const unsigned char* buffer) = 0;
    virtual const std::string  getResponseText() const  = 0;
    virtual bool isResponseRegisterSuccess() const = 0;
    virtual ~IServerResponse(){}
};


class ServerResponseInstanceFactory
{
public:
    static IServerResponse* CreateServerResponseInstance(const unsigned char* buffer);
};
