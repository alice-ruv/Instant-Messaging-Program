#pragma once
#include "IUserCommandParser.h"


class UserCommandConsoleParser : public IUserCommandParser {

private:
	const std::string userManu = "MessageU client at your service.\n" +
		std::string("\n") +
		std::string("110) Register\n") +
		std::string("120) Request for clients list\n") +
		std::string("130) Request for public key\n") +
		std::string("140) Request for waiting messages\n") +
		std::string("150) Send a text message\n") +
		std::string("151) Send a request for symmetric key\n") +
		std::string("152) Send your symmetric key\n") +
		std::string("153) Send a file\n") +
		std::string("0) Exit client\n") +
		std::string("?\n");

	const std::string userNameRequest = "Please enter user name:\n";
	const std::string textMessageRequest = "Please enter text message:\n";
	const std::string filePathRequest = "Please enter full file path:\n";


public:
	virtual ~UserCommandConsoleParser() {}
	virtual IUserCommand* getNextCommand(bool allowRegisterCommand) const;
	virtual void displayCommandResponse(const  std::string & responseText) const;

private:
	void assertUserName(const std::string& userName) const;
	std::string readUserNameFromConsole() const;
};
