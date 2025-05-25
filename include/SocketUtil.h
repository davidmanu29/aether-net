#pragma once

#include "AetherNetExport.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <string>
#include <memory>
#include "UdpSocket.h"

namespace AetherNet
{
    class AETHERNET_API  SocketUtil
    {
    public:
        static void ReportError(const std::wstring& inErrorMsg);
        static int GetLastError();

        static UdpSocketPtr CreateUDPSocket();
    };
}
