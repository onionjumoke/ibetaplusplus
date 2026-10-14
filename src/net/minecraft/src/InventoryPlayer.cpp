#include "InventoryPlayer.h"
#include "ItemStack.h"
#include "EntityPlayer.h"
#include "Block.h"
#include "Entity.h"
#include "ItemArmor.h"
#include "NBTTagList.h"
#include "NBTTagCompound.h"
#include "Material.h"

InventoryPlayer::InventoryPlayer(EntityPlayer* player) {
    mainInventory = new ItemStack*[36];
    armorInventory = new ItemStack*[4];
    for (int i = 0; i < 36; i++) {
        mainInventory[i] = nullptr;
    }
    for (int i = 0; i < 4; i++) {
        armorInventory[i] = nullptr;
    }
    currentItem = 0;
    inventoryChanged = false;
    this->player = player;
    itemStack = nullptr;
}

InventoryPlayer::~InventoryPlayer()
{
    for (int i = 0; i < 36; ++i)
        delete mainInventory[i];
    for (int i = 0; i < 4; ++i)
        delete armorInventory[i];
    delete itemStack;
    delete[] mainInventory;
    delete[] armorInventory;
}

ItemStack* InventoryPlayer::getCurrentItem() {
    if (currentItem < 9 && currentItem >= 0) {
        return mainInventory[currentItem];
    }
    return nullptr;
}

int InventoryPlayer::getInventorySlotContainItem(int itemID) {
    for (int i = 0; i < 36; i++) {
        if (mainInventory[i] != nullptr && mainInventory[i]->itemID == itemID) {
            return i;
        }
    }
    return -1;
}

int InventoryPlayer::storeItemStack(ItemStack* stack) {
    for (int i = 0; i < 36; i++) {
        if (mainInventory[i] != nullptr && 
            mainInventory[i]->itemID == stack->itemID && 
            mainInventory[i]->isStackable() && 
            mainInventory[i]->stackSize < mainInventory[i]->getMaxStackSize() && 
            mainInventory[i]->stackSize < getInventoryStackLimit() && 
            (!mainInventory[i]->getHasSubtypes() || mainInventory[i]->getItemDamage() == stack->getItemDamage())) {
            return i;
        }
    }
    return -1;
}

int InventoryPlayer::getFirstEmptyStack() {
    for (int i = 0; i < 36; i++) {
        if (mainInventory[i] == nullptr) {
            return i;
        }
    }
    return -1;
}

void InventoryPlayer::setCurrentItem(int itemID, bool unused) {
    int slot = getInventorySlotContainItem(itemID);
    if (slot >= 0 && slot < 9) {
        currentItem = slot;
    }
}

void InventoryPlayer::changeCurrentItem(int direction) {
    if (direction > 0) {
        direction = 1;
    }
    if (direction < 0) {
        direction = -1;
    }
    currentItem -= direction;
    while (currentItem < 0) {
        currentItem += 9;
    }
    while (currentItem >= 9) {
        currentItem -= 9;
    }
}

int InventoryPlayer::storePartialItemStack(ItemStack* stack) {
    int itemID = stack->itemID;
    int count = stack->stackSize;
    int slot = storeItemStack(stack);
    if (slot < 0) {
        slot = getFirstEmptyStack();
    }
    if (slot < 0) {
        return count;
    }
    if (mainInventory[slot] == nullptr) {
        mainInventory[slot] = new ItemStack(itemID, 0, stack->getItemDamage());
    }
    int transfer = count;
    if (transfer > mainInventory[slot]->getMaxStackSize() - mainInventory[slot]->stackSize) {
        transfer = mainInventory[slot]->getMaxStackSize() - mainInventory[slot]->stackSize;
    }
    if (transfer > getInventoryStackLimit() - mainInventory[slot]->stackSize) {
        transfer = getInventoryStackLimit() - mainInventory[slot]->stackSize;
    }
    if (transfer == 0) {
        return count;
    }
    count -= transfer;
    mainInventory[slot]->stackSize += transfer;
    mainInventory[slot]->animationsToGo = 5;
    return count;
}

void InventoryPlayer::decrementAnimations() {
    for (int i = 0; i < 36; i++) {
        if (mainInventory[i] != nullptr) {
            mainInventory[i]->updateAnimation(player->worldObj, player, i, currentItem == i);
        }
    }
}

bool InventoryPlayer::consumeInventoryItem(int itemID) {
    int slot = getInventorySlotContainItem(itemID);
    if (slot < 0) {
        return false;
    }
    if (--mainInventory[slot]->stackSize <= 0) {
        mainInventory[slot] = nullptr;
    }
    return true;
}

bool InventoryPlayer::addItemStackToInventory(ItemStack* stack) {
    if (!stack->isItemDamaged()) {
        int prevCount;
        do {
            prevCount = stack->stackSize;
            stack->stackSize = storePartialItemStack(stack);
        } while (stack->stackSize > 0 && stack->stackSize < prevCount);
        return stack->stackSize < prevCount;
    }
    int slot = getFirstEmptyStack();
    if (slot >= 0) {
        mainInventory[slot] = ItemStack::copyItemStack(stack);
        mainInventory[slot]->animationsToGo = 5;
        stack->stackSize = 0;
        return true;
    }
    return false;
}

int InventoryPlayer::getSizeInventory() {
    return 36 + 4;
}

ItemStack* InventoryPlayer::getStackInSlot(int slot) {
    ItemStack** inventory = mainInventory;
    if (slot >= 36) {
        slot -= 36;
        inventory = armorInventory;
    }
    return inventory[slot];
}

