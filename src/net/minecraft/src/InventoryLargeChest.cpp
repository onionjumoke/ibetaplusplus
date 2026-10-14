#include "InventoryLargeChest.h"
#include "ItemStack.h"

InventoryLargeChest::InventoryLargeChest(std::string name, IInventory* upperChest, IInventory* lowerChest) {
    this->name = name;
    this->upperChest = upperChest;
    this->lowerChest = lowerChest;
}

int InventoryLargeChest::getSizeInventory() {
    return upperChest->getSizeInventory() + lowerChest->getSizeInventory();
}

std::string InventoryLargeChest::getInvName() {
    return name;
}

ItemStack* InventoryLargeChest::getStackInSlot(int slot) {
    if (slot >= upperChest->getSizeInventory()) {
        return lowerChest->getStackInSlot(slot - upperChest->getSizeInventory());
    }
    return upperChest->getStackInSlot(slot);
}

ItemStack* InventoryLargeChest::decrStackSize(int slot, int amount) {
    if (slot >= upperChest->getSizeInventory()) {
        return lowerChest->decrStackSize(slot - upperChest->getSizeInventory(), amount);
    }
    return upperChest->decrStackSize(slot, amount);
}

void InventoryLargeChest::setInventorySlotContents(int slot, ItemStack* stack) {
    if (slot >= upperChest->getSizeInventory()) {
        lowerChest->setInventorySlotContents(slot - upperChest->getSizeInventory(), stack);
    } else {
        upperChest->setInventorySlotContents(slot, stack);
    }
}

int InventoryLargeChest::getInventoryStackLimit() {
    return upperChest->getInventoryStackLimit();
}

void InventoryLargeChest::onInventoryChanged() {
    upperChest->onInventoryChanged();
    lowerChest->onInventoryChanged();
}

bool InventoryLargeChest::canInteractWith(EntityPlayer* player) {
    return upperChest->canInteractWith(player) && lowerChest->canInteractWith(player);
}