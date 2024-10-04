#include "UserCommandConsoleParser.h"
#include <iostream>


void UserCommandConsoleParser::assertUserName(const std::string& userName) const
{
	if (userName.size() > CommonConsts::USER_NAME_BYTE_SIZE - 1)
	{
		throw std::exception("user name length exceeded max size");
	}
}


std::string UserCommandConsoleParser::readUserNameFromConsole() const
{
	std::string userName;
	std::cout << userNameRequest;
	std::getline(std::cin, userName);
	assertUserName(userName);
	return userName;
}


void UserCommandConsoleParser::displayCommandResponse(const  std::string & responseText) const
{
	std::cout << responseText << std::endl << std::endl;
}


IUserCommand* UserCommandConsoleParser::getNextCommand(bool allowRegisterCommand) const
{
	
	std::string strCode;
	std::cout << userManu;
	std::getline(std::cin, strCode);

	if (strCode.length() != 3 && strCode.length() != 1)
	{
		throw std::exception("unknown command");
	}

	int code = std::stoi(strCode);
	UserCommandCode userCommandCode = (UserCommandCode)code;

	switch (userCommandCode)
	{
		case REGISTER_COMMAND:
		{
			if (!allowRegisterCommand)
			{
				throw std::exception("user has been already registered");
			}
			std::string registerUserName = readUserNameFromConsole();
			return new RegisterUserCommand(registerUserName);
		}
		case REQUEST_FOR_CLIENTS_LIST_COMMAND:
		{
			return new ClientsListUserCommand();
		}
		case REQUEST_FOR_PUBLIC_KEY_COMMAND:
		{
			std::string targetUserName = readUserNameFromConsole();
			return new RequestPublicKeyUserCommand(targetUserName);
		}
		case REQUEST_FOR_WAITING_MESSAGES_COMMAND:
		{
			return new RequestWaitingMessagesUserCommand();
		}
		case SEND_TEXT_MESSAGE_COMMAND:
		{
			std::string targetUserName = readUserNameFromConsole();
			std::cout << textMessageRequest;
			std::string textMessage;
			std::getline(std::cin, textMessage);
			return new SendTextMessageUserCommand(targetUserName, textMessage);
		}
		case SEND_REQUEST_FOR_SYMMETRIC_KEY_COMMAND:
		{
			std::string targetUserName = readUserNameFromConsole();
			return new SendRequestSymmetricKeyUserCommand(targetUserName);
		}
		case SEND_YOUR_SYMMETRIC_KEY_COMMAND:
		{
			std::string targetUserName = readUserNameFromConsole();
			return new SendSymmetricKeyUserCommand(targetUserName);
		};
		case SEND_FILE_COMMAND:
		{
			std::string targetUserName = readUserNameFromConsole();
			std::cout << filePathRequest;
			std::string filePath;
			std::getline(std::cin, filePath);
			return new SendFileUserCommand(targetUserName, filePath);
		};
		case EXIT_CLIENT_COMMAND:
		{
			return new ExitClientUserCommand();
		}
		default:
			break;
	}
	
	throw std::exception("not valid user command code");
}