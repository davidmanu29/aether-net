// tests/UdpSocket_extra_test.cpp
#include <gtest/gtest.h>
#include "UdpSocket.h"
#include "SocketAddress.h"
#include "SocketUtil.h"
#include "WinsockInitializer.h"

using namespace AetherNet;

static WinsockInitializer _winsock_init;

static SocketAddress loopback(uint16_t port)
{
    // 127.0.0.1 is 0x7F000001 in host byte order
    return SocketAddress(0x7F000001, port);
}

TEST(UdpSocket, BindEphemeral)
{
    auto sock = AetherNet::SocketUtil::CreateUDPSocket();
    ASSERT_NE(sock, nullptr);
    // bind to 0.0.0.0:0, any interface, ephemeral port
    SocketAddress any(0x00000000, 0);
    EXPECT_EQ(sock->Bind(any), 0);
}

TEST(UdpSocket, DoubleBindSamePortFails)
{
    constexpr uint16_t PORT = 55000;
    auto s1 = AetherNet::SocketUtil::CreateUDPSocket();
    auto s2 = AetherNet::SocketUtil::CreateUDPSocket();
    ASSERT_NE(s1, nullptr);
    ASSERT_NE(s2, nullptr);

    SocketAddress addr = loopback(PORT);
    
    EXPECT_EQ(s1->Bind(addr), 0);
    
    int err = s2->Bind(addr);
    EXPECT_NE(err, 0);
    
    EXPECT_EQ(err, WSAEADDRINUSE);
}

TEST(UdpSocket, SendReceiveLoopback)
{
    constexpr uint16_t P1 = 55010;
    constexpr uint16_t P2 = 55011;
    auto s1 = AetherNet::SocketUtil::CreateUDPSocket();
    auto s2 = AetherNet::SocketUtil::CreateUDPSocket();
    ASSERT_NE(s1, nullptr);
    ASSERT_NE(s2, nullptr);

    // bind both to distinct loopback ports
    ASSERT_EQ(s1->Bind(loopback(P1)), 0);
    ASSERT_EQ(s2->Bind(loopback(P2)), 0);

    const char* msg = "hello";
    int sent = s1->SendTo(msg, (int)strlen(msg), loopback(P2));
    ASSERT_EQ(sent, (int)strlen(msg));

    // prepare receive buffer
    char buf[32];
    SocketAddress from(0x00000000, 0);
    int recvd = s2->ReceiveFrom(buf, sizeof(buf), from);
    ASSERT_EQ(recvd, sent);
    EXPECT_EQ(std::string(buf, recvd), "hello");
   
    EXPECT_EQ(from.GetIPv4Address(), 0x7F000001u);
    EXPECT_EQ(from.GetPort(), P1);
}

TEST(UdpSocket, ZeroLengthDatagram)
{
    constexpr uint16_t P1 = 55020;
    constexpr uint16_t P2 = 55021;
    auto s1 = AetherNet::SocketUtil::CreateUDPSocket();
    auto s2 = AetherNet::SocketUtil::CreateUDPSocket();
    ASSERT_EQ(s1->Bind(loopback(P1)), 0);
    ASSERT_EQ(s2->Bind(loopback(P2)), 0);

    int sent = s1->SendTo(nullptr, 0, loopback(P2));
    EXPECT_GE(sent, 0);

    char buf[1];
    SocketAddress from(0x00000000, 0);
    int recvd = s2->ReceiveFrom(buf, sizeof(buf), from);
    EXPECT_GE(recvd, 0);
    EXPECT_LE(recvd, 1);
    EXPECT_EQ(from.GetPort(), P1);
}
