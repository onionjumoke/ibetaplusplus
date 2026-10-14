#include "InventoryCraftResult.h"
#include "ItemStack.h"

InventoryCraftResult::InventoryCraftResult() {
    stackResult = new ItemStack*[1];
    stackResult[0] = nullptr;
}

InventoryCraftResult::~InventoryCraftResult() {
    delete stackResult[0];
    delete[] stackResult;
}

int InventoryCraftResult::getSizeInventory() {
    return 1;
}

ItemStack* InventoryCraftResult::getStackInSlot(int slot) {
    return stackResult[slot];
}

std::string InventoryCraftResult::getInvName() {
    return "Result";
}

ItemStack* InventoryCraftResult::decrStackSize(int slot, int amount) {
    if (stackResult[slot] != nullptr) {
        ItemStack* stack = stackResult[slot];
        stackResult[slot] = nullptr;
        return stack;
    }
    return nullptr;
}

void InventoryCraftResult::setInventorySlotContents(int slot, ItemStack* stack) {
    stackResult[slot] = stack;
}

int InventoryCraftResult::getInventoryStackLimit() {
    return 64;
}

void InventoryCraftResult::onInventoryChanged() {
}

bool InventoryCraftResult::canInteractWith(EntityPlayer* player) {
    return true;
}
