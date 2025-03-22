#pragma once

#include "AetherNetExport.h"

#include <WinSock2.h>
#include <WS2tcpip.h>

class UdpServer
{
public:
	explicit UdpServer(unsigned short port = 54000);

	~UdpServer();

	bool init();
	void run();

private:

	SOCKET mSocket;
	unsigned short mPort;
	bool mInitialized;

	//no copy-construction or assignment operation should be available
	UdpServer(const UdpServer&) = delete;
	UdpServer& operator=(const UdpServer&) = delete;
};