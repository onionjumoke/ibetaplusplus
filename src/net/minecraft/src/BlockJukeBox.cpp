#include "BlockJukeBox.h"
#include "Material.h"
#include "World.h"
#include "TileEntityRecordPlayer.h"
#include "EntityItem.h"
#include "ItemStack.h"
#include "EntityPlayer.h"
#include "TileEntity.h"

BlockJukeBox::BlockJukeBox(int_t i, int_t j) : BlockContainer(i, j, Material::wood)
{
}

int_t BlockJukeBox::getBlockTextureFromSide(int_t i)
{
	return blockIndexInTexture + (i != 1 ? 0 : 1);
}

bool BlockJukeBox::blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
	if (world->getBlockMetadata(i, j, k) == 0) return false;
	getRecordItem(world, i, j, k);
	return true;
}

void BlockJukeBox::ejectRecord(World *world, int_t i, int_t j, int_t k, int_t l)
{
	if (world->multiplayerWorld) return;
	TileEntityRecordPlayer *tileentityrecordplayer = (TileEntityRecordPlayer *)world->getBlockTileEntity(i, j, k);
	tileentityrecordplayer->record = l;
	tileentityrecordplayer->onInventoryChanged();
	world->setBlockMetadataWithNotify(i, j, k, 1);
}

void BlockJukeBox::getRecordItem(World *world, int_t i, int_t j, int_t k)
{
	if (world->multiplayerWorld) return;
	TileEntityRecordPlayer *tileentityrecordplayer = (TileEntityRecordPlayer *)world->getBlockTileEntity(i, j, k);
	int_t l = tileentityrecordplayer->record;
	if (l == 0) return;
	world->isTrapdoorOpen(1005, i, j, k, 0);
	world->playRecord(jstring(), i, j, k);
	tileentityrecordplayer->record = 0;
	tileentityrecordplayer->onInventoryChanged();
	world->setBlockMetadataWithNotify(i, j, k, 0);
	int_t i1 = l;
	float f = 0.7f;
	double d  = (double)(world->rand.nextFloat() * f) + (double)(1.0f - f) * 0.5;
	double d1 = (double)(world->rand.nextFloat() * f) + (double)(1.0f - f) * 0.2 + 0.6;
	double d2 = (double)(world->rand.nextFloat() * f) + (double)(1.0f - f) * 0.5;
	EntityItem *entityitem = new EntityItem(world, (double)i + d, (double)j + d1, (double)k + d2, new ItemStack(i1, 1, 0));
	entityitem->delayBeforeCanPickup = 10;
	world->entityJoinedWorld(entityitem);
}

void BlockJukeBox::onBlockRemoval(World *world, int_t i, int_t j, int_t k)
{
	getRecordItem(world, i, j, k);
	BlockContainer::onBlockRemoval(world, i, j, k);
}

void BlockJukeBox::dropBlockAsItemWithChance(World *world, int_t i, int_t j, int_t k, int_t l, float f)
{
	if (world->multiplayerWorld) return;
	BlockContainer::dropBlockAsItemWithChance(world, i, j, k, l, f);
}

TileEntity *BlockJukeBox::getBlockEntity()
{
	return new TileEntityRecordPlayer();
}
