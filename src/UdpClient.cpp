#include "UdpClient.h"
#include "NatPuncher.h"
#include <iostream>
#include <cstring> 
#include <thread>

UdpClient::UdpClient(const std::string& serverEndpoint)
    : mServerAddr(AetherNet::SocketAddressFactory::CreateIPv4FromString(serverEndpoint)), mInitialized(false)
{
    if (!mServerAddr)
    {
        std::cerr << "Invalid server endpoint: " << serverEndpoint << "\n";
    }
}

UdpClient::~UdpClient()
{
    if (mSocket) mSocket.reset();
}

AetherNet::UdpSocketPtr UdpClient::GetSocket() const
{
    return mSocket;
}

bool UdpClient::init()
{
    mSocket = AetherNet::SocketUtil::CreateUDPSocket();
    if (!mSocket)
    {
        std::cerr << "Failed to create UDP socket\n";
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

    int sent = mSocket->SendTo(message.data(), (int)message.size() + 1, *mServerAddr);

    if (sent < 0) return false;

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

    UdpClient client(argv[1], 54000);

    if (!client.init())
    {
        std::cerr << "Failed to initialize UDP client." << std::endl;
        return 1;
    }

    std::cout << "Connected to the server at: " << argv[1] << std::endl;
    std::cout << "You can freely send messages. Type 'exit' to close the program." << std::endl;

    std::string message;

    std::thread listener([&client]()
    {
        char buffer[1024];
        sockaddr_in fromAddr;
        int fromLen = sizeof(fromAddr);

        while (true)
        {
            int bytesReceived = recvfrom(
                client.GetSocket(),
                buffer,
                sizeof(buffer) - 1,
                0,
                reinterpret_cast<sockaddr*>(&fromAddr),
                &fromLen);

            if (bytesReceived > 0)
            {
                buffer[bytesReceived] = '\0';
                std::string peerList(buffer);
                std::cout << "Raw peer list:\n" << peerList << std::endl;

                // parse each "ip:port" line
                std::vector<sockaddr_in> peers;
                std::stringstream ss(peerList);
                std::string line;
                while (std::getline(ss, line))
                {
                    if (line.empty())
                        continue;

                    auto pos = line.find(':');
                    if (pos == std::string::npos)
                        continue;

                    std::string peerIp = line.substr(0, pos);
                    unsigned short peerPort = static_cast<unsigned short>(
                        std::stoi(line.substr(pos + 1)));

                    sockaddr_in peerAddr;
                    std::memset(&peerAddr, 0, sizeof(peerAddr));
                    peerAddr.sin_family = AF_INET;
                    peerAddr.sin_port = htons(peerPort);
                    inet_pton(AF_INET, peerIp.c_str(), &peerAddr.sin_addr);

                    peers.push_back(peerAddr);
                }

                AetherNet::NatPuncher puncher(client.GetSocket());
                puncher.punchAll(peers);
            }
        }
    });

    while (true)
    {
        std::cout << "> ";
        std::getline(std::cin, message);

        if (message == "exit") break;

        if (!client.sendMessage(message))
        {
            std::cerr << "Failed to send message." << std::endl;
            return 1;
        }
    }

    return 0;
}
#endif