ItemStack* InventoryPlayer::decrStackSize(int slot, int amount) {
    ItemStack** inventory = mainInventory;
    if (slot >= 36) {
        inventory = armorInventory;
        slot -= 36;
    }
    if (inventory[slot] != nullptr) {
        if (inventory[slot]->stackSize <= amount) {
            ItemStack* stack = inventory[slot];
            inventory[slot] = nullptr;
            return stack;
        }
        ItemStack* split = inventory[slot]->splitStack(amount);
        if (inventory[slot]->stackSize == 0) {
            inventory[slot] = nullptr;
        }
        return split;
    }
    return nullptr;
}

void InventoryPlayer::setInventorySlotContents(int slot, ItemStack* stack) {
    ItemStack** inventory = mainInventory;
    if (slot >= 36) {
        slot -= 36;
        inventory = armorInventory;
    }
    inventory[slot] = stack;
}

std::string InventoryPlayer::getInvName() {
    return "Inventory";
}

int InventoryPlayer::getInventoryStackLimit() {
    return 64;
}

void InventoryPlayer::onInventoryChanged() {
}

bool InventoryPlayer::canInteractWith(EntityPlayer* player) {
    return true;
}

float InventoryPlayer::getStrVsBlock(Block* block) {
    float strength = 1.0f;
    if (mainInventory[currentItem] != nullptr) {
        strength *= mainInventory[currentItem]->getStrVsBlock(block);
    }
    return strength;
}

NBTTagList* InventoryPlayer::writeToNBT(NBTTagList* tagList) {
    for (int i = 0; i < 36; i++) {
        if (mainInventory[i] != nullptr) {
            NBTTagCompound* tag = new NBTTagCompound();
            tag->setByte("Slot", (char)i);
            mainInventory[i]->writeToNBT(tag);
            tagList->setTag(tag);
        }
    }
    for (int i = 0; i < 4; i++) {
        if (armorInventory[i] != nullptr) {
            NBTTagCompound* tag = new NBTTagCompound();
            tag->setByte("Slot", (char)(i + 100));
            armorInventory[i]->writeToNBT(tag);
            tagList->setTag(tag);
        }
    }
    return tagList;
}

void InventoryPlayer::readFromNBT(NBTTagList* tagList) {
    for (int i = 0; i < 36; i++) {
        mainInventory[i] = nullptr;
    }
    for (int i = 0; i < 4; i++) {
        armorInventory[i] = nullptr;
    }
    for (int i = 0; i < tagList->tagCount(); i++) {
        NBTTagCompound* tag = dynamic_cast<NBTTagCompound*>(tagList->tagAt(i));
        if (tag == nullptr) {
            continue;
        }
        int slot = tag->getByte("Slot") & 0xff;
        ItemStack* stack = new ItemStack(tag);
        if (stack->getItem() == nullptr) {
            continue;
        }
        if (slot >= 0 && slot < 36) {
            mainInventory[slot] = stack;
        }
        if (slot >= 100 && slot < 104) {
            armorInventory[slot - 100] = stack;
        }
    }
}

int InventoryPlayer::getDamageVsEntity(Entity* entity) {
    ItemStack* stack = getStackInSlot(currentItem);
    if (stack != nullptr) {
        return stack->getDamageVsEntity(entity);
    }
    return 1;
}

bool InventoryPlayer::canHarvestBlock(Block* block) {
    if (block->blockMaterial->getIsHarvestable()) {
        return true;
    }
    ItemStack* stack = getStackInSlot(currentItem);
    if (stack != nullptr) {
        return stack->canHarvestBlock(block);
    }
    return false;
}

ItemStack* InventoryPlayer::armorItemInSlot(int slot) {
    return armorInventory[slot];
}

int InventoryPlayer::getTotalArmorValue() {
    int total = 0;
    int durabilitySum = 0;
    int maxDurabilitySum = 0;
    for (int i = 0; i < 4; i++) {
        if (armorInventory[i] != nullptr && dynamic_cast<ItemArmor*>(armorInventory[i]->getItem()) != nullptr) {
            int maxDamage = armorInventory[i]->getMaxDamage();
            int currentDamage = armorInventory[i]->getItemDamageForDisplay();
            int remaining = maxDamage - currentDamage;
            durabilitySum += remaining;
            maxDurabilitySum += maxDamage;
            int armorValue = ((ItemArmor*)armorInventory[i]->getItem())->damageReduceAmount;
            total += armorValue;
        }
    }
    return total;
}

void InventoryPlayer::damageArmor(int amount) {
    for (int j = 0; j < 4; j++) {
        if (armorInventory[j] == nullptr || dynamic_cast<ItemArmor*>(armorInventory[j]->getItem()) == nullptr) {
            continue;
        }
        armorInventory[j]->damageItem(amount, player);
        if (armorInventory[j]->stackSize == 0) {
            armorInventory[j]->func_1097_a(player);
            armorInventory[j] = nullptr;
        }
    }
}

void InventoryPlayer::dropAllItems() {
    for (int i = 0; i < 36; i++) {
        if (mainInventory[i] != nullptr) {
            player->dropPlayerItemWithRandomChoice(mainInventory[i], true);
            mainInventory[i] = nullptr;
        }
    }
    for (int j = 0; j < 4; j++) {
        if (armorInventory[j] != nullptr) {
            player->dropPlayerItemWithRandomChoice(armorInventory[j], true);
            armorInventory[j] = nullptr;
        }
    }
}

void InventoryPlayer::setItemStack(ItemStack* stack) {
    itemStack = stack;
}

ItemStack* InventoryPlayer::getItemStack() {
    return itemStack;
}

bool InventoryPlayer::doesXZShowFog(ItemStack*) {
    return true;
}
