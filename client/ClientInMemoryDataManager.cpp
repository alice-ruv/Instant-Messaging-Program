#include "ClientInMemoryDataManager.h"
#include <algorithm>


ClientData::ClientData(const std::string& in_userName, const  std::string & in_clientId) 
	: userName(in_userName)
	, clientId(in_clientId)
	, publicKey("")
	, symmetricKey("") {}

ClientData::~ClientData() {}


const std::string ClientData::getPublicKey() const 
{
	return publicKey;
}


void ClientData::setPublicKey(std::string& in_publicKey) 
{
	publicKey = in_publicKey;
}

const std::string ClientData::getSymmetricKey() const {
	return symmetricKey;
}


void ClientData::setSymmetricKey(std::string& in_symmetricKey) 
{
	symmetricKey = in_symmetricKey;
}


const std::string ClientData::getUserName() const
{
	return userName;
}


ClientInMemoryDataManager::ClientInMemoryDataManager() : privateKey(""), clientId(""), userName("") {}


ClientInMemoryDataManager::~ClientInMemoryDataManager()
{
	userNameToClientId.clear();
	for (auto& [clientId, clientData] : clientIdToClientDataMap) 
	{
		delete clientData;
	}
	clientIdToClientDataMap.clear();
}


std::string ClientInMemoryDataManager::getPrivateKey() const
{
	if (privateKey == "")
	{
		throw std::exception("no private key for client");
	}
	return privateKey;
}


std::string ClientInMemoryDataManager::getClientId() const
{
	if (clientId == "")
	{
		throw std::exception("no client id for client");
	}
	return clientId;
}


std::string ClientInMemoryDataManager::getUserName() const
{
	if (userName == "")
	{
		throw std::exception("no client id for client");
	}
	return userName;
}


void ClientInMemoryDataManager::setPrivateKey(const std::string& in_privateKey)
{
	privateKey = in_privateKey;
}


void ClientInMemoryDataManager::setClientId(const std::string& in_clientId)
{
	clientId = in_clientId;
}


void ClientInMemoryDataManager::setUserName(const std::string& in_userName)
{
	userName = in_userName;
}


ClientData* ClientInMemoryDataManager::getClientData(const std::string& clientId) const
{
	auto it = clientIdToClientDataMap.find(clientId);
	if (it == clientIdToClientDataMap.end())
	{
		throw std::exception("client wasn't found\n");
	}
	return it->second;
}


std::string ClientInMemoryDataManager::getClientPublicKey(const std::string& clientId) const
{
	auto clientData = getClientData(clientId);
	auto publicKey = clientData->getPublicKey();
	if (publicKey == "")
	{
		throw std::exception("no public key for client");
	}
	return publicKey;
}


std::string ClientInMemoryDataManager::getClientSymmetricKey(const std::string& clientId, bool throwOnNonExist) const
{
	auto clientData = getClientData(clientId);
	auto symmetricKey = clientData->getSymmetricKey();
	if (throwOnNonExist && (symmetricKey == ""))
	{
		throw std::exception("no symmetric key for client\n");
	}
	return symmetricKey;
}


std::string ClientInMemoryDataManager::getClientClientId(const std::string& userName) const
{
	auto it = userNameToClientId.find(userName);
	if (it == userNameToClientId.end())
	{
		throw std::exception("client id wasn't found\n");
	}
	return it->second;
}


std::string ClientInMemoryDataManager::getClientUserName(const std::string& clientId) const
{
	auto clientData = getClientData(clientId);
	auto userName = clientData->getUserName();
	return userName;
}


void ClientInMemoryDataManager::insertClientsData(std::vector<ClientDataFromServer*>& clientDataFromServer)
{
	std::for_each(clientDataFromServer.begin(), clientDataFromServer.end(),
		[this](auto & clientDataFromServer)
		{
			auto clientId = clientDataFromServer->getClientId();
			auto it =  clientIdToClientDataMap.find(clientId);
			if (it == clientIdToClientDataMap.end())
			{
				auto clientData = new ClientData(clientDataFromServer->getClientName(), clientId);
				clientIdToClientDataMap.insert(std::pair<std::string, ClientData*>(clientId, clientData));
				userNameToClientId.insert(std::pair<std::string, std::string>(clientDataFromServer->getClientName(), clientId));
			}
		}
	);
}


void ClientInMemoryDataManager::addPublicKey(const std::string& clientId, std::string & publicKey)
{
	auto clientData = getClientData(clientId);
	clientData->setPublicKey(publicKey);

}


void ClientInMemoryDataManager::addSymmetricKey(const std::string& clientId, std::string& symmetricKey)
{
	auto clientData = getClientData(clientId);
	clientData->setSymmetricKey(symmetricKey);
}