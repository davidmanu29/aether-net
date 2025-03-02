#include "SocketAddress.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <memory>

class SocketUtil;

class UdpSocket
{
public:
	~UdpSocket();
	int Bind(const SocketAddress& inToAddress);
	int SendTo(const void* inData, int inLen, const SocketAddress& inTo);
	int ReceiveFrom(void* inBuffer, int inLen, SocketAddress& outFrom);

private:
	friend class SocketUtil;
	UdpSocket(SOCKET inSocket) : mSocket(inSocket) {}
	SOCKET mSocket;
};

typedef std::shared_ptr<UdpSocket> UdpSocketPtr;