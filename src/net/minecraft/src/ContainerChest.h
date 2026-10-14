#pragma once

#include "Container.h"

class IInventory;
class ItemStack;
class EntityPlayer;

// net.minecraft.src.ContainerChest
class ContainerChest : public Container
{
public:
	ContainerChest(IInventory *iinventory, IInventory *iinventory1);

	bool isUsableByPlayer(EntityPlayer *entityplayer) override;
	ItemStack *getStackInSlot(int_t i) override;

private:
	IInventory *lowerChestInventory;
	int_t numRows;
};
