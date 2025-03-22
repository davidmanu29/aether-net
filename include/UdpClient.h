#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>

class UdpClient
{
public:
    UdpClient(const std::string& serverIp = "127.0.0.1", unsigned short port = 54000);

    ~UdpClient();
    bool init();

    bool sendMessage(const std::string& message);

private:
    SOCKET mSocket;
    sockaddr_in mServer;
    bool mInitialized;
};