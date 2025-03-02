#pragma once

#include "SocketAddress.h"
#include <string>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <cstdint>
#include <memory>
#include <cstring>

class SocketAddressFactory
{
public:
	static SocketAddressPtr CreateIPv4FromString(const std::string& inString);
};