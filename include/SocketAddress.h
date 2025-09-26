#pragma once

#include "AetherNetExport.h"

#include <cstdint>
#include <memory>
#include <cstring>

#include <WinSock2.h>

namespace AetherNet
{
	class AETHERNET_API SocketAddress
	{
		friend class UdpSocket;
	public:
		SocketAddress(uint32_t inAddress, uint16_t inPort);
		SocketAddress(const sockaddr& inSockAddr);

		uint32_t GetIPv4Address() const;
		uint16_t GetPort() const;

		size_t GetSize() const;

	private:
		sockaddr mSockAddr;
		sockaddr_in* GetAsSockAddrIn();
	};

	typedef std::shared_ptr<SocketAddress> SocketAddressPtr;
}