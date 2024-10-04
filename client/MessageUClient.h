#pragma once
#include "IClientDataManager.h"
#include "IUserCommandParser.h"
#include "INetworkClientManager.h"	

class MessageUClient
{
private:
	IClientDataManager* clientDataManager;
	IUserCommandParser * userCommandParser;
	INetworkClientManager* networkClientManager;
	bool isUserRegistered;

public:
	MessageUClient();
	~MessageUClient();
	void startClient();

private:
	static void readServerInfo(std::string& out_serverPort, std::string& out_serverUrl);
	void init();
};