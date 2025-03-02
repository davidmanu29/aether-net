#include "SocketAddress.h"

SocketAddress::SocketAddress(uint32_t inAddress, uint16_t inPort)
{
	sockaddr_in* addrIn = GetAsSockAddrIn();
	addrIn->sin_family = AF_INET;

	addrIn->sin_addr.s_addr = htonl(inAddress);
	addrIn->sin_port = htons(inPort);
}

SocketAddress::SocketAddress(const sockaddr& inSockAddr)
{
	memcpy(&mSockAddr, &inSockAddr, sizeof(sockaddr));
}

size_t SocketAddress::GetSize() const
{
	return sizeof(sockaddr);
}

sockaddr_in* SocketAddress::GetAsSockAddrIn()
{
	return reinterpret_cast<sockaddr_in*>(&mSockAddr);
}