#include "ClientDataFromServer.h"


ClientDataFromServer::ClientDataFromServer(std::string& in_clientId, std::string& in_clientName) : clientId(in_clientId), clientName(in_clientName) {}


ClientDataFromServer::~ClientDataFromServer() {}


const std::string ClientDataFromServer::getClientId() const 
{
    return clientId;
}


const std::string ClientDataFromServer::getClientName() const 
{
    return clientName;
}