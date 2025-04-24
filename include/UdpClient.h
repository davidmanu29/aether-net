#pragma once

#include <string>
#include <unordered_map>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <sstream>
#include <chrono>

class UdpClient
{
public:
    UdpClient(const std::string& serverIp = "127.0.0.1", unsigned short port = 54000);

    ~UdpClient();
    bool init();

    bool sendMessage(const std::string& message);

    SOCKET GetSocket();

private:
    SOCKET mSocket;
    sockaddr_in mServer;
    bool mInitialized;
    std::unordered_map<std::string, sockaddr_in> mClients;
};