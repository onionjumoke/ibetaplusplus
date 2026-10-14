#pragma once

#include "java/Random.h"

class Entity;
class World;

// net.minecraft.src.Teleporter
class Teleporter
{
public:
	Teleporter();

	void func_4107_a(World *world, Entity *entity);
	bool func_4106_b(World *world, Entity *entity);
	bool func_4108_c(World *world, Entity *entity);

private:
	Random field_4232_a;
};
