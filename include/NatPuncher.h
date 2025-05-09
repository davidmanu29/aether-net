#pragma once

#include "AetherNetExport.h"
#include "UdpSocket.h"
#include "SocketAddress.h"
#include "SocketUtil.h"

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>
#include <thread>
#include <chrono>

namespace AetherNet
{
	class AETHERNET_API NatPuncher
	{
	public:
		explicit NatPuncher(UdpSocketPtr sock);

		void punch(const SocketAddress& peer, int count = 5, std::chrono::milliseconds interval = std::chrono::milliseconds(200));

		void punchAll(const std::vector<SocketAddressPtr>& peers, int count = 5, std::chrono::milliseconds interval = std::chrono::milliseconds(200));

	private:
		UdpSocketPtr mSocket;
	};
}
