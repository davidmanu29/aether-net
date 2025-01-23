#include "UdpServer.h"
#include <iostream>
#include <cstring>   // for memset, memcpy
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

static long long modExp(long long base, long long exp, long long modulus)
{
    long long result = 1;
    base = base % modulus;
    while (exp > 0)
    {
        if (exp & 1)
            result = (result * base) % modulus;
        base = (base * base) % modulus;
        exp >>= 1;
    }
    return result;
}

static const long long DH_PRIME = 23;
static const long long DH_BASE = 5;

UdpServer::UdpServer()
    : running_(false)
    , serverThread_()
    , serverSocket_(-1)
    , listenPort_(0)
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

UdpServer::~UdpServer()
{
    stop();
#ifdef _WIN32
    WSACleanup();
#endif
}

bool UdpServer::start(unsigned short port)
{
    listenPort_ = port;

    serverSocket_ = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket_ < 0)
    {
        std::cerr << "[UdpServer] Failed to create socket.\n";
        return false;
    }

    // Bind
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket_, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        std::cerr << "[UdpServer] Failed to bind socket.\n";
#ifdef _WIN32
        closesocket(serverSocket_);
#else
        close(serverSocket_);
#endif
        return false;
    }

    running_ = true;
    serverThread_ = std::thread(&UdpServer::serverLoop, this);

    std::cout << "[UdpServer] Server started on port " << port << "\n";
    return true;
}

void UdpServer::stop()
{
    running_ = false;
    if (serverThread_.joinable())
        serverThread_.join();

    if (serverSocket_ >= 0)
    {
#ifdef _WIN32
        closesocket(serverSocket_);
#else
        close(serverSocket_);
#endif
        serverSocket_ = -1;
    }
    std::cout << "[UdpServer] Server stopped.\n";
}

void UdpServer::registerClient(const ClientEndpoint& client)
{
    std::lock_guard<std::mutex> lock(clientsMutex_);
    clients_.push_back(client);
}

std::vector<ClientEndpoint> UdpServer::getRegisteredClients()
{
    std::lock_guard<std::mutex> lock(clientsMutex_);
    return clients_;
}

void UdpServer::serverLoop()
{
    char buffer[1024];
    sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    while (running_)
    {
        memset(buffer, 0, sizeof(buffer));

        int received = recvfrom(serverSocket_, buffer, sizeof(buffer), 0,
            (struct sockaddr*)&clientAddr, &addrLen);

        if (received > 0)
        {
            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
            unsigned short clientPort = ntohs(clientAddr.sin_port);

            ClientEndpoint ce{ clientIP, clientPort };
            registerClient(ce);

            std::string msg(buffer, buffer + received);
            std::cout << "[UdpServer] Received from " << clientIP << ":" << clientPort
                << " -> " << msg << "\n";

            if (msg.rfind("DH:", 0) == 0)
            {
                long long clientPubKey = std::stoll(msg.substr(3));

                static long long serverSecret = 15;
                long long serverPubKey = modExp(DH_BASE, serverSecret, DH_PRIME);

                long long sharedKey = modExp(clientPubKey, serverSecret, DH_PRIME);
                std::cout << "[UdpServer] Computed sharedKey: " << sharedKey << "\n";

                std::string response = "DH_SERVER:" + std::to_string(serverPubKey);
                sendto(serverSocket_, response.c_str(), response.size(), 0,
                    (struct sockaddr*)&clientAddr, addrLen);
            }
            else if (msg == "GET_CLIENTS")
            {
                auto clientList = getRegisteredClients();
                std::string listStr = "CLIENT_LIST\n";
                for (auto& c : clientList)
                {
                    listStr += c.ip + ":" + std::to_string(c.port) + "\n";
                }
                sendto(serverSocket_, listStr.c_str(), listStr.size(), 0,
                    (struct sockaddr*)&clientAddr, addrLen);
            }
            else
            {
                std::string reply = "[ServerEcho] " + msg;
                sendto(serverSocket_, reply.c_str(), reply.size(), 0,
                    (struct sockaddr*)&clientAddr, addrLen);
            }
        }
    }
}
