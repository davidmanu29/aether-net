#pragma once

#include <cstdint>
#include <WinSock2.h>
#include <cstring>

enum class Action : uint8_t
{
	MOVE = 1,
};

#pragma pack(push,1)
struct MovePacket
{
	uint8_t type;
	uint32_t actorId;
	float x;
	float y;
};
#pragma pack(pop)

static constexpr size_t MOVE_PACKET_SIZE = sizeof(MovePacket);

static inline uint32_t aethernet_htonf(float f)
{
	uint32_t i;
	std::memcpy(&i, &f, sizeof(i));

	return htonl(i);
}

static inline float aethernet_ntohf(uint32_t i)
{
	i = ntohl(i);
	float f;
	std::memcpy(&f, &i, sizeof(f));

	return f;
}