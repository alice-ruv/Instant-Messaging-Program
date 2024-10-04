#include "MessageUClient.h"
#include "ClientInMemoryDataManager.h"
#include "UserCommandConsoleParser.h"
#include "TCPNetworkClientManager.h"
#include "RSAWrapper.h"
#include "Base64Wrapper.h"
#include <fstream>
#include <filesystem>
#include <sstream>
#include <filesystem>
#include <boost/asio/ip/address.hpp>
#include <iostream>

namespace fs = std::filesystem;

class ExecutionContext
{
public:
    ExecutionContext() : userCommand(nullptr), serverRequest(nullptr), response(nullptr){}

    ~ExecutionContext()
    {
        if (userCommand != nullptr)
        {
            delete userCommand;
        }

        if (serverRequest != nullptr)
        {
            delete serverRequest;
        }

        if (response != nullptr)
        {
            delete response;
        }
    }

    IUserCommand* userCommand;
    IServerRequest* serverRequest;
    IServerResponse* response;
};


MessageUClient::MessageUClient() : isUserRegistered(false), networkClientManager(nullptr), clientDataManager(nullptr), userCommandParser(nullptr) {}


MessageUClient::~MessageUClient()
{
    if (networkClientManager != nullptr)
    {
        delete networkClientManager;
        networkClientManager = nullptr;
    }

    if (clientDataManager != nullptr)
    {
        delete clientDataManager;
        clientDataManager = nullptr;
    }

    if (userCommandParser != nullptr)
    {
        delete userCommandParser;
        userCommandParser = nullptr;
    }
}


void MessageUClient::init()
{
    std::string serverPort;
    std::string serverUrl;
    readServerInfo(serverPort, serverUrl);
    
    networkClientManager = new TCPNetworkClientManager(serverPort, serverUrl);
    clientDataManager = new ClientInMemoryDataManager();
    userCommandParser = new UserCommandConsoleParser();

    if (!std::filesystem::exists(CommonConsts::USER_INFO_FILE_NAME))
    {
        //file not exist, user not registerd, so creating private key
        RSAPrivateWrapper rsapriv;
        auto privateKey = Base64Wrapper::encode(rsapriv.getPrivateKey());
        clientDataManager->setPrivateKey(privateKey);
        return;
    }

    std::ifstream myInfoFile(CommonConsts::USER_INFO_FILE_NAME);
    std::string userName;
    std::string clientId;
    std::string privateKey;

    if (!std::getline(myInfoFile, userName))
    {
        throw std::exception("init failed");
    }
    else if(userName.length() >= CommonConsts::USER_NAME_BYTE_SIZE)
    {
        throw std::exception("user name too long");
    }

    if (!std::getline(myInfoFile, clientId))
    {
        throw std::exception("init failed");
    }
    else if (clientId.length() > (CommonConsts::CLIENT_ID_BYTE_SIZE * 2))
    {
        throw std::exception("client id too long");
    }

    privateKey = std::string((std::istreambuf_iterator<char>(myInfoFile)), std::istreambuf_iterator<char>());
    
    //Validation on private key. Throw an exception in case private key is invalid
    RSAPrivateWrapper rsaPrivateWrapper(Base64Wrapper::decode(privateKey));
    
    clientDataManager->setUserName(userName);
    clientDataManager->setClientId(clientId);
    clientDataManager->setPrivateKey(privateKey);
    isUserRegistered = true;
}


void MessageUClient::startClient()
{
    try
    {
        init();
    }

    catch (const std::exception& e)  
    {
        std::cerr << "Fatal error in init. Closing client. Exception: " << e.what() << std::endl;
        return;
    }

	while (true)
	{
        ExecutionContext executionContext;
		try
		{
            executionContext.userCommand = userCommandParser->getNextCommand(!isUserRegistered);

            if (executionContext.userCommand->isClientDone())
            {
                userCommandParser->displayCommandResponse(std::string("Client finished successfully.\nGoodbye!"));
                break;
            }

            executionContext.serverRequest = executionContext.userCommand->createServerRequest(clientDataManager);
            executionContext.response = networkClientManager->HandleRequest(*executionContext.serverRequest);
            executionContext.response->handleResponse(clientDataManager);
            auto responseText = executionContext.response->getResponseText();
            userCommandParser->displayCommandResponse(responseText);

            if (!isUserRegistered)
            {
                isUserRegistered = executionContext.response->isResponseRegisterSuccess();
            }
		}

		catch (const std::exception& e)
		{
            userCommandParser->displayCommandResponse(e.what());
		}

        catch (...)
        {
            userCommandParser->displayCommandResponse(std::string("unknown error"));
        }
	}
}


void MessageUClient::readServerInfo(std::string & out_serverPort, std::string& out_serverUrl)
{
    //when debugging in visual studio the current path is not where the exe is by default
    //assuming that the server.info file will be in current path location
    auto serverInfoFilePath = fs::current_path();
    serverInfoFilePath.append(CommonConsts::SERVER_INFO_FILE_NAME);

    if (!fs::exists(serverInfoFilePath))
    {
        std::cerr << serverInfoFilePath << " file not exist." << std::endl;
        throw std::exception("readServerInfo failed");
    }

    std::fstream myfile(serverInfoFilePath, std::ios_base::in);
    std::getline(myfile, out_serverUrl, ':');
    std::getline(myfile, out_serverPort);
    myfile.close(); 

    if (out_serverPort.empty() || std::find_if(out_serverPort.begin(),
        out_serverPort.end(), [](unsigned char c) { return !std::isdigit(c); }) != out_serverPort.end())
    {
        std::cerr << out_serverPort + " is not a valid string for port number" << std::endl;
        throw std::exception("readServerInfo failed");
    }

    int port_int = std::stoi(out_serverPort);

    if (port_int < CommonConsts::MIN_VALID_PORT_NUMBER || port_int > CommonConsts::MAX_VALID_PORT_NUMBER)
    {
        std::cerr << "Port number should be bigger than " << CommonConsts::MIN_VALID_PORT_NUMBER << " and not bigger than " << CommonConsts::MAX_VALID_PORT_NUMBER << std::endl;
        throw std::exception("readServerInfo failed");
    }

    boost::system::error_code ec;
    boost::asio::ip::address::from_string(out_serverUrl, ec);

    if (ec)
    {
        throw std::exception("readServerInfo failed");
    }
}