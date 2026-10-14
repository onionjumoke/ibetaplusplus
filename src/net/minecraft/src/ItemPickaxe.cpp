#include "ItemPickaxe.h"
#include "Block.h"
#include <vector>
#include "Material.h"

namespace {
// The effective-block list has to exist BEFORE the ItemTool base ctor
// runs, so build it lazily in a static vector and hand its storage to
// the base. Leaving it empty made every tool return strength 1.0
// (hand speed), which is the "all tools are slow" bug.
std::vector<Block*> &pickaxeBlockList()
{
	static std::vector<Block*> v = {
		Block::cobblestone,
		Block::stairDouble,
		Block::stairSingle,
		Block::stone,
		Block::sandStone,
		Block::cobblestoneMossy,
		Block::oreIron,
		Block::blockSteel,
		Block::oreCoal,
		Block::blockGold,
		Block::oreGold,
		Block::oreDiamond,
		Block::blockDiamond,
		Block::ice,
		Block::netherrack,
		Block::oreLapis,
		Block::blockLapis
	};
	return v;
}
}


Block* ItemPickaxe::blocksEffectiveAgainst[] = { nullptr };
int ItemPickaxe::numBlocksEffectiveAgainst = 0;
bool ItemPickaxe::initialized = false;

ItemPickaxe::ItemPickaxe(int i, EnumToolMaterial enumtoolmaterial)
    : ItemTool(i, 2, enumtoolmaterial, pickaxeBlockList().data(), (int)pickaxeBlockList().size()) {
    if (!initialized) {
        // blocksEffectiveAgainst = { Block::cobblestone, Block::stairDouble, Block::stairSingle, Block::stone, Block::sandStone, Block::cobblestoneMossy, Block::oreIron, Block::blockSteel, Block::oreCoal, Block::blockGold, Block::oreGold, Block::oreDiamond, Block::blockDiamond, Block::ice, Block::netherrack, Block::oreLapis, Block::blockLapis };
        initialized = true;
    }
}

bool ItemPickaxe::canHarvestBlock(Block* block) {
    if (block == Block::obsidian) {
        return EnumToolMaterialHelper::getHarvestLevel(toolMaterial) == 3;
    }
    if (block == Block::blockDiamond || block == Block::oreDiamond) {
        return EnumToolMaterialHelper::getHarvestLevel(toolMaterial) >= 2;
    }
    if (block == Block::blockGold || block == Block::oreGold) {
        return EnumToolMaterialHelper::getHarvestLevel(toolMaterial) >= 2;
    }
    if (block == Block::blockSteel || block == Block::oreIron) {
        return EnumToolMaterialHelper::getHarvestLevel(toolMaterial) >= 1;
    }
    if (block == Block::blockLapis || block == Block::oreLapis) {
        return EnumToolMaterialHelper::getHarvestLevel(toolMaterial) >= 1;
    }
    if (block == Block::oreRedstone || block == Block::oreRedstoneGlowing) {
        return EnumToolMaterialHelper::getHarvestLevel(toolMaterial) >= 2;
    }
    if (block->blockMaterial == Material::rock) {
        return true;
    }
    return block->blockMaterial == Material::iron;
}
