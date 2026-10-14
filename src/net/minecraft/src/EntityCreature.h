#pragma once

#include <memory>
#include "EntityLiving.h"

class World;
class PathEntity;
class Entity;

// net.minecraft.src.EntityCreature
class EntityCreature : public EntityLiving
{
public:
	EntityCreature(World *world);
	~EntityCreature() override;

protected:
	virtual bool isMovementCeased();
	void updatePlayerActionState() override;
	virtual void func_31026_E();
	virtual void attackEntity(Entity *entity, float f);
	virtual void attackBlockedEntity(Entity *entity, float f);
	virtual float getBlockPathWeight(int_t i, int_t j, int_t k);
	virtual Entity *findPlayerToAttack();

public:
	bool getCanSpawnHere() override;
	bool hasPath();
	void setPathToEntity(PathEntity *pathentity);
	Entity *getTarget();
	void setTarget(Entity *entity);

private:
	std::unique_ptr<PathEntity> pathToEntity;
protected:
	Entity *playerToAttack;
	bool hasAttacked;
};
