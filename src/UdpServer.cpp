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
        std::string msg(buffer);
        if (msg.rfind("REGISTER ", 0) == 0)
        {
            // parse client ID
            try
            {
                uint32_t clientId = std::stoul(msg.substr(9));
                // store this client endpoint
                mClients[clientId] =
                    std::make_shared<SocketAddress>(
                        fromAddr.GetIPv4Address(),
                        fromAddr.GetPort()
                    );
                std::cout << "[UdpServer] Registered client " << clientId
                    << " at " << fromAddr.GetIPv4Address()
                    << ":" << fromAddr.GetPort() << "\n";
            }
            catch (...)
            {
                continue;
            }
        }

        // broadcast each peer-list:
        if (mClients.size() >= 2)
        {
            for (auto& [recipientId, recipientAddr] : mClients)
            {
                std::string fullList;
                for (auto& [peerId, peerAddr] : mClients)
                {
                    if (peerId == recipientId)
                        continue;

                    in_addr pin{};
                    pin.s_addr = htonl(peerAddr->GetIPv4Address());
                    char peerIpStr[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &pin, peerIpStr, sizeof(peerIpStr));

                    fullList += std::to_string(peerId)
                        + " "
                        + std::string(peerIpStr)
                        + ":"
                        + std::to_string(peerAddr->GetPort())
                        + "\n";
                }

                if (!fullList.empty())
                {
                    mSocket->SendTo(
                        fullList.data(),
                        static_cast<int>(fullList.size()),
                        *recipientAddr
                    );
                }
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