#pragma once
#include "INetworkClientManager.h"

class TCPNetworkClientManager : public INetworkClientManager
{
	std::string serverPort;
	std::string serverUrl;

public:
	TCPNetworkClientManager(std::string& in_serverPort, std::string& in_serverUrl);
	virtual ~TCPNetworkClientManager();
	virtual IServerResponse* HandleRequest(IServerRequest& request) const;

private:
	static void clear(unsigned char* buffer);
};