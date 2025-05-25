#pragma once

#include <WinSock2.h>
#include "AetherNetExport.h"

class AETHERNET_API WinsockInitializer
{
public:
	WinsockInitializer()
	{
		if (WSAStartup(MAKEWORD(2, 2), &mWsadata) != 0)
		{
			//TODO: add log
		}
	}

	~WinsockInitializer()
	{
		WSACleanup();
	}

	WinsockInitializer(const WinsockInitializer&) = delete;
	WinsockInitializer& operator=(const WinsockInitializer&) = delete;

private:

	WSADATA mWsadata;
};