#include "UdpServer.h"
#include <iostream>

UdpServer::UdpServer()
{

}

UdpServer::~UdpServer()
{

}

bool UdpServer::start(int port)
{
    std::cout << "Starting on port " << port << std::endl;
    return true;
}

void UdpServer::stop()
{
    std::cout << "Server stopped.\n";
}