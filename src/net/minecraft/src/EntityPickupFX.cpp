#include "EntityPickupFX.h"

#include "pc/OpenGL.h"

#include "Entity.h"
#include "EntityItem.h"
#include "ItemStack.h"
#include "MathHelper.h"
#include "RenderManager.h"
#include "Tessellator.h"
#include "World.h"

EntityPickupFX::EntityPickupFX(World *world, Entity *entity, Entity *entity1, float f)
	: EntityFX(world, entity->posX, entity->posY, entity->posZ, entity->motionX, entity->motionY, entity->motionZ)
{
	field_678_p = 0;
	field_677_q = 0;
	field_679_o = entity1;
	field_677_q = 3;
	field_676_r = f;

	// The picked-up entity is deleted by the world right after the pickup (in Java the GC
	// kept it alive while this effect still referenced it for ~3 ticks). Holding the raw
	// pointer here dereferenced freed memory -> crash in RenderManager::getEntityRenderObject
	// at typeid(*entity). Render a private copy we own instead, fully decoupled from the world.
	EntityItem *srcItem = dynamic_cast<EntityItem *>(entity);
	if (srcItem != nullptr && srcItem->item != nullptr)
	{
		EntityItem *copy = new EntityItem(world, entity->posX, entity->posY, entity->posZ, srcItem->item->copy());
		copy->rotationYaw = entity->rotationYaw;
		field_675_a = copy;
		ownsRenderEntity = true;
	}
	else
	{
		// Not an item (no such pickup type in beta 1.7.3); keep prior behaviour.
		field_675_a = entity;
		ownsRenderEntity = false;
	}
}

EntityPickupFX::~EntityPickupFX()
{
	if (ownsRenderEntity)
		delete field_675_a;
}

void EntityPickupFX::renderParticle(Tessellator *tessellator, float f, float f1, float f2, float f3, float f4, float f5)
{
	float f6 = ((float)field_678_p + f) / (float)field_677_q;
	f6 *= f6;
	double d  = field_675_a->posX;
	double d1 = field_675_a->posY;
	double d2 = field_675_a->posZ;
	double d3 = field_679_o->lastTickPosX + (field_679_o->posX - field_679_o->lastTickPosX) * (double)f;
	double d4 = field_679_o->lastTickPosY + (field_679_o->posY - field_679_o->lastTickPosY) * (double)f + (double)field_676_r;
	double d5 = field_679_o->lastTickPosZ + (field_679_o->posZ - field_679_o->lastTickPosZ) * (double)f;
	double d6 = d  + (d3 - d ) * (double)f6;
	double d7 = d1 + (d4 - d1) * (double)f6;
	double d8 = d2 + (d5 - d2) * (double)f6;
	int_t i = MathHelper::floor_double(d6);
	int_t j = MathHelper::floor_double(d7 + (double)(yOffset / 2.0f));
	int_t k = MathHelper::floor_double(d8);
	float f7 = worldObj->getLightBrightness(i, j, k);
	d6 -= interpPosX;
	d7 -= interpPosY;
	d8 -= interpPosZ;
	glColor4f(f7, f7, f7, 1.0f);
	RenderManager::instance->renderEntityWithPosYaw(field_675_a, (float)d6, (float)d7, (float)d8, field_675_a->rotationYaw, f);
}

void EntityPickupFX::onUpdate()
{
	field_678_p++;
	if (field_678_p == field_677_q)
	{
		setEntityDead();
	}
}

int_t EntityPickupFX::getFXLayer()
{
	return 3;
}
