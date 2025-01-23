#include "UdpClient.h"
#include <iostream>

UdpClient::UdpClient()
{

}

UdpClient::~UdpClient()
{
   
}

bool UdpClient::connectToServer(const std::string& ip, int port)
{
    std::cout << "Connecting to " << ip << ":" << port << std::endl;
    return true;
}

bool UdpClient::sendMessage(const std::string& msg)
{
    std::cout << "Sending: " << msg << std::endl;
    return true;
}

std::string UdpClient::receiveMessage()
{
    std::string dummy = "Fake data from server";
    std::cout << "Received: " << dummy << std::endl;
    return dummy;
}

void UdpClient::disconnect()
{
    std::cout << "Disconnected.\n";
}
