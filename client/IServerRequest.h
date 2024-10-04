#pragma once
#include "common.h"

class IServerRequest
{
public:
	virtual ParseStatus prepareRequestBuffer(unsigned char* buffer) = 0;
	virtual ~IServerRequest() {};
};


