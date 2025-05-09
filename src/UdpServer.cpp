#include "UdpServer.h"

AetherNet::UdpServer::UdpServer(unsigned short port)
	: mBindAddr(AetherNet::SocketAddressFactory::CreateIPv4FromString("0.0.0.0:" + std::to_string(port)))
{
    if (!mBindAddr)
    {
        std::cerr << "UdpServer: invalid bind address\n";
    }
}

AetherNet::UdpServer::~UdpServer()
{
    mSocket.reset();
}

bool AetherNet::UdpServer::init()
{
    mSocket = AetherNet::SocketUtil::CreateUDPSocket();
    if (!mSocket)
    {
        std::cerr << "UdpServer::init: failed to create socket\n";
        return false;
    }

    if (int err = mSocket->Bind(*mBindAddr); err != 0)
    {
        std::cerr << "UdpServer::init: bind failed (" << err << ")\n";
        return false;
    }

    mInitialized = true;
    return true;
}

void AetherNet::UdpServer::run()
{
    if (!mInitialized)
    {
        std::cerr << "UdpServer::run called before init()\n";
        return;
    }

    // Log the bind address using the public getters
    {
        in_addr in;
        in.s_addr = htonl(mBindAddr->GetIPv4Address());

        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &in, ipStr, sizeof(ipStr));

        std::cout << "UDP Server listening on "
            << ipStr << ":" << mBindAddr->GetPort()
            << std::endl;
    }

    char buffer[1024];
    AetherNet::SocketAddress fromAddr(0, 0);

    while (true)
    {
        int bytes = mSocket->ReceiveFrom(buffer, sizeof(buffer) - 1, fromAddr);
        if (bytes < 0)
        {
            continue;
        }

        buffer[bytes] = '\0';

        {
            in_addr in;
            in.s_addr = htonl(fromAddr.GetIPv4Address());

            char ipStr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &in, ipStr, sizeof(ipStr));

            int port = fromAddr.GetPort();
            std::string key = std::string(ipStr) + ":" + std::to_string(port);

            {
                uint32_t ip_h = fromAddr.GetIPv4Address();
                uint16_t port = fromAddr.GetPort();
                mClients[key] = std::make_shared<AetherNet::SocketAddress>(ip_h, port);
            }

            std::string listForNew;
            for (auto& [k, addr] : mClients)
                if (k != key)
                    listForNew += k + "\n";

            if (!listForNew.empty())
            {
                mSocket->SendTo(listForNew.data(),
                    (int)listForNew.size(),
                    fromAddr);
            }

            std::string notif = key + "\n";
            for (auto& [k, peerAddrPtr] : mClients)
            {
                mSocket->SendTo(
                    notif.data(),
                    (int)notif.size(),
                    *peerAddrPtr
                    );
            }
        }
    }
}

#ifdef UDPSERVER_MAIN
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
#endif