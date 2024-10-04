#pragma once
#include "IClientDataManager.h"
#include <map>

class ClientData
{
private:
    std::string publicKey;
    std::string symmetricKey;
    const std::string userName;
    const std::string clientId;

public:
    ClientData(const std::string& in_userName, const std::string & in_clientId);
    ~ClientData();

    const std::string getPublicKey() const;
    const std::string getSymmetricKey() const;

    void setPublicKey(std::string& in_publicKey);    
    void setSymmetricKey(std::string& in_symmetricKey);

    const std::string getUserName() const;
};


class ClientInMemoryDataManager : public IClientDataManager
{
private:
    std::string privateKey;
    std::string clientId;
    std::string userName;

	std::map<std::string/*userName*/, std::string/*clientId*/> userNameToClientId;
	std::map<std::string/*clientId*/, ClientData*> clientIdToClientDataMap;

public:
    ClientInMemoryDataManager(); 
    virtual ~ClientInMemoryDataManager();
    //current client data
    virtual std::string getPrivateKey() const;
    virtual std::string getClientId() const;
    virtual std::string getUserName() const;

    virtual void setPrivateKey(const std::string & privateKey);
    virtual void setClientId(const std::string& clientId);
    virtual void setUserName(const std::string& userName);

	virtual std::string getClientPublicKey(const std::string& clientId) const;
    virtual std::string getClientSymmetricKey(const std::string& clientId, bool throwOnNonExist = true) const;
    virtual std::string getClientClientId(const std::string& userName) const;
    virtual std::string getClientUserName(const std::string& clientId) const;

    virtual void insertClientsData(std::vector <ClientDataFromServer*>& clientDataFromServer);
    virtual void addPublicKey(const std::string& clientId, std::string& publicKey);
    virtual void addSymmetricKey(const std::string& clientId, std::string& symmetricKey);

private:
	ClientData* getClientData(const std::string& clientId) const;

};