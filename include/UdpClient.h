#pragma once

#include "AetherNetExport.h"
#include <string>

class AETHERNET_API UdpClient
{
public:
	UdpClient();
	~UdpClient();

	bool connectToServer(const std::string& ip, int port);

	bool sendMessage(const std::string& msg);

	std::string receiveMessage();

	void disconnect();
};
