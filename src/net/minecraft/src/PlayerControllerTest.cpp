#include "PlayerControllerTest.h"
#include "EntityPlayerSP.h"

#include "Block.h"
#include "EntityPlayer.h"
#include "InventoryPlayer.h"
#include "ItemStack.h"
#include "Minecraft.h"
#include "Session.h"
#include "World.h"

PlayerControllerTest::PlayerControllerTest(Minecraft *minecraft)
	: PlayerController(minecraft)
{
	field_1064_b = true;
}

void PlayerControllerTest::func_6473_b(EntityPlayer *entityplayer)
{
	for (int_t i = 0; i < 9 && i < (int_t)Session::registeredBlocksList.size(); i++)
	{
		if (entityplayer->inventory->mainInventory[i] == nullptr)
			mc->thePlayer->inventory->mainInventory[i] = new ItemStack(Session::registeredBlocksList[i]);
		else
			mc->thePlayer->inventory->mainInventory[i]->stackSize = 1;
	}
}

bool PlayerControllerTest::shouldDrawHUD()
{
	return false;
}

void PlayerControllerTest::func_717_a(World *world)
{
	PlayerController::func_717_a(world);
}

void PlayerControllerTest::updateController()
{
}
