#pragma once
#include "IClientDataManager.h"
#include "IServerRequest.h" 


enum UserCommandCode
{
    REGISTER_COMMAND = 110,
    REQUEST_FOR_CLIENTS_LIST_COMMAND = 120,
    REQUEST_FOR_PUBLIC_KEY_COMMAND = 130,
    REQUEST_FOR_WAITING_MESSAGES_COMMAND = 140,
    SEND_TEXT_MESSAGE_COMMAND = 150,
    SEND_REQUEST_FOR_SYMMETRIC_KEY_COMMAND = 151,
    SEND_YOUR_SYMMETRIC_KEY_COMMAND = 152,
    SEND_FILE_COMMAND = 153,
    EXIT_CLIENT_COMMAND = 0
};


class IUserCommand
{

public:
    virtual IServerRequest* createServerRequest(IClientDataManager * clientDataManager) = 0;
    virtual bool isClientDone() const = 0;
    virtual ~IUserCommand();
};


class UserCommandBase : public IUserCommand
{
public:
    virtual bool isClientDone() const;
    virtual ~UserCommandBase();
};


class UserCommandSpecificUser : public UserCommandBase
{
protected:
    const std::string targetUserName;

protected:
    UserCommandSpecificUser(const std::string& in_targetUserName);

public:
    virtual ~UserCommandSpecificUser();
};


class RegisterUserCommand : public UserCommandBase
{
private:
    std::string registerUserName;

public:
    RegisterUserCommand(const std::string& in_registerUserName);
    virtual ~RegisterUserCommand();

    virtual IServerRequest* createServerRequest(IClientDataManager* clientDataManager);
};

class ClientsListUserCommand : public UserCommandBase
{
public:
    ClientsListUserCommand();
    virtual ~ClientsListUserCommand();

    virtual IServerRequest* createServerRequest(IClientDataManager* clientDataManager);
};


class RequestPublicKeyUserCommand : public UserCommandSpecificUser
{
public:
    RequestPublicKeyUserCommand(const std::string& in_targetUserName);
    virtual ~RequestPublicKeyUserCommand();
    virtual IServerRequest* createServerRequest(IClientDataManager* clientDataManager);
};


class RequestWaitingMessagesUserCommand :public UserCommandBase
{
public:
    RequestWaitingMessagesUserCommand();
    virtual ~RequestWaitingMessagesUserCommand();
    virtual IServerRequest* createServerRequest(IClientDataManager* clientDataManager);
};


class SendTextMessageUserCommand : public UserCommandSpecificUser
{
private:
     std::string textMessage;
public:
    SendTextMessageUserCommand (const std::string& in_targetUserName, std::string& in_textMessage);
    virtual ~SendTextMessageUserCommand();

    virtual IServerRequest* createServerRequest(IClientDataManager* clientDataManager);
};


class SendRequestSymmetricKeyUserCommand : public UserCommandSpecificUser
{
public:
    SendRequestSymmetricKeyUserCommand(const std::string& in_targetUserName);
    virtual ~SendRequestSymmetricKeyUserCommand();
    virtual IServerRequest* createServerRequest(IClientDataManager* clientDataManager);

};


class SendSymmetricKeyUserCommand : public UserCommandSpecificUser
{
public:
    SendSymmetricKeyUserCommand(const std::string& in_targetUserName);
    virtual ~SendSymmetricKeyUserCommand();

    virtual IServerRequest* createServerRequest(IClientDataManager* clientDataManager);
};


class SendFileUserCommand : public UserCommandSpecificUser
{
private:
    const std::string filePath;

public:
    SendFileUserCommand(const std::string& in_targetUserName, const std::string& in_filePath);
    virtual ~SendFileUserCommand();

    virtual IServerRequest* createServerRequest(IClientDataManager* clientDataManager);
};


class ExitClientUserCommand : public UserCommandBase
{
public:
    ExitClientUserCommand();
    virtual ~ExitClientUserCommand();

    virtual IServerRequest* createServerRequest(IClientDataManager* clientDataManager);
    virtual bool isClientDone() const;
};