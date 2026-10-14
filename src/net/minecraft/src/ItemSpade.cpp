#include "ItemSpade.h"
#include "Block.h"
#include "BlockGrass.h"
#include <vector>

namespace {
// The effective-block list has to exist BEFORE the ItemTool base ctor
// runs, so build it lazily in a static vector and hand its storage to
// the base. Leaving it empty made every tool return strength 1.0
// (hand speed), which is the "all tools are slow" bug.
std::vector<Block*> &spadeBlockList()
{
	static std::vector<Block*> v = {
		Block::grass,
		Block::dirt,
		Block::sand,
		Block::gravel,
		Block::snow,
		Block::blockSnow,
		Block::blockClay,
		Block::tilledField
	};
	return v;
}
}


Block* ItemSpade::blocksEffectiveAgainst[] = { nullptr };
int ItemSpade::numBlocksEffectiveAgainst = 0;
bool ItemSpade::initialized = false;

ItemSpade::ItemSpade(int i, EnumToolMaterial enumtoolmaterial)
    : ItemTool(i, 1, enumtoolmaterial, spadeBlockList().data(), (int)spadeBlockList().size()) {
    if (!initialized) {
        // blocksEffectiveAgainst = { Block::grass, Block::dirt, Block::sand, Block::gravel, Block::snow, Block::blockSnow, Block::blockClay, Block::tilledField };
        initialized = true;
    }
}

bool ItemSpade::canHarvestBlock(Block* block) {
    if (block == Block::snow) return true;
    return block == Block::blockSnow;
}
