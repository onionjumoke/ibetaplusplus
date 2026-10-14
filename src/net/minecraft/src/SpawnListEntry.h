#pragma once

#include <typeindex>
#include "java/Type.h"

// net.minecraft.src.SpawnListEntry
// Java Class replaced with std::type_index
class SpawnListEntry
{
public:
	SpawnListEntry(std::type_index cls, int_t i);

	std::type_index entityClass;
	int_t spawnRarityRate;
};
