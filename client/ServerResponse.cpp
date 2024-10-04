#include "ServerResponse.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include "AESWrapper.h"
#include "RSAWrapper.h"
#include "Base64Wrapper.h"
#include <windows.h>
#include <sstream>


ServerResponseBase::ServerResponseBase(unsigned int in_version, unsigned int in_payloadSize) 
	: version(in_version)
	, payloadSize(in_payloadSize)
	, responseText("") {}


bool ServerResponseBase::isResponseRegisterSuccess() const
{
	return false;
}


const std::string ServerResponseBase::getResponseText() const
{
	return responseText;
}


ServerResponseBase::~ServerResponseBase() {}


GeneralErrorResponse::GeneralErrorResponse(unsigned int in_version, unsigned int in_payloadSize) : ServerResponseBase(in_version, in_payloadSize) {}


GeneralErrorResponse::~GeneralErrorResponse() {}


BigResponse::BigResponse(unsigned int in_version, unsigned int in_payloadSize) : ServerResponseBase(in_version, in_payloadSize), payloadCopiedSize(0)
{
	payload = (in_payloadSize != 0) ? new unsigned char[in_payloadSize] : nullptr;
}

BigResponse::~BigResponse()
{
	if (payload != nullptr)
	{
		delete[] payload;
		payload = nullptr;
	}
}


RegisterSuccessResponse::RegisterSuccessResponse(unsigned int in_version, unsigned int in_payloadSize) 
	: ServerResponseBase(in_version, in_payloadSize)
	, clientId("")
	, isCompletedSuccess(false) {}


bool RegisterSuccessResponse::isResponseRegisterSuccess() const
{
	return isCompletedSuccess;
}

RegisterSuccessResponse::~RegisterSuccessResponse() {}


ListClientsSuccessResponse::ListClientsSuccessResponse(unsigned int in_version, unsigned int in_payloadSize) : BigResponse(in_version, in_payloadSize)
{
	if (payloadSize % (CommonConsts::CLIENT_ID_BYTE_SIZE + CommonConsts::USER_NAME_BYTE_SIZE) != 0)
	{
		throw std::exception("invalid payload size for ListClientsSuccessResponse");
	}
	numOfClients = payloadSize / (CommonConsts::CLIENT_ID_BYTE_SIZE + CommonConsts::USER_NAME_BYTE_SIZE);
	if (numOfClients != 0)
	{
		clientDataFromServerList.reserve(numOfClients);
	}
}


void ListClientsSuccessResponse::insertClientDataFromServer(ClientDataFromServer* clientDataFromServer)
{
	clientDataFromServerList.push_back(clientDataFromServer);
}


unsigned int ListClientsSuccessResponse::getNumOfClients() const
{
	return numOfClients;
}


ListClientsSuccessResponse::~ListClientsSuccessResponse()
{
	std::for_each(clientDataFromServerList.begin(), clientDataFromServerList.end(),
		[this](auto& ClientDataFromServer)
		{
			delete ClientDataFromServer;
		}
	);
	clientDataFromServerList.clear();
}


PublicKeySuccessResponse::PublicKeySuccessResponse(unsigned int in_version, unsigned int in_payloadSize) 
	: ServerResponseBase(in_version, in_payloadSize)
	, clientId("")
	, publicKey("") {}


PublicKeySuccessResponse::~PublicKeySuccessResponse() {}


MessageSentSuccessResponse::MessageSentSuccessResponse(unsigned int in_version, unsigned int in_payloadSize) 
	: ServerResponseBase(in_version, in_payloadSize)
	, clientId("")
	, messageId(0) {}


MessageSentSuccessResponse::~MessageSentSuccessResponse() {}


PullMessagesSuccessResponse::PullMessagesSuccessResponse(unsigned int in_version, unsigned int in_payloadSize) : BigResponse(in_version, in_payloadSize) {}


PullMessagesSuccessResponse::~PullMessagesSuccessResponse() 
{
	std::for_each(messageDataFromServerList.begin(), messageDataFromServerList.end(), [this](auto& mssageDataFromServer)
		{
			delete mssageDataFromServer;
		}
	);
	messageDataFromServerList.clear();
}


void PullMessagesSuccessResponse::insertMessageDataFromServer(MessageDataFromServer* messageDataFromServer)
{
	messageDataFromServerList.push_back(messageDataFromServer);
}


