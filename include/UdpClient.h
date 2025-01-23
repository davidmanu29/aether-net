#pragma once

#include "AetherNetExport.h"

#include <string>
#include <thread>
#include <atomic>

class AETHERNET_API UdpClient
{
public:
    UdpClient();
    ~UdpClient();

    bool connectToServer(const std::string& serverIp, unsigned short serverPort);
    bool sendMessage(const std::string& msg);
    std::string receiveMessage();
    void disconnect();

    bool doDiffieHellmanHandshake();

    bool requestClientList();

private:
    void receiveLoop();

    std::atomic<bool> connected_;
    std::thread recvThread_;

    int clientSocket_;
    std::string serverIp_;
    unsigned short serverPort_;
};