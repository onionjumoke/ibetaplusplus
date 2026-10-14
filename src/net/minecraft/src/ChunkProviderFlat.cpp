#include "ChunkProviderFlat.h"

#include <cstdlib>
#include <vector>

#include "World.h"
#include "Chunk.h"
#include "Block.h"
#include "BlockGrass.h"
#include "BlockLeaves.h"
#include "WorldGenTrees.h"

namespace
{
inline void put(std::vector<byte_t> &blocks, int x, int y, int z, int id)
{
	blocks[(x << 11) | (z << 7) | y] = (byte_t)id;
}

// Scatter one ore type as short random-walk veins confined to the stone band
// (y61..y63). Everything is clamped, so veins near a chunk edge just get
// shallower rather than leaking into the dirt or neighbouring chunk.
void scatterOre(std::vector<byte_t> &blocks, Random &r, int blockId, int veins, int minSize, int maxSize)
{
	if (veins <= 0)
		return;
	const int yLo = 61, yHi = 63;

	for (int v = 0; v < veins; v++)
	{
		int size = minSize;
		if (maxSize > minSize)
			size += r.nextInt(maxSize - minSize + 1);

		int x = r.nextInt(16);
		int z = r.nextInt(16);
		int y = yLo + r.nextInt(yHi - yLo + 1);

		for (int n = 0; n < size; n++)
		{
			if (x >= 0 && x < 16 && z >= 0 && z < 16 && y >= yLo && y <= yHi)
				put(blocks, x, y, z, blockId);
			x += r.nextInt(3) - 1;
			y += r.nextInt(3) - 1;
			z += r.nextInt(3) - 1;
		}
	}
}
} // namespace

ChunkProviderFlat::ChunkProviderFlat(World *world, long_t seed, Mode modeIn)
	: worldObj(world)
	, baseSeed(seed)
	, mode(modeIn)
{
}

bool ChunkProviderFlat::chunkExists(int_t, int_t) { return false; }

Chunk *ChunkProviderFlat::prepareChunk(int_t i, int_t j) { return provideChunk(i, j); }

Chunk *ChunkProviderFlat::provideChunk(int_t i, int_t j)
{
	std::vector<byte_t> blocks(16 * 16 * 128, 0);

	// Per-chunk RNG so ore/feature layout is identical however many times (or
	// in whatever order) this chunk is generated.
	Random r(baseSeed + (long_t)i * 3418731287125636LL + (long_t)j * 132897987541LL);

	if (mode == SUPERFLAT)
	{
		for (int x = 0; x < 16; x++)
			for (int z = 0; z < 16; z++)
			{
				put(blocks, x, 60, z, Block::bedrock->blockID);
				put(blocks, x, 61, z, Block::stone->blockID);
				put(blocks, x, 62, z, Block::stone->blockID);
				put(blocks, x, 63, z, Block::stone->blockID);
				put(blocks, x, 64, z, Block::dirt->blockID);
				put(blocks, x, 65, z, Block::dirt->blockID);
				put(blocks, x, 66, z, Block::dirt->blockID);
				put(blocks, x, 67, z, Block::grass->blockID);
			}

		// Ore tiers (in the 3 stone layers only). Coal + iron are super common,
		// gold is rare, redstone rarer, diamond extremely rare and never in a
		// vein smaller than 4.
		scatterOre(blocks, r, Block::oreCoal->blockID, 8, 4, 10);
		scatterOre(blocks, r, Block::oreIron->blockID, 8, 3, 7);
		scatterOre(blocks, r, Block::oreGold->blockID, 2, 2, 4);
		scatterOre(blocks, r, Block::oreRedstone->blockID, 1, 3, 5);
		if (r.nextInt(4) == 0) // ~25% of chunks
			scatterOre(blocks, r, Block::oreDiamond->blockID, 1, 4, 6);

		// A little water hole in the first chunk.
		if (i == 0 && j == 0)
		{
			for (int dx = -1; dx <= 1; dx++)
				for (int dz = -1; dz <= 1; dz++)
				{
					int x = 3 + dx, z = 3 + dz;
					put(blocks, x, 67, z, Block::waterStill->blockID);
					put(blocks, x, 66, z, Block::waterStill->blockID);
					put(blocks, x, 65, z, Block::dirt->blockID);
				}
		}
	}
	else // NETHER
	{
		for (int x = 0; x < 16; x++)
			for (int z = 0; z < 16; z++)
				for (int y = 0; y < 32; y++)
					put(blocks, x, y, z, Block::netherrack->blockID);

		// Lava pools sunk into the top of the netherrack.
		for (int t = 0; t < 5; t++)
		{
			int cx = r.nextInt(16), cz = r.nextInt(16), rad = 2 + r.nextInt(3);
			for (int dx = -rad; dx <= rad; dx++)
				for (int dz = -rad; dz <= rad; dz++)
				{
					if (dx * dx + dz * dz > rad * rad)
						continue;
					int x = cx + dx, z = cz + dz;
					if (x < 0 || x > 15 || z < 0 || z > 15)
						continue;
					int depth = 1 + r.nextInt(3);
					for (int y = 0; y < depth; y++)
						put(blocks, x, 31 - y, z, Block::lavaStill->blockID);
				}
		}

		// Glowstone clusters hanging in the upper netherrack.
		for (int t = 0; t < 4; t++)
		{
			int cx = r.nextInt(16), cz = r.nextInt(16), cy = 27 + r.nextInt(4);
			for (int dx = -1; dx <= 1; dx++)
				for (int dz = -1; dz <= 1; dz++)
					for (int dy = 0; dy <= 1; dy++)
						if (r.nextInt(3) == 0)
						{
							int x = cx + dx, z = cz + dz, y = cy + dy;
							if (x >= 0 && x < 16 && z >= 0 && z < 16 && y < 32)
								put(blocks, x, y, z, Block::glowStone->blockID);
						}
		}

		// Soul sand patches (Beta 1.7.3 has no nether quartz block).
		for (int t = 0; t < 3; t++)
		{
			int cx = r.nextInt(16), cz = r.nextInt(16), rad = 1 + r.nextInt(2);
			for (int dx = -rad; dx <= rad; dx++)
				for (int dz = -rad; dz <= rad; dz++)
					if (r.nextInt(2) == 0)
					{
						int x = cx + dx, z = cz + dz;
						if (x >= 0 && x < 16 && z >= 0 && z < 16)
							put(blocks, x, 31, z, Block::slowSand->blockID);
					}
		}
	}

	Chunk *chunk = new Chunk(worldObj, blocks, i, j);
	chunk->hasRealData = true;
	chunk->generateSkylightMap();
	return chunk;
}

void ChunkProviderFlat::populate(IChunkProvider *, int_t i, int_t j)
{
	// A single real oak (medium-sized, vanilla WorldGenTrees — not a custom
	// build) in the first chunk. Running it here rather than assembling blocks
	// by hand lets the world set light/height correctly around it.
	if (mode == SUPERFLAT && i == 0 && j == 0)
	{
		Random treeRand(baseSeed + 1284863LL);
		WorldGenTrees gen;
		gen.generate(worldObj, treeRand, 8, 68, 8);
	}
}

bool ChunkProviderFlat::saveChunks(bool, IProgressUpdate *) { return true; }
bool ChunkProviderFlat::unload100OldestChunks() { return false; }
bool ChunkProviderFlat::canSave() { return false; }
jstring ChunkProviderFlat::makeString() { return u"FlatChunkCache"; }
