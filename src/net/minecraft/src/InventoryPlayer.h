#pragma once

#include "IInventory.h"

class ItemStack;
class EntityPlayer;
class Block;
class Entity;
class NBTTagList;

class InventoryPlayer : public IInventory {
public:
    ItemStack** mainInventory;
    ItemStack** armorInventory;
    int currentItem;
    bool inventoryChanged;
    EntityPlayer* player;
    ItemStack* itemStack;

    InventoryPlayer(EntityPlayer* player);
    ~InventoryPlayer() override;
    
    ItemStack* getCurrentItem();
    int getInventorySlotContainItem(int itemID);
    int storeItemStack(ItemStack* stack);
    int getFirstEmptyStack();
    void setCurrentItem(int itemID, bool unused);
    void changeCurrentItem(int direction);
    int storePartialItemStack(ItemStack* stack);
    void decrementAnimations();
    bool consumeInventoryItem(int itemID);
    bool addItemStackToInventory(ItemStack* stack);
    float getStrVsBlock(Block* block);
    NBTTagList* writeToNBT(NBTTagList* tagList);
    void readFromNBT(NBTTagList* tagList);
    int getTotalArmorValue();
    void damageArmor(int amount);
    void dropAllItems();
    ItemStack* armorItemInSlot(int slot);
    int getDamageVsEntity(Entity* entity);
    bool canHarvestBlock(Block* block);
    void setItemStack(ItemStack* stack);
    ItemStack* getItemStack();
    bool doesXZShowFog(ItemStack* stack);
    
    // IInventory implementation
    int getSizeInventory() override;
    ItemStack* getStackInSlot(int slot) override;
    ItemStack* decrStackSize(int slot, int amount) override;
    void setInventorySlotContents(int slot, ItemStack* stack) override;
    std::string getInvName() override;
    int getInventoryStackLimit() override;
    void onInventoryChanged() override;
    bool canInteractWith(EntityPlayer* player) override;
};
