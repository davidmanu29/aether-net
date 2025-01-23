#include "UdpClient.h"
#include <iostream>
#include <cstring> 
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <mutex>

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

UdpClient::UdpClient()
    : connected_(false)
    , clientSocket_(-1)
    , serverPort_(0)
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

UdpClient::~UdpClient()
{
    disconnect();
#ifdef _WIN32
    WSACleanup();
#endif
}

bool UdpClient::connectToServer(const std::string& ip, unsigned short port)
{
    serverIp_ = ip;
    serverPort_ = port;

    clientSocket_ = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket_ < 0)
    {
        std::cerr << "[UdpClient] Error creating socket\n";
        return false;
    }

    sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &servAddr.sin_addr);

    if (::connect(clientSocket_, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
    {
        std::cerr << "[UdpClient] Error connecting to server\n";
#ifdef _WIN32
        closesocket(clientSocket_);
#else
        close(clientSocket_);
#endif
        return false;
    }

    connected_ = true;
    recvThread_ = std::thread(&UdpClient::receiveLoop, this);

    std::cout << "[UdpClient] Connected to " << ip << ":" << port << "\n";
    return true;
}

bool UdpClient::sendMessage(const std::string& msg)
{
    if (!connected_)
        return false;

    int sent = send(clientSocket_, msg.c_str(), (int)msg.size(), 0);
    return (sent == (int)msg.size());
}

std::string UdpClient::receiveMessage()
{
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    int received = recv(clientSocket_, buffer, sizeof(buffer), 0);
    if (received > 0)
    {
        return std::string(buffer, buffer + received);
    }
    return "";
}

void UdpClient::disconnect()
{
    connected_ = false;
    if (recvThread_.joinable())
        recvThread_.join();

    if (clientSocket_ >= 0)
    {
#ifdef _WIN32
        closesocket(clientSocket_);
#else
        close(clientSocket_);
#endif
        clientSocket_ = -1;
    }
    std::cout << "[UdpClient] Disconnected.\n";
}

void UdpClient::receiveLoop()
{
    char buffer[1024];
    while (connected_)
    {
        memset(buffer, 0, sizeof(buffer));
        int received = recv(clientSocket_, buffer, sizeof(buffer), 0);
        if (received > 0)
        {
            std::string msg(buffer, buffer + received);
            std::cout << "[UdpClient] AsyncReceived: " << msg << "\n";
        }
    }
}

bool UdpClient::doDiffieHellmanHandshake()
{
    if (!connected_) return false;

    static long long clientSecret = 6;
    long long clientPubKey = modExp(DH_BASE, clientSecret, DH_PRIME);

    std::string msg = "DH:" + std::to_string(clientPubKey);
    if (!sendMessage(msg))
        return false;

    std::string response = receiveMessage();
    if (response.rfind("DH_SERVER:", 0) == 0)
    {
        long long serverPubKey = std::stoll(response.substr(10));
        long long sharedKey = modExp(serverPubKey, clientSecret, DH_PRIME);
        std::cout << "[UdpClient] Computed sharedKey: " << sharedKey << "\n";
        return true;
    }
    return false;
}

bool UdpClient::requestClientList()
{
    if (!connected_)
        return false;

    if (!sendMessage("GET_CLIENTS"))
        return false;

    std::string response = receiveMessage();
    if (response.rfind("CLIENT_LIST", 0) == 0)
    {
        std::cout << "[UdpClient] Received client list:\n" << response << "\n";

        auto pos = response.find('\n');

        if (pos != std::string::npos)
        {
            auto lines = response.substr(pos + 1);

            size_t start = 0;
            while (true)
            {
                size_t lineEnd = lines.find('\n', start);
                if (lineEnd == std::string::npos) break;
                std::string line = lines.substr(start, lineEnd - start);
                start = lineEnd + 1;

                auto sep = line.find(':');
                if (sep == std::string::npos) continue;
                std::string ip = line.substr(0, sep);
                std::string port = line.substr(sep + 1);

                sockaddr_in punchAddr;
                memset(&punchAddr, 0, sizeof(punchAddr));
                punchAddr.sin_family = AF_INET;
                punchAddr.sin_port = htons((unsigned short)std::stoi(port));
                inet_pton(AF_INET, ip.c_str(), &punchAddr.sin_addr);

                std::string helloMsg = "HOLE_PUNCH_HELLO from " + serverIp_ + ":" + std::to_string(serverPort_);
                sendto(clientSocket_, helloMsg.c_str(), helloMsg.size(), 0,
                    (struct sockaddr*)&punchAddr, sizeof(punchAddr));
            }
        }
        return true;
    }
    return false;
}
