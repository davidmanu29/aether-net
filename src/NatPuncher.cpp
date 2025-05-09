#include "NatPuncher.h"
#include <iostream>

AetherNet::NatPuncher::NatPuncher(UdpSocketPtr sock)
	:mSocket(std::move(sock))
{
}

void AetherNet::NatPuncher::punch(const SocketAddress& peer, int count, std::chrono::milliseconds interval)
{
    for (int i = 0; i < count; ++i)
    {
        // send zero-length packet
        int sent = mSocket->SendTo(nullptr, 0, peer);

        if (sent < 0)
        {
            std::cerr << "[NatPuncher] send failed: " << SocketUtil::GetLastError() << std::endl;
        }
        else
        {
            // log "peer IP:port"
            in_addr in;
            in.s_addr = htonl(peer.GetIPv4Address());
            char ipStr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &in, ipStr, sizeof(ipStr));

            std::cout << "[NatPuncher] sent packet #" << (i + 1)
                << " to " << ipStr << ":" << peer.GetPort()
                << std::endl;
        }

        std::this_thread::sleep_for(interval);
    }
}

void AetherNet::NatPuncher::punchAll(const std::vector<SocketAddressPtr>& peers, int count, std::chrono::milliseconds interval)
{
    for (auto const& peerPtr : peers)
    {
        punch(*peerPtr, count, interval);
    }
}
