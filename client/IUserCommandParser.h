#pragma once
#include "UserCommand.h"

class IUserCommandParser
{
public:
	virtual IUserCommand* getNextCommand(bool allowRegisterCommand) const = 0;
	virtual void displayCommandResponse(const std::string & responseText) const  = 0;

	virtual ~IUserCommandParser(){}
};