#include "GameWindowListener.h"

#include <cstdlib>
#include <iostream>

#include "Minecraft.h"

GameWindowListener::GameWindowListener(Minecraft *minecraft, std::thread *thread)
    : mc(minecraft), mcThread(thread)
{
}

void GameWindowListener::windowClosing()
{
    mc->shutdown();

    if (mcThread != nullptr && mcThread->joinable())
    {
        try
        {
            mcThread->join();
        }
        catch (...)
        {
            std::cerr << "GameWindowListener: interrupted while joining mcThread" << std::endl;
        }
    }

    std::exit(0);
}
