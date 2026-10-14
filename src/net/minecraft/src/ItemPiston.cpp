#include "ItemPiston.h"

ItemPiston::ItemPiston(int i)
    : ItemBlock(i) {
}

int ItemPiston::getPlacedBlockMetadata(int i) {
    return 7;
}
