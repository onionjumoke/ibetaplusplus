#include "EntityChicken.h"

#include "Item.h"
#include "World.h"

EntityChicken::EntityChicken(World *world)
	: EntityAnimal(world)
{
	field_753_a = false;
	field_752_b = 0.0f;
	destPos = 0.0f;
	field_757_d = 0.0f;
	field_756_e = 0.0f;
	field_755_h = 1.0f;
	texture = String::fromUTF8("/mob/chicken.png");
	setSize(0.3f, 0.4f);
	health = 4;
	timeUntilNextEgg = rand.nextInt(6000) + 6000;
}

void EntityChicken::onLivingUpdate()
{
	EntityAnimal::onLivingUpdate();
	field_756_e = field_752_b;
	field_757_d = destPos;
	destPos += (float)(onGround ? -1 : 4) * 0.3f;
	if (destPos < 0.0f) { destPos = 0.0f; }
	if (destPos > 1.0f) { destPos = 1.0f; }
	if (!onGround && field_755_h < 1.0f)
	{
		field_755_h = 1.0f;
	}
	field_755_h *= 0.9f;
	if (!onGround && motionY < 0.0)
	{
		motionY *= 0.6;
	}
	field_752_b += field_755_h * 2.0f;
	if (!worldObj->multiplayerWorld && --timeUntilNextEgg <= 0)
	{
		worldObj->playSoundAtEntity(this, u"mob.chickenplop", 1.0f, (rand.nextFloat() - rand.nextFloat()) * 0.2f + 1.0f);
		dropItem(Item::egg->shiftedIndex, 1);
		timeUntilNextEgg = rand.nextInt(6000) + 6000;
	}
}

jstring EntityChicken::getLivingSound() { return u"mob.chicken"; }
jstring EntityChicken::getHurtSound()   { return u"mob.chickenhurt"; }
jstring EntityChicken::getDeathSound()  { return u"mob.chickenhurt"; }
int_t   EntityChicken::getDropItemId()  { return Item::feather->shiftedIndex; }
