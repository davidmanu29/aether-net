#include "UdpServer.h"
#include <iostream>
#include <cstring>

UdpServer::UdpServer(unsigned short port)
	: mSocket(INVALID_SOCKET), mPort(port), mInitialized(false)
{

}

UdpServer::~UdpServer()
{
    if (mSocket != INVALID_SOCKET)
    {
        closesocket(mSocket);
    }

    WSACleanup();
}

bool UdpServer::init()
{
    WSAData wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (result != 0)
    {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return false;
    }

    mSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (mSocket == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return false;
    }

    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(mPort);

    if (bind(mSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
        closesocket(mSocket);
        WSACleanup();
        return false;
    }

    mInitialized = true;
    return true;
}

void UdpServer::run()
{
    if (!mInitialized)
    {
        std::cerr << "Server not initialized." << std::endl;
        return;
    }

    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    char buffer[1024];

    std::cout << "UDP Server running on port " << mPort << ". Waiting for messages..." << std::endl;

    while (true)
    {
        std::memset(&clientAddr, 0, clientAddrSize);
        std::memset(buffer, 0, sizeof(buffer));

        int bytesReceived = recvfrom(mSocket, buffer, sizeof(buffer) - 1, 0,
            reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrSize);

        if (bytesReceived == SOCKET_ERROR)
        {
            std::cerr << "recvfrom failed: " << WSAGetLastError() << std::endl;
            continue;
        }
        buffer[bytesReceived] = '\0'; // ensure null terminated string

        char clientIp[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, INET_ADDRSTRLEN);
        int clientPort = ntohs(clientAddr.sin_port);

        std::string clientKey = std::string(clientIp) + ":" + std::to_string(clientPort);

        mClients[clientKey] = clientAddr;

        //send list to new client
        std::string listForNew;
        for (auto& [key, addr] : mClients)
        {
            if (key != clientKey)
            {
                listForNew += key + "\n";
            }
        }

        if (!listForNew.empty())
        {
            std::cout << "Sending full client list to: " << clientKey << std::endl;
            sendto(
                mSocket, 
                listForNew.c_str(),
                static_cast<int>(listForNew.size()),
                0,
                reinterpret_cast<sockaddr*>(&clientAddr),
                sizeof(clientAddr));
        }

        //notify other clients about the new one
        std::string newEntry = clientKey + "\n";
        for (auto& [key, addr] : mClients)
        {
            if (key == clientKey) continue;
            sendto(
                mSocket,
                newEntry.c_str(),
                static_cast<int>(newEntry.size()),
                0,
                reinterpret_cast<const sockaddr*>(&addr),
                sizeof(addr));
        }
    }
}

#ifdef UDPSERVER_MAIN
int main()
{
    UdpServer server(54000);

    if (!server.init())
    {
        std::cerr << "Failed to initialize UDP server." << std::endl;
        return 1;
    }

    server.run();
    return 0;
}
#endif