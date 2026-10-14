#pragma once

#include <deque>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <string>
#include <thread>
#include <vector>
#include <atomic>
#include <iosfwd>

#include "java/Type.h"

namespace JavaNetwork
{
class Socket;
}

class NetHandler;
class Packet;

// net.minecraft.src.NetworkManager
class NetworkManager
{
public:
	NetworkManager(const std::string &host, int_t port, const std::string &s, NetHandler *nethandler);
	~NetworkManager();

	void addToSendQueue(Packet *packet);
	void processReadPackets();
	void wakeThreads();
	void networkShutdown(const std::string &s, const std::vector<std::string> &aobj);
	void func_28142_c();
	void flushQueue() { func_28142_c(); }

	static bool isRunning(NetworkManager *networkmanager);
	static bool isServerTerminating(NetworkManager *networkmanager);
	static bool readNetworkPacket(NetworkManager *networkmanager);
	static bool sendNetworkPacket(NetworkManager *networkmanager);
	static bool func_28138_e(NetworkManager *networkmanager);
	static void func_30005_a(NetworkManager *networkmanager, std::exception &exception);
	static std::thread *getReadThread(NetworkManager *networkmanager);
	static std::thread *getWriteThread(NetworkManager *networkmanager);

	static std::mutex threadSyncObject;
	static std::ostream *func_28140_f(NetworkManager *networkmanager);

	static int_t field_28145_d[256];
	static int_t field_28144_e[256];
	static int_t numReadThreads;
	static int_t numWriteThreads;

	int_t chunkDataSendCounter;

private:
	bool readPacket();
	bool sendPacket();
	void onNetworkError(std::exception &exception);
	void readThreadRun();
	void writeThreadRun();
	void sleepThread();

	std::mutex sendQueueLock;
	std::mutex readQueueLock;
	std::mutex threadSleepLock;
	std::condition_variable threadSleepCondition;
	std::unique_ptr<JavaNetwork::Socket> networkSocket;
	std::unique_ptr<std::istream> socketInputStream;
	std::unique_ptr<std::ostream> socketOutputStream;
	std::string remoteSocketAddress;
	std::atomic_bool running;
	std::atomic_bool serverTerminating;
	std::atomic_bool terminating;
	std::string terminationReason;
	std::vector<std::string> field_20101_t;
	std::deque<std::unique_ptr<Packet>> readPackets;
	std::deque<std::unique_ptr<Packet>> dataPackets;
	std::deque<std::unique_ptr<Packet>> chunkDataPackets;
	NetHandler *netHandler;
	std::thread readThread;
	std::thread writeThread;
	int_t timeSinceLastRead;
	int_t sendQueueByteLength;
	int_t field_20100_w;
};
