#include "NetworkManager.h"

#include <chrono>
#include <iostream>
#include <stdexcept>

#include "NetHandler.h"
#include "Packet.h"
#include "ThreadCloseConnection.h"
#include "java/JavaNetwork.h"
#include "java/System.h"

int_t NetworkManager::field_28145_d[256];
int_t NetworkManager::field_28144_e[256];
int_t NetworkManager::numReadThreads = 0;
int_t NetworkManager::numWriteThreads = 0;
std::mutex NetworkManager::threadSyncObject;

NetworkManager::NetworkManager(const std::string &host, int_t port, const std::string &s, NetHandler *nethandler)
	: networkSocket(JavaNetwork::createSocket())
	, running(true)
	, serverTerminating(false)
	, terminating(false)
	, terminationReason("")
	, netHandler(nethandler)
	, chunkDataSendCounter(0)
	, timeSinceLastRead(0)
	, sendQueueByteLength(0)
	, field_20100_w(50)
{
	if (networkSocket == nullptr || !networkSocket->connect(host, port))
		throw std::runtime_error("Connection refused: " + host + ":" + std::to_string(port));

	remoteSocketAddress = networkSocket->getRemoteSocketAddress();
	socketInputStream = JavaNetwork::createInputStream(*networkSocket);
	socketOutputStream = JavaNetwork::createOutputStream(*networkSocket);
	readThread = std::thread(&NetworkManager::readThreadRun, this);
	writeThread = std::thread(&NetworkManager::writeThreadRun, this);
	(void)s;
}

NetworkManager::~NetworkManager()
{
	networkShutdown("disconnect.closed", std::vector<std::string>());
	if (readThread.joinable() && readThread.get_id() != std::this_thread::get_id())
		readThread.join();
	if (writeThread.joinable() && writeThread.get_id() != std::this_thread::get_id())
		writeThread.join();
}

void NetworkManager::addToSendQueue(Packet *packet)
{
	if (serverTerminating || packet == nullptr)
		return;

	std::lock_guard<std::mutex> guard(sendQueueLock);
	sendQueueByteLength += packet->getPacketSize() + 1;
	if (packet->isChunkDataPacket)
		chunkDataPackets.emplace_back(packet);
	else
		dataPackets.emplace_back(packet);
	wakeThreads();
}

bool NetworkManager::sendPacket()
{
	bool flag = false;
	try
	{
		if (socketOutputStream == nullptr)
			return false;

		std::unique_ptr<Packet> packet;
		{
			std::lock_guard<std::mutex> guard(sendQueueLock);
			if (!dataPackets.empty() && (chunkDataSendCounter == 0 || System::currentTimeMillis() - dataPackets[0]->creationTimeMillis >= chunkDataSendCounter))
			{
				packet = std::move(dataPackets.front());
				dataPackets.pop_front();
				sendQueueByteLength -= packet->getPacketSize() + 1;
			}
		}
		if (packet != nullptr)
		{
			Packet::writePacket(packet.get(), *socketOutputStream);
			field_28144_e[packet->getPacketId()] += packet->getPacketSize() + 1;
			flag = true;
		}

		std::unique_ptr<Packet> packet1;
		{
			std::lock_guard<std::mutex> guard(sendQueueLock);
			if (field_20100_w-- <= 0 && !chunkDataPackets.empty() && (chunkDataSendCounter == 0 || System::currentTimeMillis() - chunkDataPackets[0]->creationTimeMillis >= chunkDataSendCounter))
			{
				packet1 = std::move(chunkDataPackets.front());
				chunkDataPackets.pop_front();
				sendQueueByteLength -= packet1->getPacketSize() + 1;
			}
		}
		if (packet1 != nullptr)
		{
			Packet::writePacket(packet1.get(), *socketOutputStream);
			field_28144_e[packet1->getPacketId()] += packet1->getPacketSize() + 1;
			field_20100_w = 0;
			flag = true;
		}
	}
	catch (std::exception &exception)
	{
		if (!terminating)
			onNetworkError(exception);
		return false;
	}
	return flag;
}

void NetworkManager::wakeThreads()
{
	threadSleepCondition.notify_all();
}

bool NetworkManager::readPacket()
{
	bool flag = false;
	try
	{
		if (socketInputStream == nullptr)
			return false;

		std::unique_ptr<Packet> packet = Packet::readPacket(*socketInputStream, netHandler != nullptr && netHandler->isServerHandler());
		if (packet != nullptr)
		{
			field_28145_d[packet->getPacketId()] += packet->getPacketSize() + 1;
			std::lock_guard<std::mutex> guard(readQueueLock);
			readPackets.emplace_back(std::move(packet));
			flag = true;
		}
		else
		{
			networkShutdown("disconnect.endOfStream", std::vector<std::string>());
		}
	}
	catch (std::exception &exception)
	{
		if (!terminating)
			onNetworkError(exception);
		return false;
	}
	return flag;
}

