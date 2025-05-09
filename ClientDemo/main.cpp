#include "GameClient.h"
#include <iostream>

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Usage: UdpClientExe <yourId> <server-ip> [port]\n";
        return 1;
    }

    uint32_t myId = static_cast<uint32_t>(std::stoi(argv[1]));
    std::string serverIp = argv[2];
    unsigned short port = (argc >= 4) ? (unsigned short)std::stoi(argv[3]) : 54000;

    GameClient app(myId, serverIp, port);

    if (!app.init()) return 1;
    app.run();

    return 0;
}