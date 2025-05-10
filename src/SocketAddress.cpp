#include "SocketAddress.h"

AetherNet::SocketAddress::SocketAddress(uint32_t inAddress, uint16_t inPort)
{
	sockaddr_in* addrIn = GetAsSockAddrIn();
	addrIn->sin_family = AF_INET;

	addrIn->sin_addr.s_addr = htonl(inAddress);
	addrIn->sin_port = htons(inPort);
}

AetherNet::SocketAddress::SocketAddress(const sockaddr& inSockAddr)
{
	memcpy(&mSockAddr, &inSockAddr, sizeof(sockaddr));
}

size_t AetherNet::SocketAddress::GetSize() const
{
	return sizeof(sockaddr);
}

sockaddr_in* AetherNet::SocketAddress::GetAsSockAddrIn()
{
	return reinterpret_cast<sockaddr_in*>(&mSockAddr);
}

uint32_t AetherNet::SocketAddress::GetIPv4Address() const
{
	auto sin = reinterpret_cast<const sockaddr_in*>(&mSockAddr);

	return ntohl(sin->sin_addr.s_addr);
}
uint16_t AetherNet::SocketAddress::GetPort() const
{
	auto sin = reinterpret_cast<const sockaddr_in*>(&mSockAddr);

	return ntohs(sin->sin_port);
}