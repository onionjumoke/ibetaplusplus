#pragma once

#include "Gui.h"
#include <vector>
#include <string>

class Minecraft;
class RenderItem;
class ChatLine;
class Random;

// net.minecraft.src.GuiIngame
class GuiIngame : public Gui
{
public:
	GuiIngame(Minecraft *minecraft);

	void renderGameOverlay(float_t partialTick, bool showDebug, int_t mouseX, int_t mouseY);

private:
	void renderPumpkinBlur(int_t w, int_t h);
	void renderVignette(float_t brightness, int_t w, int_t h);
	void renderPortalOverlay(float_t intensity, int_t w, int_t h);
	void renderInventorySlot(int_t slot, int_t x, int_t y, float_t partialTick);

public:
	void updateTick();
	void clearChatMessages();
	void addChatMessage(const std::string &msg);
	void setRecordPlayingMessage(const std::string &record);
	void addChatMessageTranslate(const std::string &key);

private:
	static RenderItem *itemRenderer;
	std::vector<ChatLine *> chatMessageList;
	Random *rand;
	Minecraft *mc;

public:
	std::string field_933_a;   // tab-completion target

private:
	int_t updateCounter;
	std::string recordPlaying;
	int_t recordPlayingUpFor;
	bool field_22065_l;        // record playing with color effect

public:
	float_t damageGuiPartialTime;
	float_t prevVignetteBrightness;
};
