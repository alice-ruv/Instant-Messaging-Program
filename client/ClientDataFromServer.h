#pragma once
#include <string>

class ClientDataFromServer
{
private:
    const std::string clientId;
    const std::string clientName;

public:
    ClientDataFromServer(std::string& in_clientId, std::string& in_clientName);
    ~ClientDataFromServer();

    const std::string getClientId() const;
    const std::string getClientName() const;
};