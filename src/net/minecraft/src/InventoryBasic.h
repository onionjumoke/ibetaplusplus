#pragma once

#include <string>
#include <vector>
#include "IInventory.h"
#include "IInvBasic.h"

class ItemStack;

class InventoryBasic : public IInventory {
public:
    InventoryBasic(std::string name, int slotsCount);
    
    ItemStack* getStackInSlot(int slot) override;
    ItemStack* decrStackSize(int slot, int amount) override;
    void setInventorySlotContents(int slot, ItemStack* stack) override;
    int getSizeInventory() override;
    std::string getInvName() override;
    int getInventoryStackLimit() override;
    void onInventoryChanged() override;
    bool canInteractWith(EntityPlayer* player) override;
    
    void addInventoryChangeCallback(IInvBasic* callback);

private:
    std::string inventoryTitle;
    int slotsCount;
    ItemStack** inventoryContents;
    std::vector<IInvBasic*> changeCallbacks;
};