ParseStatus RegisterSuccessResponse::parsePayload(const unsigned char* buffer)
{
	assertPayloadSize(CommonConsts::CLIENT_ID_BYTE_SIZE, payloadSize);
	clientId = getClientIdFromBuffer(buffer + CommonConsts::CLIENT_ID_OFFSET_IN_RESPONSE_BUFFER);
	return ParseStatus::DONE;
}


void PullMessagesSuccessResponse::parseDataList()
{
	unsigned int currentOffset = 0;

	while (currentOffset < payloadSize)
	{
		if ((payloadSize - currentOffset) < (CommonConsts::CLIENT_ID_BYTE_SIZE + CommonConsts::MESSAGE_ID_BYTE_SIZE + CommonConsts::MESSAGE_TYPE_BYTE_SIZE + CommonConsts::CONTENT_SIZE_BYTE_SIZE))
		{
			throw std::exception("invalid payload size");
		}

		std::string clientId = getClientIdFromBuffer(payload + currentOffset);
		currentOffset = currentOffset + CommonConsts::CLIENT_ID_BYTE_SIZE;
		unsigned int messageId = (unsigned int)getNumFromBuffer(payload + currentOffset, CommonConsts::MESSAGE_ID_BYTE_SIZE);
		currentOffset = currentOffset + CommonConsts::MESSAGE_ID_BYTE_SIZE;
		MessageType messageType = (MessageType)getNumFromBuffer(payload + currentOffset, CommonConsts::MESSAGE_TYPE_BYTE_SIZE);
		currentOffset = currentOffset + CommonConsts::MESSAGE_TYPE_BYTE_SIZE;
		unsigned int messageSize = (unsigned int)getNumFromBuffer(payload + currentOffset, CommonConsts::CONTENT_SIZE_BYTE_SIZE);
		currentOffset = currentOffset + CommonConsts::CONTENT_SIZE_BYTE_SIZE;

		if ((payloadSize - currentOffset) < messageSize)
		{
			throw std::exception("invalid payload size");
		}

		MessageDataFromServer* messageDataFromServer = new MessageDataFromServer(clientId, messageId, messageType, messageSize, payload + currentOffset);
		currentOffset = currentOffset + messageSize;
		insertMessageDataFromServer(messageDataFromServer);
	}
}


void ListClientsSuccessResponse::parseDataList()
{
	for (unsigned int i = 0; i < numOfClients; i++)
	{
		auto offset = payload + i * (unsigned long long int)CommonConsts::CLIENT_DATA_FROM_SERVER_SIZE_IN_RESPONSE_BUFFER;
		std::string clientId = getClientIdFromBuffer(offset);
		std::string clientName(offset + CommonConsts::CLIENT_ID_BYTE_SIZE, offset + CommonConsts::CLIENT_ID_BYTE_SIZE + CommonConsts::USER_NAME_BYTE_SIZE);
		clientName = std::string(clientName.c_str());
		ClientDataFromServer* clientDataFromServer = new ClientDataFromServer(clientId, clientName);
		insertClientDataFromServer(clientDataFromServer);
	}
}


ParseStatus BigResponse::parsePayload(const unsigned char* buffer)
{
	if (payloadSize == 0)
	{
		return ParseStatus::DONE;
	}

	unsigned int bufferStartIndex = (payloadCopiedSize != 0) ? 0 : CommonConsts::PAYLOAD_OFFSET_IN_RESPONSE_BUFFER;
	unsigned int numOfBytesToCopy = 0;

	if ((payloadSize - payloadCopiedSize) > (CommonConsts::BUFFER_PARTITION_SIZE - bufferStartIndex))
	{
		numOfBytesToCopy = CommonConsts::BUFFER_PARTITION_SIZE - bufferStartIndex;
	}
	else
	{
		numOfBytesToCopy = payloadSize - payloadCopiedSize;
	}

	std::copy(buffer + bufferStartIndex, buffer + bufferStartIndex + numOfBytesToCopy, payload + payloadCopiedSize);

	payloadCopiedSize += numOfBytesToCopy;

	if (payloadCopiedSize != payloadSize)
	{
		return ParseStatus::UNDONE;
	}

	parseDataList();
	return ParseStatus::DONE;
}


