#include "SpawnListEntry.h"

SpawnListEntry::SpawnListEntry(std::type_index cls, int_t i) :
	entityClass(cls)
{
	spawnRarityRate = i;
}
