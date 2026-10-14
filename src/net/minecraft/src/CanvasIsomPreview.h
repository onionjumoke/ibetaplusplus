#pragma once

// net.minecraft.src.CanvasIsomPreview
class CanvasIsomPreview
{
public:
	static bool isRunning(CanvasIsomPreview *canvasisompreview) { return canvasisompreview != nullptr && canvasisompreview->running; }
	void showNextBuffer() {}

	bool running = false;
};
