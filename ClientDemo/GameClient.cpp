#include "GameClient.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <WinSock2.h>

GameClient::GameClient(uint32_t id, const std::string& serverEndpoint)
	: mUdpClient(serverEndpoint)
	, mGClientId(id)
{ }

GameClient::~GameClient()
{
	mRunning = false;

	if (mWindow)
	{
		glfwDestroyWindow(mWindow);
		glfwTerminate();
	}
}

void GameClient::ProcessIncomingReq(const MovePacket& pkt)
{
	std::lock_guard lk(mMutex);

	mActors[pkt.actorId] = {
		aethernet_ntohf(pkt.x),
		aethernet_ntohf(pkt.y)
	};
}

bool GameClient::init()
{
	if (!mUdpClient.init()) return false;
	mPuncher = new AetherNet::NatPuncher(mUdpClient.GetSocket());

	if (!glfwInit())
	{
		std::cerr << "glfwInit failed\n";
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	mWindow = glfwCreateWindow(800, 600, "Client Demo", nullptr, nullptr);

	if (!mWindow)
	{
		std::cerr << "glfwCreateWindow failed\n";
		return false;
	}

	glfwMakeContextCurrent(mWindow);
	glfwSwapInterval(1);

	mRunning = true;
	std::thread(&GameClient::NetworkLoop, this).detach();

	{
		std::lock_guard lk(mMutex);
		mActors[mGClientId] = { 400.f, 300.f };
	}

	return true;
}

void GameClient::NetworkLoop()
{
	// 1) tell the rendezvous server who you are
	if (!mUdpClient.sendMessage("REGISTER\n"))
	{
		std::cerr << "Failed to register with server\n";
		return;
	}

	// 2) receive the server’s peer-list (text "ip:port\n...")
	char textBuf[1024];
	AetherNet::SocketAddress fromAddr(0, 0);

	int n = mUdpClient.GetSocket()->ReceiveFrom(textBuf, sizeof(textBuf), fromAddr);
	if (n <= 0)
	{
		std::cerr << "Did not receive peer list\n";
		return;
	}
	textBuf[n] = '\0';

	{
		std::lock_guard lk(mMutex);
		mPeers.clear();
		std::stringstream ss(textBuf);
		std::string line;
		while (std::getline(ss, line))
		{
			if (line.empty()) continue;

			if (auto peer = AetherNet::SocketAddressFactory::CreateIPv4FromString(line))
				mPeers.push_back(peer);
		}
	}

	// 3) hole-punch all peers once
	mPuncher->punchAll(mPeers);

	// 4) now consume _both_ binary MOVE packets and any future text updates
	char recvBuf[1024];
	while (mRunning)
	{
		int bytes = mUdpClient.GetSocket()
			->ReceiveFrom(recvBuf, sizeof(recvBuf), fromAddr);

		if (bytes < 0)
		{
			int err = -bytes;
			// silently ignore “message too long” (EMSGSIZE = 10040)
			if (err != WSAEMSGSIZE)
				std::cerr << "ReceiveFrom error: " << err << "\n";
			continue;
		}

		if (bytes == (int)MOVE_PACKET_SIZE)
		{
			// exactly one MOVE packet
			auto const* p = reinterpret_cast<const MovePacket*>(recvBuf);
			if (p->type == static_cast<uint8_t>(Action::MOVE))
			{
				MovePacket copy;
				copy.type = p->type;
				copy.actorId = ntohl(p->actorId);
				copy.x = aethernet_ntohf(p->x);
				copy.y = aethernet_ntohf(p->y);
				ProcessIncomingReq(copy);
			}
		}
		else if (bytes > 0)
		{
			// some other text message (e.g. a new peer joined/left)
			recvBuf[bytes] = '\0';
			std::lock_guard lk(mMutex);
			std::stringstream ss(recvBuf);
			std::string line;
			mPeers.clear();
			while (std::getline(ss, line))
			{
				if (line.empty()) continue;
				if (auto peer = AetherNet::SocketAddressFactory::CreateIPv4FromString(line))
					mPeers.push_back(peer);
			}
			// re–punch in case peers changed
			mPuncher->punchAll(mPeers);
		}
		// else bytes==0: empty datagram? ignore
	}
}

void GameClient::HandleInput(float dt)
{
	const float speed = 200.f;
	int16_t dx = 0;
	int16_t dy = 0;

	if (glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS) dy -= (int16_t)(speed * dt);
	if (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS) dy += (int16_t)(speed * dt);
	if (glfwGetKey(mWindow, GLFW_KEY_A) == GLFW_PRESS) dx -= (int16_t)(speed * dt);
	if (glfwGetKey(mWindow, GLFW_KEY_D) == GLFW_PRESS) dx += (int16_t)(speed * dt);

	if (dx || dy)
	{
		std::lock_guard lock(mMutex);
		auto& me = mActors[mGClientId];
		me.x += dx;
		me.y += dy;
		BroadcastPosition();
	}
}

void GameClient::BroadcastPosition()
{
	MovePacket packet;
	packet.type = static_cast<uint8_t>(Action::MOVE);
	packet.actorId = htonl(mGClientId);
	packet.x = aethernet_htonf(mActors[mGClientId].x);
	packet.y = aethernet_htonf(mActors[mGClientId].y);

	for (auto& peer : mPeers)
	{
		mUdpClient.GetSocket()->SendTo(&packet, MOVE_PACKET_SIZE, *peer);
	}
}

void GameClient::RenderFrame()
{
	int width, height;
	glfwGetFramebufferSize(mWindow, &width, &height);

	glViewport(0, 0, width, height);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);


	std::lock_guard lk(mMutex);

	for (auto& x : mActors)
	{
		const auto& a = x.second;

		float xn = (a.x / 400.f) - 1.f;
		float yn = (a.y / 300.f) - 1.f;
		float s = 10.f / 400.f;

		if (x.first == mGClientId)
		{
			glColor3f(0, 1, 0);
		}
		else
		{
			glColor3f(1, 1, 1);
		}

		glBegin(GL_QUADS);
		glVertex2f(xn - s, yn - s);
		glVertex2f(xn + s, yn - s);
		glVertex2f(xn + s, yn + s);
		glVertex2f(xn - s, yn + s);
		glEnd();
	}

	glfwSwapBuffers(mWindow);
}

void GameClient::run()
{
	using clk = std::chrono::steady_clock;
	auto last = clk::now();

	while (mRunning && !glfwWindowShouldClose(mWindow))
	{
		glfwPollEvents();
		auto now = clk::now();
		float dt = std::chrono::duration<float>(now - last).count();

		last = now;

		HandleInput(dt);
		RenderFrame();

		{
			std::lock_guard lk(mMutex);
			BroadcastPosition();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}

	mRunning = false;
}