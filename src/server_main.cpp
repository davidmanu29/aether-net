#include "UdpServer.h"
#include <iostream>

int main()
{
    AetherNet::UdpServer server(54000);
    if (!server.init())
    {
        std::cerr << "Failed to initialize UDP server." << std::endl;
        return 1;
    }
    server.run();
    return 0;
}
