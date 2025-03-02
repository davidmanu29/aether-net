#include <cstdint>
#include <memory>
#include <cstring>

#include <WinSock2.h>

class SocketAddress
{
	friend class UdpSocket;
public:
	SocketAddress(uint32_t inAddress, uint16_t inPort);
	SocketAddress(const sockaddr& inSockAddr);

	size_t GetSize() const;

private:
	sockaddr mSockAddr;
	sockaddr_in* GetAsSockAddrIn();
};

typedef std::shared_ptr<SocketAddress> SocketAddressPtr;