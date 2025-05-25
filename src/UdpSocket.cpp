#include "UdpSocket.h"
#include "SocketUtil.h"

AetherNet::UdpSocket::~UdpSocket()
{
    closesocket(mSocket);
}

int AetherNet::UdpSocket::Bind(const SocketAddress& inBindAddress)
{
    int err = bind(mSocket, &inBindAddress.mSockAddr, inBindAddress.GetSize());
    if (err != 0)
    {
        SocketUtil::ReportError(L"UDPSocket::Bind");
        return SocketUtil::GetLastError();
    }
    return 0;
}

int AetherNet::UdpSocket::SendTo(const void* inData, int inLen, const SocketAddress& inTo)
{
    int byteSentCount = sendto(mSocket,
        static_cast<const char*>(inData),
        inLen,
        0,
        &inTo.mSockAddr,
        inTo.GetSize());

    if (byteSentCount >= 0)
    {
        return byteSentCount;
    }
    else
    {
        SocketUtil::ReportError(L"UdpSocket::SendTo");
        return -SocketUtil::GetLastError();
    }
}

int AetherNet::UdpSocket::ReceiveFrom(void* inBuffer, int inLen, SocketAddress& outFrom)
{
    int fromLength = outFrom.GetSize();
    int readByteCount = recvfrom(mSocket,
        static_cast<char*>(inBuffer),
        inLen,
        0,
        &outFrom.mSockAddr,
        &fromLength);

    if (readByteCount >= 0)
    {
        return readByteCount;
    }
    else
    {
        SocketUtil::ReportError(L"UDPSocket::ReceiveFrom");
        return -SocketUtil::GetLastError();
    }   
}