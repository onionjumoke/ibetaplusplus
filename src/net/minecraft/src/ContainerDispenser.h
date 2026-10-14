#pragma once

#include "Container.h"

class IInventory;
class TileEntityDispenser;
class EntityPlayer;

// net.minecraft.src.ContainerDispenser
class ContainerDispenser : public Container
{
public:
	ContainerDispenser(IInventory *iinventory, TileEntityDispenser *tileentitydispenser);

	bool isUsableByPlayer(EntityPlayer *entityplayer) override;

private:
	TileEntityDispenser *dispenser;
};
