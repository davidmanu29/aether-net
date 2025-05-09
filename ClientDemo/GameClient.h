#pragma once

#include <GLFW/glfw3.h>
#include "UdpClient.h"
#include "NatPuncher.h"
#include "UdpProtocol.h"

#include <unordered_map>
#include <mutex>
#include <thread>
#include <atomic>
#include <string>
#include <vector>

struct Actor
{
	float x, y;
};

class GameClient
{
public:
	GameClient(uint32_t id, const std::string& serverEndpoint);
	~GameClient();

	bool init();
	void run();

private:
	UdpClient mUdpClient;
	AetherNet::NatPuncher* mPuncher = nullptr;
	GLFWwindow* mWindow = nullptr;

	std::unordered_map<uint32_t, Actor> mActors;
	std::vector<AetherNet::SocketAddressPtr> mPeers;
	std::string mKey;
	std::mutex mMutex;
	std::atomic<bool> mRunning{ false };

	uint32_t mGClientId;

	void NetworkLoop();
	void ProcessIncomingReq(const MovePacket& packet);
	void HandleInput(float dt);
	void BroadcastPosition();
	void RenderFrame();
};



