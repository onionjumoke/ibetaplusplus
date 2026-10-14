#include "NetworkWriterThread.h"

#include <chrono>
#include <iostream>
#include <ostream>

#include "NetworkManager.h"

NetworkWriterThread::NetworkWriterThread(NetworkManager *networkmanager, const std::string &name)
    : netManager(networkmanager), threadName(name)
{
}

void NetworkWriterThread::start()
{
    alive = true;
    thread = std::thread(&NetworkWriterThread::run, this);
}

bool NetworkWriterThread::isAlive() const
{
    return alive.load();
}

void NetworkWriterThread::join()
{
    if (thread.joinable())
        thread.join();
}

void NetworkWriterThread::run()
{
    {
        std::lock_guard<std::mutex> lock(NetworkManager::threadSyncObject);
        NetworkManager::numWriteThreads++;
    }

    try
    {
        while (NetworkManager::isRunning(netManager))
        {
            while (NetworkManager::sendNetworkPacket(netManager)) ;

            try
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            catch (...) {}

            try
            {
                std::ostream *os = NetworkManager::func_28140_f(netManager);
                if (os != nullptr)
                    os->flush();
            }
            catch (std::exception &ioexception)
            {
                if (!NetworkManager::func_28138_e(netManager))
                {
                    NetworkManager::func_30005_a(netManager, ioexception);
                }
                std::cerr << ioexception.what() << std::endl;
            }
        }
    }
    catch (...)
    {
        // absorb
    }

    {
        std::lock_guard<std::mutex> lock(NetworkManager::threadSyncObject);
        NetworkManager::numWriteThreads--;
    }

    alive = false;
}
