#include "SocketUtil.h"
#include <iostream>

void SocketUtil::ReportError(const std::wstring& inErrorMsg)
{
    int error = WSAGetLastError();
    std::wcerr << inErrorMsg << L" Error: " << error << std::endl;
}

int SocketUtil::GetLastError()
{
    return WSAGetLastError();
}

UdpSocketPtr SocketUtil::CreateUDPSocket()
{
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET)
    {
        ReportError(L"SocketUtil::CreateUDPSocket");
        return nullptr;
    }
}
