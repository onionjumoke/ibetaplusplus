#pragma once

#include "IChunkProvider.h"
#include "java/Random.h"
#include "java/Type.h"

class World;
class Chunk;

// Layered generator for the optional "Superflat" overworld and the (always)
// flat Nether. Builds each chunk's 16x16x128 block array directly, so there is
// no noise/decoration pass.
class ChunkProviderFlat : public IChunkProvider
{
public:
	enum Mode
	{
		SUPERFLAT, // bedrock y60, dirt y61-63, grass y64, one oak tree at (0,0)
		NETHER     // 32 layers of netherrack + lava pools + glowstone + soul sand
	};

	ChunkProviderFlat(World *world, long_t seed, Mode mode);
	~ChunkProviderFlat() override = default;

	bool   chunkExists(int_t i, int_t j) override;
	Chunk *provideChunk(int_t i, int_t j) override;
	Chunk *prepareChunk(int_t i, int_t j) override;
	void   populate(IChunkProvider *ichunkprovider, int_t i, int_t j) override;
	bool   saveChunks(bool flag, IProgressUpdate *iprogressupdate) override;
	bool   unload100OldestChunks() override;
	bool   canSave() override;
	jstring makeString() override;

private:
	World *worldObj;
	long_t baseSeed; // per-chunk RNG derived from this + (chunkX,chunkZ)
	Mode   mode;
};
