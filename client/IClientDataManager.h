#pragma once
#include "ClientDataFromServer.h"
#include <vector>


class IClientDataManager
{
public:
	//our client information.
	virtual std::string getPrivateKey() const = 0;
	virtual std::string getClientId() const = 0;
	virtual std::string getUserName() const = 0;

	virtual void setPrivateKey(const std::string& privateKey) = 0;
	virtual void setClientId(const std::string& clientId) = 0;
	virtual void setUserName(const std::string& userName) = 0;


	//other clients information
	virtual std::string getClientPublicKey(const std::string& clientId) const  = 0;
	virtual std::string getClientSymmetricKey(const std::string& clientId, bool throwOnNonExist = true) const  = 0;
	virtual std::string getClientClientId(const std::string& userName) const = 0;
	virtual std::string getClientUserName(const std::string& clientId) const  = 0;

	virtual void insertClientsData(std::vector<ClientDataFromServer*>& clientDataFromServer) = 0;
	virtual void addPublicKey(const std::string& clientId, std::string& publicKey) = 0;
	virtual void addSymmetricKey(const std::string& clientId, std::string& symmetricKey) = 0;

	virtual ~IClientDataManager(){}
};