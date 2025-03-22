#include "UdpClient.h"
#include <iostream>
#include <cstring> 

UdpClient::UdpClient(const std::string& serverIp, unsigned short port)
    : mSocket(INVALID_SOCKET), mInitialized(false)
{
    std::memset(&mServer, 0, sizeof(mServer));
    mServer.sin_family = AF_INET;
    mServer.sin_port = htons(port);
    if (inet_pton(AF_INET, serverIp.c_str(), &mServer.sin_addr) <= 0)
    {
        std::cerr << "Invalid server IP address." << std::endl;
    }
}

UdpClient::~UdpClient()
{
    if (mSocket != INVALID_SOCKET)
    {
        closesocket(mSocket);
    }
    WSACleanup();
}

bool UdpClient::init()
{
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return false;
    }
    mSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (mSocket == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return false;
    }
    mInitialized = true;
    return true;
}

bool UdpClient::sendMessage(const std::string& message)
{
    if (!mInitialized)
    {
        std::cerr << "Client not initialized." << std::endl;
        return false;
    }
    int sendOk = sendto(mSocket, message.c_str(), static_cast<int>(message.size() + 1), 0,
        reinterpret_cast<sockaddr*>(&mServer), sizeof(mServer));
    if (sendOk == SOCKET_ERROR)
    {
        std::cerr << "sendto failed: " << WSAGetLastError() << std::endl;
        return false;
    }
    return true;
}

#ifdef UDPCLIENT_MAIN
#include <iostream>

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: UdpClientExe <message>" << std::endl;
        return 1;
    }
    UdpClient client("127.0.0.1", 54000);
    if (!client.init())
    {
        std::cerr << "Failed to initialize UDP client." << std::endl;
        return 1;
    }
    std::string message(argv[1]);
    if (!client.sendMessage(message))
    {
        std::cerr << "Failed to send message." << std::endl;
        return 1;
    }
    std::cout << "Message sent successfully." << std::endl;
    return 0;
}
#endif