#include "SocketAddressFactory.h"

AetherNet::SocketAddressPtr AetherNet::SocketAddressFactory::CreateIPv4FromString(const std::string& inString)
{
	size_t pos = inString.find_last_of(':');
	std::string host, service;

	if (pos != std::string::npos)
	{
		host = inString.substr(0, pos);
		service = inString.substr(pos + 1);
	}
	else
	{
		host = inString;
		service = "0"; // use default port
	}

	addrinfo hint;
	memset(&hint, 0, sizeof(hint));
	hint.ai_family = AF_INET;

	addrinfo* result = nullptr;
	int error = getaddrinfo(host.c_str(), service.c_str(), &hint, &result);

	if (error != 0)
	{
		if (result != nullptr)
		{
			freeaddrinfo(result);
		}

		return nullptr;
	}

	addrinfo* originalResult = result;

	while (result && !result->ai_addr)
	{
		result = result->ai_next;
	}

	if (result == nullptr || result->ai_addr == nullptr)
	{
		freeaddrinfo(originalResult);
		return nullptr;
	}

	SocketAddressPtr toReturn = std::make_shared<SocketAddress>(*result->ai_addr);
	freeaddrinfo(originalResult);

	return toReturn;
}