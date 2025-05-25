#include <gtest/gtest.h>
#include "SocketAddress.h"
#include <WinSock2.h>

// size should always be sizeof(sockaddr)
TEST(SocketAddress, GetSizeIsSockaddrSize)
{
    AetherNet::SocketAddress addr(0x01020304, 1234);
    EXPECT_EQ(addr.GetSize(), sizeof(sockaddr));
}

// wrapping an existing sockaddr_in with the second constructor
TEST(SocketAddress, WrapsSockaddrCorrectly)
{
    sockaddr_in sin{};
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(0x0A0B0C0D); // 10.11.12.13
    sin.sin_port = htons(4321);

    AetherNet::SocketAddress wrapped(reinterpret_cast<const sockaddr&>(sin));

    EXPECT_EQ(wrapped.GetIPv4Address(), 0x0A0B0C0Du);
    EXPECT_EQ(wrapped.GetPort(), 4321u);
}

// round-trip extremes: port=0, INADDR_ANY
TEST(SocketAddress, RoundTripZeroAndAny)
{
    // address = 0.0.0.0 port = 0
    AetherNet::SocketAddress addr(0x00000000, 0);
    EXPECT_EQ(addr.GetIPv4Address(), 0u);
    EXPECT_EQ(addr.GetPort(), 0u);
}

// round-trip with random values
TEST(SocketAddress, RoundTripRandom)
{
    struct Case { uint32_t ip; uint16_t port; };
    std::vector<Case> cases = {
        {0x7F000001,  80},      // 127.0.0.1:80
        {0xC0A80001, 8080},     // 192.168.0.1:8080
        {0xFFFFFFFF, 65535},    // broadcast
        {0x01020304,  12345},   // arbitrary
    };

    for (auto c : cases)
    {
        AetherNet::SocketAddress addr(c.ip, c.port);
        EXPECT_EQ(addr.GetIPv4Address(), c.ip)
            << "for IP 0x" << std::hex << c.ip;
        EXPECT_EQ(addr.GetPort(), c.port)
            << "for port " << std::dec << c.port;
    }
}
