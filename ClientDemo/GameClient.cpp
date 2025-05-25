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

	mPrivateKey = SecurityService::GenPrivate();
	mPublicKey = SecurityService::ComputePublic(mPrivateKey);

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
	if (!mUdpClient.sendMessage("REGISTER " + std::to_string(mGClientId) + "\n"))
	{
		std::cerr << "Failed to register with server\n";
		return;
	}

	// 2) receive the server's peer-list (text "ip:port\n...")
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

			std::istringstream ls(line);
			uint32_t peerId;
			std::string endpoint;
			ls >> peerId >> endpoint;

			auto peer = AetherNet::SocketAddressFactory::CreateIPv4FromString(endpoint);
			std::cout << "[NetworkLoop] discovered peer: " << line << "\n";
			if (!peer) continue;

			mPeers.emplace_back(peerId, peer);
		}
	}

	// 3) hole-punch all peers once
	mPuncher->punchAll(
		[&]
		{
			std::vector<AetherNet::SocketAddressPtr> addrs;
			addrs.reserve(mPeers.size());
			for (auto& pr : mPeers) addrs.push_back(pr.second);

			return addrs;
		}());

	// 4) broadcast key to other peers
	for (auto const& [peerId, peer] : mPeers)
	{
		std::string keyMsg =
			"KEY " + std::to_string(mGClientId)
			+ " " + std::to_string(mPublicKey)
			+ "\n";

		mUdpClient.GetSocket()->SendTo(
			keyMsg.data(),
			static_cast<int>(keyMsg.size()),
			*peer
		);
	}

	// 5) now consume KEY and MOVE packets
	char recvBuf[1024];
	while (mRunning)
	{
		int bytes = mUdpClient.GetSocket()
			->ReceiveFrom(recvBuf, sizeof(recvBuf), fromAddr);

		if (bytes < 0)
		{
			int err = -bytes;
			if (err != WSAEMSGSIZE)
				std::cerr << "ReceiveFrom error: " << err << "\n";
			continue;
		}

		if (bytes == 0)
		{
			continue;
		}

		if (bytes == (int)MOVE_PACKET_SIZE)
		{
			auto const* p = reinterpret_cast<const MovePacket*>(recvBuf);
			if (p->type == static_cast<uint8_t>(Action::MOVE))
			{
				uint32_t actorId = ntohl(p->actorId);

		
				uint32_t encX = p->x, encY = p->y;
				uint32_t secret = 0;
				auto it = mSharedKeys.find(actorId);
				if (it != mSharedKeys.end())
					secret = it->second;

				uint32_t rawNetX = encX ^ secret;
				uint32_t rawNetY = encY ^ secret;
				float x = aethernet_ntohf(rawNetX);
				float y = aethernet_ntohf(rawNetY);

				std::lock_guard lock(mMutex);
				mActors[actorId] = { x, y };
			}
			continue;
		}

		recvBuf[bytes] = '\0';
		std::string msg(recvBuf);

		if (msg.rfind("KEY ", 0) == 0)
		{
			std::istringstream is(msg.substr(4));
			uint32_t peerId;
			int      peerPub;
			is >> peerId >> peerPub;

			int shared = SecurityService::ComputeShared(peerPub, mPrivateKey);
			{
				std::lock_guard lock(mMutex);
				mSharedKeys[peerId] = shared;
			}

			std::string reply =
				"KEY " + std::to_string(mGClientId)
				+ " " + std::to_string(mPublicKey)
				+ "\n";
			mUdpClient.GetSocket()
				->SendTo(reply.data(),
					static_cast<int>(reply.size()),
					fromAddr);

			continue;
		}

		{
			std::lock_guard lock(mMutex);
			mPeers.clear();

			std::stringstream ss(msg);
			std::string line;
			while (std::getline(ss, line))
			{
				if (line.empty()) continue;
				std::istringstream ls(line);

				uint32_t peerId;
				std::string endpoint;
				ls >> peerId >> endpoint;
				if (!peerId || endpoint.empty()) continue;

				if (auto peer = AetherNet::SocketAddressFactory::CreateIPv4FromString(endpoint))
					mPeers.emplace_back(peerId, peer);
			}

			std::vector<AetherNet::SocketAddressPtr> addrs;
			addrs.reserve(mPeers.size());
			for (auto& pr : mPeers)
				addrs.push_back(pr.second);

			mPuncher->punchAll(addrs);
		}
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

	float X = mActors[mGClientId].x;
	float Y = mActors[mGClientId].y;

	for (auto const& [peerId, peer] : mPeers)
	{
		auto it = mSharedKeys.find(peerId);
		uint32_t secret = (it != mSharedKeys.end())
			? it->second
			: 0u;           

		// XOR-encryption with secret
		uint32_t netX = aethernet_htonf(X) ^ secret;
		uint32_t netY = aethernet_htonf(Y) ^ secret;

		packet.x = netX;
		packet.y = netY;

		// send encrypted packet
		mUdpClient.GetSocket()->SendTo(
			&packet,
			MOVE_PACKET_SIZE,
			*peer
		);
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