ParseStatus PublicKeySuccessResponse::parsePayload(const unsigned char* buffer)
{
	assertPayloadSize(CommonConsts::CLIENT_ID_BYTE_SIZE + CommonConsts::PUBLIC_KEY_BYTE_SIZE, payloadSize);
	clientId = getClientIdFromBuffer(buffer + CommonConsts::CLIENT_ID_OFFSET_IN_RESPONSE_BUFFER);
	auto offset = buffer + CommonConsts::PUBLIC_KEY_OFFSET_IN_RESPONSE_BUFFER;
	publicKey = std::string(offset, offset + CommonConsts::PUBLIC_KEY_BYTE_SIZE);

	return ParseStatus::DONE;
}


ParseStatus MessageSentSuccessResponse::parsePayload(const unsigned char* buffer)
{
	assertPayloadSize(CommonConsts::CLIENT_ID_BYTE_SIZE + CommonConsts::MESSAGE_ID_BYTE_SIZE, payloadSize);
	clientId = getClientIdFromBuffer(buffer + CommonConsts::CLIENT_ID_OFFSET_IN_RESPONSE_BUFFER);
	messageId = (unsigned int)getNumFromBuffer(buffer + CommonConsts::MESSAGE_ID_OFFSET_IN_RESPONSE_BUFFER, CommonConsts::MESSAGE_ID_BYTE_SIZE);

	return ParseStatus::DONE;
}


ParseStatus GeneralErrorResponse::parsePayload(const unsigned char* buffer)
{
	assertPayloadSize(0, payloadSize);
	return ParseStatus::DONE;
}


IServerResponse* ServerResponseInstanceFactory::CreateServerResponseInstance(const unsigned char* buffer)
{
	//parsing header
	unsigned int version = (unsigned int)ServerResponseBase::getNumFromBuffer(buffer + CommonConsts::VERSION_OFFSET_IN_RESPONSE_BUFFER, CommonConsts::VERSION_BYTE_SIZE);
	ResponseCode responseCode = (ResponseCode)ServerResponseBase::getNumFromBuffer(buffer + CommonConsts::RESPONSE_CODE_OFFSET_IN_RESPONSE_BUFFER, CommonConsts::RESPONSE_CODE_BYTE_SIZE);
	unsigned int payloadSize = (unsigned int)ServerResponseBase::getNumFromBuffer(buffer + CommonConsts::PAYLOAD_SIZE_OFFSET_IN_RESPONSE_BUFFER, CommonConsts::PAYLOAD_SIZE_BYTE_SIZE);

	switch (responseCode)
	{
		case REGISTER_SUCCESS:
		{
			return new RegisterSuccessResponse(version, payloadSize);
		}
		case LIST_CLIENTS_SUCCESS:
		{
			return new ListClientsSuccessResponse(version, payloadSize);
		}
		case PUBLIC_KEY_SUCCESS:
		{
			return new PublicKeySuccessResponse(version, payloadSize);
		}
		case MESSAGE_SENT_SUCCESS:
		{
			return new MessageSentSuccessResponse(version, payloadSize);
		}
		case MESSAGES_PULLED_SUCCESS:
		{
			return new PullMessagesSuccessResponse(version, payloadSize);
		}
		case GENERAL_SERVER_ERROR:
			return new GeneralErrorResponse(version, payloadSize);
		default:
		{

		}
	}

	throw std::exception("Not valid response code");
}


unsigned long long int ServerResponseBase::getNumFromBuffer(const unsigned char* buffer, int numOfBytes)
{
	unsigned num = 0;

	for (int i = 0; i < numOfBytes; i++)
	{
		num += buffer[i] << (8 * i);
	}

	return num;
}


void ServerResponseBase::assertPayloadSize(unsigned int expectedPayloadSize, unsigned int payloadSize)
{
	if (expectedPayloadSize != payloadSize)
	{
		throw std::exception("payload size is not the expected payload size");
	}
}


std::string ServerResponseBase::getClientIdFromBuffer(const unsigned char* buffer)
{

	std::stringstream clientId;

	for (unsigned int i = 0; i < CommonConsts::CLIENT_ID_BYTE_SIZE; i++)
	{
		if (buffer[i] > 0xF)
		{
			clientId << std::hex << (unsigned int)buffer[i];
		}
		else //below 16 we have only one char and we need to add 0 before the number
		{
			clientId << "0" << std::hex << (unsigned int)buffer[i];
		}

	}
	return clientId.str();
}


void GeneralErrorResponse::handleResponse(IClientDataManager* clientDataManager)
{
	responseText = CommonConsts::ERROR_MESSAGE;
}


