#include "ItemAxe.h"
#include "Block.h"
#include <vector>

namespace {
// The effective-block list has to exist BEFORE the ItemTool base ctor
// runs, so build it lazily in a static vector and hand its storage to
// the base. Leaving it empty made every tool return strength 1.0
// (hand speed), which is the "all tools are slow" bug.
std::vector<Block*> &axeBlockList()
{
	static std::vector<Block*> v = {
		Block::planks,
		Block::bookShelf,
		Block::wood,
		Block::chest
	};
	return v;
}
}


Block* ItemAxe::blocksEffectiveAgainst[] = { nullptr };
int ItemAxe::numBlocksEffectiveAgainst = 0;
bool ItemAxe::initialized = false;

ItemAxe::ItemAxe(int i, EnumToolMaterial enumtoolmaterial)
    : ItemTool(i, 3, enumtoolmaterial, axeBlockList().data(), (int)axeBlockList().size()) {
    if (!initialized) {
        // blocksEffectiveAgainst = { Block::planks, Block::bookShelf, Block::wood, Block::chest };
        initialized = true;
    }
}
