#include "ItemLeaves.h"
#include "Block.h"
#include "ColorizerFoliage.h"
#include "BlockLeaves.h"

ItemLeaves::ItemLeaves(int i)
    : ItemBlock(i) {
    setMaxDamage(0);
    setHasSubtypes(true);
}

int ItemLeaves::getPlacedBlockMetadata(int i) {
    return i | 8;
}

int ItemLeaves::getIconFromDamage(int i) {
    return Block::leaves->getBlockTextureFromSideAndMetadata(0, i);
}

int ItemLeaves::getColorFromDamage(int i) {
    if ((i & 1) == 1) {
        return ColorizerFoliage::getFoliageColorPine();
    }
    if ((i & 2) == 2) {
        return ColorizerFoliage::getFoliageColorBirch();
    } else {
        return ColorizerFoliage::getSpawnDelay();
    }
}