void RegisterSuccessResponse::handleResponse(IClientDataManager* clientDataManager)
{
	std::ofstream out( CommonConsts::USER_INFO_FILE_NAME, std::ofstream::out);

	out << clientDataManager->getUserName() << std::endl;
	out << clientId << std::endl;
	out << clientDataManager->getPrivateKey();
	out.close();

	clientDataManager->setClientId(clientId);
	responseText = "Registration finished successfully.\n";
	isCompletedSuccess = true;
}


void ListClientsSuccessResponse::handleResponse(IClientDataManager* clientDataManager)
{
	clientDataManager->insertClientsData(clientDataFromServerList);
	responseText = "----clients list from server----\n";

	std::for_each(clientDataFromServerList.begin(), clientDataFromServerList.end(),
		[this](auto& clientDataFromServer)
		{
			responseText += "client id: " + clientDataFromServer->getClientId() + "\n";
			responseText += "user name: " + clientDataFromServer->getClientName() + "\n";
			responseText += "\n";
		}
	);

	responseText += "----end of clients list from server----\n";
}


void PublicKeySuccessResponse::handleResponse(IClientDataManager* clientDataManager)
{
	clientDataManager->addPublicKey(clientId, publicKey);
	responseText = "Public key retrieved successfully.\n";
}


void MessageSentSuccessResponse::handleResponse(IClientDataManager* clientDataManager)
{
	responseText = "Message sent successfully.\n";
}


std::string PullMessagesSuccessResponse::decrypteMessage(const MessageDataFromServer * messageDataFromServer, IClientDataManager* clientDataManager)
{
	std::string symmetricKey = clientDataManager->getClientSymmetricKey(messageDataFromServer->getClientId());	
	std::string contentEncrypted = messageDataFromServer->getMessageContent();
	AESWrapper aes((unsigned char*)symmetricKey.c_str(), (unsigned int)symmetricKey.length());
	return aes.decrypt(contentEncrypted.c_str(), (unsigned int)contentEncrypted.length());
}


void PullMessagesSuccessResponse::handleResponse(IClientDataManager* clientDataManager)
{
	if (messageDataFromServerList.size() == 0)
	{
		responseText = "No new messages for you.\n";
		return;
	}

	std::for_each(messageDataFromServerList.begin(), messageDataFromServerList.end(),
		[this, clientDataManager](auto& messageDataFromServer)
		{
			std::string userName;
			try
			{
				 userName = clientDataManager->getClientUserName(messageDataFromServer->getClientId());

			}
			catch (const std::exception&)
			{
				responseText += "unknown user id\n";
				return;
			}

			responseText += "From " + userName + "\n";
			responseText += "Content:\n";

			switch (messageDataFromServer->getMessageType())
			{
				case REQUEST_SYMMETRIC_KEY:
				{
					responseText += "Request for symmetric key\n";
					break;
				}
				case SEND_SYMMETRIC_KEY:
				{
					try
					{
						std::string privateKey = clientDataManager->getPrivateKey();
						std::string symmetricKeyEncrypted = messageDataFromServer->getMessageContent();
						RSAPrivateWrapper rsapriv(Base64Wrapper::decode(privateKey));
						std::string symmetricKeyDecrypted = rsapriv.decrypt(symmetricKeyEncrypted);
						clientDataManager->addSymmetricKey(messageDataFromServer->getClientId(), symmetricKeyDecrypted);
						responseText += "Symmetric key received\n";
					}
					catch (const std::exception&)
					{
						responseText += "can't decrypt message\n";
					}
					break;
				}
				case SEND_TEXT_MESSAGE:
				{
					try
					{
						responseText += decrypteMessage(messageDataFromServer, clientDataManager);
						responseText += "\n";
					}
					catch (const std::exception&)
					{
						responseText += "can't decrypt message\n";
					}
					break;
				}
				case SEND_FILE:
					try
					{
						auto fileContent = decrypteMessage(messageDataFromServer, clientDataManager);
						char tempPath[MAX_PATH] = { 0 };
						GetTempPathA(MAX_PATH, tempPath);
						std::string sPath = tempPath;

						sPath += messageDataFromServer->getClientId();
						sPath += ".";
						sPath += std::to_string(messageDataFromServer->getMessageId());

						std::ofstream out(sPath, std::ofstream::out | std::ofstream::binary);
						out << fileContent;
						out.close();

						responseText += sPath + "\n";
					}
					catch (const std::exception&)
					{
						responseText += "can't decrypt message\n";
					}
					break;
				default:
					break;
				}
				responseText += "----<EOM>----:\n\n";
			}
	);
}