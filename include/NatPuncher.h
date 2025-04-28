#pragma once

#include <WinSock2.h>
#include "AetherNetExport.h"
#include <vector>
#include <chrono>

namespace AetherNet
{
	class AETHERNET_API NatPuncher
	{
	public:
		explicit NatPuncher(SOCKET sock);

		void punch(const sockaddr_in& peer, int count = 5, std::chrono::milliseconds interval = std::chrono::milliseconds(200));

		void punchAll(const std::vector<sockaddr_in>& peers, int count = 5, std::chrono::milliseconds interval = std::chrono::milliseconds(200));

	private:
		SOCKET mSocket;
	};
}
