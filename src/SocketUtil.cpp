#include "SocketUtil.h"
#include <iostream>

void AetherNet::SocketUtil::ReportError(const std::wstring& inErrorMsg)
{
    int error = WSAGetLastError();
    std::wcerr << inErrorMsg << L" Error: " << error << std::endl;
}

int AetherNet::SocketUtil::GetLastError()
{
    return WSAGetLastError();
}

AetherNet::UdpSocketPtr AetherNet::SocketUtil::CreateUDPSocket()
{
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET)
    {
        ReportError(L"SocketUtil::CreateUDPSocket");
        return nullptr;
    }

    return std::make_shared<UdpSocket>(sock);
}
