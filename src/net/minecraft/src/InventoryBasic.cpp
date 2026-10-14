#include "InventoryBasic.h"
#include "ItemStack.h"

InventoryBasic::InventoryBasic(std::string name, int slotsCount) {
    this->inventoryTitle = name;
    this->slotsCount = slotsCount;
    this->inventoryContents = new ItemStack*[slotsCount];
    for (int i = 0; i < slotsCount; i++) {
        this->inventoryContents[i] = nullptr;
    }
}

ItemStack* InventoryBasic::getStackInSlot(int slot) {
    return inventoryContents[slot];
}

ItemStack* InventoryBasic::decrStackSize(int slot, int amount) {
    if (inventoryContents[slot] != nullptr) {
        if (inventoryContents[slot]->stackSize <= amount) {
            ItemStack* stack = inventoryContents[slot];
            inventoryContents[slot] = nullptr;
            onInventoryChanged();
            return stack;
        }
        ItemStack* split = inventoryContents[slot]->splitStack(amount);
        if (inventoryContents[slot]->stackSize == 0) {
            inventoryContents[slot] = nullptr;
        }
        onInventoryChanged();
        return split;
    }
    return nullptr;
}

void InventoryBasic::setInventorySlotContents(int slot, ItemStack* stack) {
    inventoryContents[slot] = stack;
    if (stack != nullptr && stack->stackSize > getInventoryStackLimit()) {
        stack->stackSize = getInventoryStackLimit();
    }
    onInventoryChanged();
}

int InventoryBasic::getSizeInventory() {
    return slotsCount;
}

std::string InventoryBasic::getInvName() {
    return inventoryTitle;
}

int InventoryBasic::getInventoryStackLimit() {
    return 64;
}

void InventoryBasic::onInventoryChanged() {
    for (size_t i = 0; i < changeCallbacks.size(); i++) {
        changeCallbacks[i]->onInventoryChanged(this);
    }
}

bool InventoryBasic::canInteractWith(EntityPlayer* player) {
    return true;
}

void InventoryBasic::addInventoryChangeCallback(IInvBasic* callback) {
    changeCallbacks.push_back(callback);
}