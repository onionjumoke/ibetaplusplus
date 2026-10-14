#pragma once

#include <string>

class Item;
class Block;
class NBTTagCompound;
class World;
class EntityPlayer;
class EntityLiving;
class Entity;

class ItemStack {
public:
    ItemStack(Block* block);
    ItemStack(Block* block, int count);
    ItemStack(Block* block, int count, int damage);
    ItemStack(Item* item);
    ItemStack(Item* item, int count);
    ItemStack(Item* item, int count, int damage);
    ItemStack(int id, int count, int damage);
    ItemStack(NBTTagCompound* nbt);

    ItemStack* splitStack(int amount);
    Item* getItem();
    int getIconIndex();
    bool useItem(EntityPlayer* entityplayer, World* world, int i, int j, int k, int l);
    float getStrVsBlock(Block* block);
    ItemStack* useItemRightClick(World* world, EntityPlayer* entityplayer);
    NBTTagCompound* writeToNBT(NBTTagCompound* nbt);
    void readFromNBT(NBTTagCompound* nbt);
    int getMaxStackSize();
    bool isStackable();
    bool isItemStackDamageable();
    bool getHasSubtypes();
    bool isItemDamaged();
    int getItemDamageForDisplay();
    int getItemDamage();
    void setItemDamage(int damage);
    int getMaxDamage();
    void damageItem(int damage, Entity* entity);
    void hitEntity(EntityLiving* entityliving, EntityPlayer* entityplayer);
    void onDestroyBlock(int i, int j, int k, int l, EntityPlayer* entityplayer);
    int getDamageVsEntity(Entity* entity);
    bool canHarvestBlock(Block* block);
    void func_1097_a(EntityPlayer* entityplayer);
    void onItemUse(EntityPlayer* entityplayer);
    void useItemOnEntity(EntityLiving* entityliving);
    ItemStack* copy();
    static bool areItemStacksEqual(ItemStack* a, ItemStack* b);
    bool isItemStackEqual(ItemStack* other);
    bool isItemEqual(ItemStack* other);
    std::string getItemName();
    static ItemStack* copyItemStack(ItemStack* itemstack);
    std::string toString();
    void updateAnimation(World* world, Entity* entity, int i, bool flag);
    void onCrafting(World* world, EntityPlayer* entityplayer);
    bool isStackEqual(ItemStack* other);

    int stackSize;
    // Java defaults this to 0. The ItemStack(NBTTagCompound*) ctor doesn't set it, so items LOADED
    // from a save had garbage here; a large positive value makes GuiIngame::renderInventorySlot scale
    // the icon to ~0 width (glScalef(1/f2,...)) → the hotbar icon vanishes while its count still shows.
    int animationsToGo = 0;
    int itemID;

private:
    int itemDamage;
};
