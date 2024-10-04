#include "TCPNetworkClientManager.h"
#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;


TCPNetworkClientManager::TCPNetworkClientManager(std::string& in_serverPort, std::string& in_serverUrl) 
    :serverPort(in_serverPort)
    ,serverUrl(in_serverUrl) {}


TCPNetworkClientManager::~TCPNetworkClientManager() {}


void TCPNetworkClientManager::clear(unsigned char* buffer)
{
    for (unsigned int i = 0; i < CommonConsts::BUFFER_PARTITION_SIZE; i++)
    {
        buffer[i] = '\0';
    }
}


IServerResponse* TCPNetworkClientManager::HandleRequest(IServerRequest& request) const
{
    IServerResponse* response = nullptr;
    try
    {
        boost::asio::io_context io_context;
        tcp::socket s(io_context);
        tcp::resolver resolver(io_context);
        boost::asio::connect(s, resolver.resolve(serverUrl, serverPort));
        unsigned char data[CommonConsts::BUFFER_PARTITION_SIZE];
        clear(data);
        ParseStatus parseStatus = ParseStatus::UNDONE;

        do
        {
            parseStatus = request.prepareRequestBuffer(data);
            boost::asio::write(s, boost::asio::buffer(data, CommonConsts::BUFFER_PARTITION_SIZE));
            clear(data);
        } 
        while (parseStatus == ParseStatus::UNDONE);

        parseStatus = ParseStatus::UNDONE;

        do
        {
            boost::system::error_code error;
            size_t length = boost::asio::read(s, boost::asio::buffer(data, CommonConsts::BUFFER_PARTITION_SIZE), error);

            if (error == boost::asio::error::eof)
            {
                throw boost::system::system_error(error);
                // Connection closed by peer.
                //error here because we expected more data
            }
            else if (error)
                throw boost::system::system_error(error); // Some other error.

            if (response == nullptr)
            {
                response = ServerResponseInstanceFactory::CreateServerResponseInstance(data);
                parseStatus = response->parsePayload(data);
            }
            else
            {
                parseStatus = response->parsePayload(data);
            }

            clear(data);
        } 
        while(parseStatus == ParseStatus::UNDONE);

        return response;
    }
    catch (const std::exception& e)
    {
        if (response != nullptr)
        {
            delete response;
        }

        throw e;
    }
}
