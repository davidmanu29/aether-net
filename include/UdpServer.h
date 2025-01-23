#pragma once

#include "AetherNetExport.h"

class AETHERNET_API UdpServer
{
public: 
	UdpServer();
	~UdpServer();

	bool start(int port);

	void stop();
};