#pragma once

#include <string>
#include "IInventory.h"

class ItemStack;

class InventoryLargeChest : public IInventory {
public:
    InventoryLargeChest(std::string name, IInventory* upperChest, IInventory* lowerChest);
    
    int getSizeInventory() override;
    std::string getInvName() override;
    ItemStack* getStackInSlot(int slot) override;
    ItemStack* decrStackSize(int slot, int amount) override;
    void setInventorySlotContents(int slot, ItemStack* stack) override;
    int getInventoryStackLimit() override;
    void onInventoryChanged() override;
    bool canInteractWith(EntityPlayer* player) override;

private:
    std::string name;
    IInventory* upperChest;
    IInventory* lowerChest;
};