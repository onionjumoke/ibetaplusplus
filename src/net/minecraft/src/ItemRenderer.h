#pragma once

class Minecraft;
class ItemStack;
class EntityLiving;
class RenderBlocks;
class MapItemRenderer;

class ItemRenderer {
public:
    ItemRenderer(Minecraft* minecraft);

    void renderItem(EntityLiving* entityliving, ItemStack* itemstack);
    void renderItemInFirstPerson(float f);
    void renderOverlays(float f);
    void updateEquippedItem();
    void resetEquippedProgress();
    void func_9449_b();
    void func_9450_c();

private:
    void renderInsideOfBlock(float f, int i);
    void renderWarpedTextureOverlay(float f);
    void renderFireInFirstPerson(float f);

    Minecraft* mc;
    ItemStack* itemToRender;
    float equippedProgress;
    float prevEquippedProgress;
    RenderBlocks* renderBlocksInstance;
    MapItemRenderer* field_28131_f;
    int field_20099_f;
};
