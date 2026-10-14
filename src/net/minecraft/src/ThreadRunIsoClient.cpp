#include "ThreadRunIsoClient.h"

#include <chrono>

#include "CanvasIsomPreview.h"

ThreadRunIsoClient::ThreadRunIsoClient(CanvasIsomPreview *canvasisompreview)
	: isoCanvas(canvasisompreview)
{
}

ThreadRunIsoClient::~ThreadRunIsoClient()
{
	if (worker.joinable())
		worker.detach();
}

void ThreadRunIsoClient::start()
{
	worker = std::thread(&ThreadRunIsoClient::run, this);
	worker.detach();
}

void ThreadRunIsoClient::run()
{
	while (CanvasIsomPreview::isRunning(isoCanvas))
	{
		isoCanvas->showNextBuffer();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}
