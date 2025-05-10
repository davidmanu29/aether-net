#pragma once

#include "AetherNetExport.h"

#include "WinsockInitializer.h"
#include <string>
#include <unordered_map>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <sstream>
#include <chrono>
#include "SocketAddressFactory.h"
#include "SocketUtil.h"

class AETHERNET_API UdpClient
{
public:
    UdpClient(const std::string& serverEndpoint);
    ~UdpClient();

    bool init();
    bool sendMessage(const std::string& message);

    AetherNet::UdpSocketPtr GetSocket() const;

private:
    WinsockInitializer mWinsock;
    AetherNet::SocketAddressPtr mServerAddr;
    AetherNet::UdpSocketPtr mSocket;
    bool mInitialized;
    std::unordered_map<std::string, sockaddr_in> mClients;
};