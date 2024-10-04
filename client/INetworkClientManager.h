#pragma once
#include "IServerRequest.h"
#include "IServerResponse.h"

class INetworkClientManager
{
public:
	virtual IServerResponse* HandleRequest(IServerRequest& request) const = 0;
	virtual ~INetworkClientManager(){}
};
