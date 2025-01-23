#pragma once

#include "AetherNetExport.h"

#include <string>
#include <thread>
#include <atomic>
#include <map>
#include <mutex>
#include <vector>

struct ClientEndpoint
{
    std::string ip;
    unsigned short port;
};

class AETHERNET_API UdpServer
{
public:
    UdpServer();
    ~UdpServer();

    bool start(unsigned short port);
    void stop();

    void registerClient(const ClientEndpoint& client);
    std::vector<ClientEndpoint> getRegisteredClients();

private:
    void serverLoop();

    std::atomic<bool> running_;
    std::thread serverThread_;

    std::mutex clientsMutex_;
    std::vector<ClientEndpoint> clients_;

    int serverSocket_;
    unsigned short listenPort_;
};