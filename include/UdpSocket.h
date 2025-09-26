#pragma once

#include "AetherNetExport.h"
#include "SocketAddress.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <memory>

class SocketUtil;

namespace AetherNet
{
	class AETHERNET_API  UdpSocket
	{
	public:
		UdpSocket(SOCKET inSocket) : mSocket(inSocket) {}
		~UdpSocket();
		int Bind(const SocketAddress& inToAddress);
		int SendTo(const void* inData, int inLen, const SocketAddress& inTo);
		int ReceiveFrom(void* inBuffer, int inLen, SocketAddress& outFrom);

	private:
		friend class SocketUtil;
		SOCKET mSocket;
	};

	typedef std::shared_ptr<UdpSocket> UdpSocketPtr;
}