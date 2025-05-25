#pragma once

#include "AetherNetExport.h"

#include "WinsockInitializer.h"
#include "UdpSocket.h"
#include "SocketAddress.h"
#include "SocketAddressFactory.h"
#include "SocketUtil.h"

#include <string>
#include <unordered_map>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <cstring>

namespace AetherNet
{
	class AETHERNET_API UdpServer
	{
	public:
		explicit UdpServer(unsigned short port = 54000);

		~UdpServer();

		bool init();
		void run();

	private:
		WinsockInitializer mWinsock;
		AetherNet::UdpSocketPtr mSocket;
		AetherNet::SocketAddressPtr mBindAddr;
		bool mInitialized{ false };

		std::unordered_map<uint32_t, AetherNet::SocketAddressPtr> mClients;

		//no copy-construction or assignment operation should be available
		UdpServer(const UdpServer&) = delete;
		UdpServer& operator=(const UdpServer&) = delete;
	};
}

