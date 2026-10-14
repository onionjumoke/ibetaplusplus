#include "ContainerDispenser.h"

#include "IInventory.h"
#include "Slot.h"
#include "TileEntityDispenser.h"
#include "EntityPlayer.h"

ContainerDispenser::ContainerDispenser(IInventory *iinventory, TileEntityDispenser *tileentitydispenser)
{
	dispenser = tileentitydispenser;
	for(int_t i = 0; i < 3; i++)
	{
		for(int_t l = 0; l < 3; l++)
		{
			addSlot(new Slot(tileentitydispenser, l + i * 3, 62 + l * 18, 17 + i * 18));
		}
	}
	for(int_t j = 0; j < 3; j++)
	{
		for(int_t i1 = 0; i1 < 9; i1++)
		{
			addSlot(new Slot(iinventory, i1 + j * 9 + 9, 8 + i1 * 18, 84 + j * 18));
		}
	}
	for(int_t k = 0; k < 9; k++)
	{
		addSlot(new Slot(iinventory, k, 8 + k * 18, 142));
	}
}

bool ContainerDispenser::isUsableByPlayer(EntityPlayer *entityplayer)
{
	return dispenser->canInteractWith(entityplayer);
}