void NetworkManager::onNetworkError(std::exception &exception)
{
	std::cerr << exception.what() << std::endl;
	networkShutdown("disconnect.genericReason", std::vector<std::string>{std::string("Internal exception: ") + exception.what()});
}

void NetworkManager::networkShutdown(const std::string &s, const std::vector<std::string> &aobj)
{
	if (!running)
		return;

	terminating = true;
	terminationReason = s;
	field_20101_t = aobj;
	running = false;
	wakeThreads();

	// Only close the socket here so a blocked recv() in the read thread returns
	// and both threads observe running == false and exit. Do NOT free the stream
	// or socket objects: another thread may still be inside is.get()/flush().
	// They are released in the destructor, after both threads have been joined.
	if (networkSocket != nullptr)
		networkSocket->close();
}

void NetworkManager::processReadPackets()
{
	if (sendQueueByteLength > 0x100000)
		networkShutdown("disconnect.overflow", std::vector<std::string>());

	bool empty;
	{
		std::lock_guard<std::mutex> guard(readQueueLock);
		empty = readPackets.empty();
	}

	if (empty)
	{
		if (timeSinceLastRead++ == 1200)
			networkShutdown("disconnect.timeout", std::vector<std::string>());
	}
	else
	{
		timeSinceLastRead = 0;
	}

	for (int_t i = 100; i-- >= 0;)
	{
		std::unique_ptr<Packet> packet;
		{
			std::lock_guard<std::mutex> guard(readQueueLock);
			if (readPackets.empty())
				break;
			packet = std::move(readPackets.front());
			readPackets.pop_front();
		}
		if (packet != nullptr && netHandler != nullptr)
			packet->processPacket(*netHandler);
	}

	wakeThreads();

	{
		std::lock_guard<std::mutex> guard(readQueueLock);
		empty = readPackets.empty();
	}
	if (terminating && empty && netHandler != nullptr)
		netHandler->handleErrorMessage(terminationReason, field_20101_t);
}

void NetworkManager::func_28142_c()
{
	wakeThreads();
	serverTerminating = true;
	if (networkSocket != nullptr)
		networkSocket->close();

	ThreadCloseConnection *threadcloseconnection = new ThreadCloseConnection(this);
	threadcloseconnection->start();
}

void NetworkManager::readThreadRun()
{
	numReadThreads++;
	try
	{
		while (running && !serverTerminating)
		{
			while (running && !serverTerminating && readPacket())
			{
			}
			sleepThread();
		}
	}
	catch (...)
	{
		numReadThreads--;
		throw;
	}
	numReadThreads--;
}

void NetworkManager::writeThreadRun()
{
	numWriteThreads++;
	try
	{
		while (running)
		{
			while (running && sendPacket())
			{
			}
			sleepThread();
			try
			{
				if (socketOutputStream != nullptr)
					socketOutputStream->flush();
			}
			catch (std::exception &exception)
			{
				if (!terminating)
					onNetworkError(exception);
				std::cerr << exception.what() << std::endl;
			}
		}
	}
	catch (...)
	{
		numWriteThreads--;
		throw;
	}
	numWriteThreads--;
}

void NetworkManager::sleepThread()
{
	std::unique_lock<std::mutex> lock(threadSleepLock);
	threadSleepCondition.wait_for(lock, std::chrono::milliseconds(100));
}

bool NetworkManager::isRunning(NetworkManager *networkmanager)
{
	return networkmanager != nullptr && networkmanager->running;
}

bool NetworkManager::isServerTerminating(NetworkManager *networkmanager)
{
	return networkmanager != nullptr && networkmanager->serverTerminating;
}

bool NetworkManager::readNetworkPacket(NetworkManager *networkmanager)
{
	return networkmanager != nullptr && networkmanager->readPacket();
}

bool NetworkManager::sendNetworkPacket(NetworkManager *networkmanager)
{
	return networkmanager != nullptr && networkmanager->sendPacket();
}

bool NetworkManager::func_28138_e(NetworkManager *networkmanager)
{
	return networkmanager != nullptr && networkmanager->terminating;
}

void NetworkManager::func_30005_a(NetworkManager *networkmanager, std::exception &exception)
{
	if (networkmanager != nullptr)
		networkmanager->onNetworkError(exception);
}

std::thread *NetworkManager::getReadThread(NetworkManager *networkmanager)
{
	return networkmanager != nullptr ? &networkmanager->readThread : nullptr;
}

std::thread *NetworkManager::getWriteThread(NetworkManager *networkmanager)
{
	return networkmanager != nullptr ? &networkmanager->writeThread : nullptr;
}

std::ostream *NetworkManager::func_28140_f(NetworkManager *networkmanager)
{
	return networkmanager != nullptr ? networkmanager->socketOutputStream.get() : nullptr;
}
