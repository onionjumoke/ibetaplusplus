#pragma once

#include <any>
#include "java/Type.h"

// net.minecraft.src.WatchableObject
// Java Object replaced with std::any
class WatchableObject
{
public:
	WatchableObject(int_t i, int_t j, std::any obj)
		: objectType(i), dataValueId(j), watchedObject(std::move(obj)), isWatching(true) {}

	int_t getDataValueId() const { return dataValueId; }
	void setObject(std::any obj) { watchedObject = std::move(obj); }
	std::any getObject() const { return watchedObject; }
	int_t getObjectType() const { return objectType; }
	void setWatching(bool flag) { isWatching = flag; }

private:
	const int_t objectType;
	const int_t dataValueId;
	std::any watchedObject;
	bool isWatching;
};
