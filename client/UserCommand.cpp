#include "UserCommand.h"
#include "Base64Wrapper.h"
#include "RSAWrapper.h"
#include "AESWrapper.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include "ServerRequest.h"

namespace fs = std::filesystem;


IUserCommand::~IUserCommand() {}


UserCommandBase::~UserCommandBase() {}

bool UserCommandBase::isClientDone() const 
{ 
	return false; 
}


UserCommandSpecificUser::UserCommandSpecificUser(const std::string& in_targetUserName) : targetUserName(in_targetUserName) {}

UserCommandSpecificUser::~UserCommandSpecificUser() {}


RegisterUserCommand::RegisterUserCommand(const std::string& in_registerUserName) : registerUserName(in_registerUserName) {}

RegisterUserCommand::~RegisterUserCommand() {}


ClientsListUserCommand::ClientsListUserCommand() {}

ClientsListUserCommand::~ClientsListUserCommand() {}


RequestPublicKeyUserCommand::RequestPublicKeyUserCommand(const std::string& in_targetUserName) : UserCommandSpecificUser(in_targetUserName) {}

RequestPublicKeyUserCommand::~RequestPublicKeyUserCommand() {}


RequestWaitingMessagesUserCommand::RequestWaitingMessagesUserCommand() {}

RequestWaitingMessagesUserCommand::~RequestWaitingMessagesUserCommand() {}


SendTextMessageUserCommand::SendTextMessageUserCommand(const std::string& in_targetUserName, std::string& in_textMessage) 
	: UserCommandSpecificUser(in_targetUserName)
	, textMessage(in_textMessage) {}

SendTextMessageUserCommand::~SendTextMessageUserCommand() {}


SendRequestSymmetricKeyUserCommand::SendRequestSymmetricKeyUserCommand(const std::string& in_targetUserName) : UserCommandSpecificUser(in_targetUserName) {}

SendRequestSymmetricKeyUserCommand::~SendRequestSymmetricKeyUserCommand() {}

SendSymmetricKeyUserCommand::SendSymmetricKeyUserCommand(const std::string& in_targetUserName) : UserCommandSpecificUser(in_targetUserName) {}

SendSymmetricKeyUserCommand::~SendSymmetricKeyUserCommand() {}


SendFileUserCommand::SendFileUserCommand(const std::string& in_targetUserName, const std::string& in_filePath) : UserCommandSpecificUser(in_targetUserName), filePath(in_filePath) {}

SendFileUserCommand::~SendFileUserCommand() {}


ExitClientUserCommand::ExitClientUserCommand() {}

ExitClientUserCommand::~ExitClientUserCommand() {}


bool ExitClientUserCommand::isClientDone() const 
{ 
	return true; 
}


IServerRequest* RegisterUserCommand::createServerRequest(IClientDataManager* clientDataManager)
{
	clientDataManager->setUserName(registerUserName);
	auto privateKey = clientDataManager->getPrivateKey();
	RSAPrivateWrapper rsaPrivateWrapper(Base64Wrapper::decode(privateKey));
	auto publicKey = rsaPrivateWrapper.getPublicKey();
	std::string fakeId = "CB10D6FB5FD741F39BE43BC3F0B7564C";	// we must send a client id
	return new RegisterServerRequest(fakeId,CommonConsts::CLIENT_VERSION, registerUserName, publicKey);
}


IServerRequest* ClientsListUserCommand::createServerRequest(IClientDataManager* clientDataManager)
{
	auto clientID = clientDataManager->getClientId();
	return new ListClientsServerRequest(clientID, CommonConsts::CLIENT_VERSION);
}


IServerRequest* RequestPublicKeyUserCommand::createServerRequest(IClientDataManager* clientDataManager)
{
	auto clientID = clientDataManager->getClientId();
	auto targetClientId = clientDataManager->getClientClientId(targetUserName);
	return new GetPublicKeyServerRequest(clientID, CommonConsts::CLIENT_VERSION, targetClientId);
}


IServerRequest* RequestWaitingMessagesUserCommand::createServerRequest(IClientDataManager* clientDataManager)
{
	auto clientID = clientDataManager->getClientId();
	return new PullMessagesServerRequest(clientID, CommonConsts::CLIENT_VERSION);
}


IServerRequest* SendTextMessageUserCommand::createServerRequest(IClientDataManager* clientDataManager)
{
	auto clientID = clientDataManager->getClientId();
	auto targetClientId = clientDataManager->getClientClientId(targetUserName);
	auto symmetricKey = clientDataManager->getClientSymmetricKey(targetClientId);
	AESWrapper aes(reinterpret_cast<unsigned const char*>(symmetricKey.c_str()), (unsigned int)symmetricKey.size());
	auto encryptedTextMessage = aes.encrypt(textMessage.c_str(), (unsigned int)textMessage.length());

	return new SendMessageServerRequest(clientID, CommonConsts::CLIENT_VERSION, targetClientId, MessageType::SEND_TEXT_MESSAGE, encryptedTextMessage);
}


IServerRequest* SendRequestSymmetricKeyUserCommand::createServerRequest(IClientDataManager* clientDataManager)
{
	auto clientID = clientDataManager->getClientId();
	auto targetClientId = clientDataManager->getClientClientId(targetUserName);
	std::string emptyString("");

	return new SendMessageServerRequest(clientID, CommonConsts::CLIENT_VERSION, targetClientId, MessageType::REQUEST_SYMMETRIC_KEY, emptyString);
}


IServerRequest* SendSymmetricKeyUserCommand::createServerRequest(IClientDataManager* clientDataManager)
{
	auto clientID = clientDataManager->getClientId();
	auto targetClientId = clientDataManager->getClientClientId(targetUserName);
	auto symmetricKey = clientDataManager->getClientSymmetricKey(targetClientId, false);
	auto targetPublicKey = clientDataManager->getClientPublicKey(targetClientId);

	if (symmetricKey == "")
	{
		AESWrapper aes;
		auto keyBuffer = aes.getKey();
		symmetricKey = std::string(keyBuffer, keyBuffer + AESWrapper::DEFAULT_KEYLENGTH);
		clientDataManager->addSymmetricKey(targetClientId, symmetricKey);
	}

	RSAPublicWrapper rsapub(targetPublicKey);
	auto encryptedTextMessage = rsapub.encrypt(symmetricKey);

	return new SendMessageServerRequest(clientID, CommonConsts::CLIENT_VERSION, targetClientId, MessageType::SEND_SYMMETRIC_KEY, encryptedTextMessage);
}


IServerRequest* SendFileUserCommand::createServerRequest(IClientDataManager* clientDataManager)
{
	auto clientID = clientDataManager->getClientId();
	auto targetClientId = clientDataManager->getClientClientId(targetUserName);
	auto symmetricKey = clientDataManager->getClientSymmetricKey(targetClientId);
	AESWrapper aes(reinterpret_cast<unsigned const char*>(symmetricKey.c_str()), (unsigned int)symmetricKey.size());

	if (!std::filesystem::exists(filePath))
	{
		throw std::exception("file not found");
	}

	std::ifstream file(filePath, std::ios::in | std::ios::binary);
	std::noskipws(file);
	std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	auto encryptedTextMessage = aes.encrypt(fileContent.c_str(), (unsigned int)fileContent.length());

	return new SendMessageServerRequest(clientID, CommonConsts::CLIENT_VERSION, targetClientId, MessageType::SEND_FILE, encryptedTextMessage);
}


IServerRequest* ExitClientUserCommand::createServerRequest(IClientDataManager* clientDataManager)
{
	throw std::exception("not supposed to be called");
}