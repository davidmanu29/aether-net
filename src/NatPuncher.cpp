#include "NatPuncher.h"
#include <iostream>

AetherNet::NatPuncher::NatPuncher(SOCKET sock)
	:mSocket(sock)
{
}

void AetherNet::NatPuncher::punch(const sockaddr_in& peer, int count, std::chrono::milliseconds interval)
{
	char const* buffer = nullptr;
	int bufferLen = 0;

	for (int i = 0; i < count; ++i)
	{
		int sent = sendto(
			mSocket,
			buffer,
			bufferLen,
			0,
			reinterpret_cast<const sockaddr*>(&peer),
			sizeof(peer));

		if (sent == SOCKET_ERROR)
		{
			std::cerr << "[NatPuncher] send to failed : " + std::to_string(WSAGetLastError());
		}
		else
		{
			std::cout << "[NatPuncher] sent packet #" + std::to_string(i + 1) + " to " + inet_ntoa(peer.sin_addr) + ":" + std::to_string(ntohs(peer.sin_port)) << std::endl;
		}
	}
}

void AetherNet::NatPuncher::punchAll(const std::vector<sockaddr_in>& peers, int count, std::chrono::milliseconds interval)
{
	for (auto const& peer : peers)
	{
		punch(peer, count, interval);
	}
}
