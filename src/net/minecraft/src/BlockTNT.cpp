#include "BlockTNT.h"
#include "Material.h"
#include "World.h"
#include "Block.h"
#include "EntityTNTPrimed.h"
#include "ItemStack.h"
#include "EntityPlayer.h"
#include "Item.h"

BlockTNT::BlockTNT(int_t i, int_t j) : Block(i, j, Material::tnt)
{
}

int_t BlockTNT::getBlockTextureFromSide(int_t i)
{
	if (i == 0) return blockIndexInTexture + 2;
	if (i == 1) return blockIndexInTexture + 1;
	return blockIndexInTexture;
}

void BlockTNT::onBlockAdded(World *world, int_t i, int_t j, int_t k)
{
	Block::onBlockAdded(world, i, j, k);
	if (world->isBlockIndirectlyGettingPowered(i, j, k))
	{
		onBlockDestroyedByPlayer(world, i, j, k, 1);
		world->setBlockWithNotify(i, j, k, 0);
	}
}

void BlockTNT::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
	if (l > 0 && Block::blocksList[l]->canProvidePower() && world->isBlockIndirectlyGettingPowered(i, j, k))
	{
		onBlockDestroyedByPlayer(world, i, j, k, 1);
		world->setBlockWithNotify(i, j, k, 0);
	}
}

int_t BlockTNT::quantityDropped(Random &random) { return 0; }

void BlockTNT::onBlockDestroyedByExplosion(World *world, int_t i, int_t j, int_t k)
{
	EntityTNTPrimed *entitytntprimed = new EntityTNTPrimed(world, (float)i + 0.5f, (float)j + 0.5f, (float)k + 0.5f);
	entitytntprimed->fuse = world->rand.nextInt(entitytntprimed->fuse / 4) + entitytntprimed->fuse / 8;
	world->entityJoinedWorld(entitytntprimed);
}

void BlockTNT::onBlockDestroyedByPlayer(World *world, int_t i, int_t j, int_t k, int_t l)
{
	if (world->multiplayerWorld) return;
	if ((l & 1) == 0)
	{
		dropBlockAsItem_do(world, i, j, k, new ItemStack(Block::tnt->blockID, 1, 0));
	}
	else
	{
		EntityTNTPrimed *entitytntprimed = new EntityTNTPrimed(world, (float)i + 0.5f, (float)j + 0.5f, (float)k + 0.5f);
		world->entityJoinedWorld(entitytntprimed);
		world->playSoundAtEntity(entitytntprimed, u"random.fuse", 1.0f, 1.0f);
	}
}

void BlockTNT::onBlockClicked(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
	if (entityplayer->getCurrentEquippedItem() != nullptr &&
	    entityplayer->getCurrentEquippedItem()->itemID == Item::flintAndSteel->shiftedIndex)
	{
		world->setBlockMetadata(i, j, k, 1);
	}
	Block::onBlockClicked(world, i, j, k, entityplayer);
}

bool BlockTNT::blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
	return Block::blockActivated(world, i, j, k, entityplayer);
}
