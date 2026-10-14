#pragma once

#include "IInventory.h"

class ItemStack;

class InventoryCraftResult : public IInventory {
public:
    InventoryCraftResult();
    ~InventoryCraftResult() override;
    
    int getSizeInventory() override;
    ItemStack* getStackInSlot(int slot) override;
    std::string getInvName() override;
    ItemStack* decrStackSize(int slot, int amount) override;
    void setInventorySlotContents(int slot, ItemStack* stack) override;
    int getInventoryStackLimit() override;
    void onInventoryChanged() override;
    bool canInteractWith(EntityPlayer* player) override;

private:
    ItemStack** stackResult;
};
