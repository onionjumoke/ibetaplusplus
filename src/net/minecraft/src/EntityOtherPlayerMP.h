#pragma once

#include "EntityPlayer.h"
#include "java/Type.h"

class World;
class ItemStack;

// net.minecraft.src.EntityOtherPlayerMP
class EntityOtherPlayerMP : public EntityPlayer
{
public:
	EntityOtherPlayerMP(World *world, jstring s);

protected:
	void resetHeight() override;

public:
	bool attackEntityFrom(Entity *entity, int_t i) override;
	void setPositionAndRotation2(double d, double d1, double d2, float f, float f1, int_t i) override;
	void onUpdate() override;
	float getShadowSize() override;
	void onLivingUpdate() override;
	void outfitWithItem(int_t i, int_t j, int_t k);
	void func_6420_o();

private:
	int_t field_785_bg;
	double field_784_bh;
	double field_783_bi;
	double field_782_bj;
	double field_780_bk;
	double field_786_bl;
	float field_20924_a;
};
