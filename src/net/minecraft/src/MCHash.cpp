#include "MCHash.h"

MCHash::MCHash()
    : slots(16, nullptr), count(0), threshold(12), growFactor(0.75f), versionStamp(0)
{
}

MCHash::~MCHash()
{
    clearMap();
}

int_t MCHash::computeHash(int_t i)
{
    i ^= (int_t)((uint_t)i >> 20) ^ (int_t)((uint_t)i >> 12);
    return i ^ (int_t)((uint_t)i >> 7) ^ (int_t)((uint_t)i >> 4);
}

int_t MCHash::getSlotIndex(int_t i, int_t j)
{
    return i & (j - 1);
}

int_t MCHash::getHash(int_t i)
{
    return computeHash(i);
}

void *MCHash::lookup(int_t i)
{
    int_t j = computeHash(i);
    for (MCHashEntry *e = slots[getSlotIndex(j, (int_t)slots.size())]; e; e = e->nextEntry)
        if (e->hashEntry == i)
            return e->valueEntry;
    return nullptr;
}

void MCHash::addKey(int_t i, void *obj)
{
    int_t j = computeHash(i);
    int_t k = getSlotIndex(j, (int_t)slots.size());
    for (MCHashEntry *e = slots[k]; e; e = e->nextEntry)
        if (e->hashEntry == i)
        {
            e->valueEntry = obj;
            return;
        }
    versionStamp++;
    insert(j, i, obj, k);
}

void *MCHash::removeObject(int_t i)
{
    MCHashEntry *e = removeEntry(i);
    if (e == nullptr)
        return nullptr;
    void *value = e->valueEntry;
    delete e;
    return value;
}

MCHashEntry *MCHash::removeEntry(int_t i)
{
    int_t j = computeHash(i);
    int_t k = getSlotIndex(j, (int_t)slots.size());
    MCHashEntry *prev = slots[k];
    MCHashEntry *cur;
    MCHashEntry *next;
    for (cur = prev; cur; cur = next)
    {
        next = cur->nextEntry;
        if (cur->hashEntry == i)
        {
            versionStamp++;
            count--;
            if (prev == cur)
                slots[k] = next;
            else
                prev->nextEntry = next;
            return cur;
        }
        prev = cur;
    }
    return nullptr;
}

void MCHash::clearMap()
{
    versionStamp++;
    for (auto &slot : slots)
    {
        MCHashEntry *entry = slot;
        while (entry != nullptr)
        {
            MCHashEntry *next = entry->nextEntry;
            delete entry;
            entry = next;
        }
        slot = nullptr;
    }
    count = 0;
}

void MCHash::grow(int_t i)
{
    int_t j = (int_t)slots.size();
    if (j == 0x40000000)
    {
        threshold = 0x7fffffff;
        return;
    }
    std::vector<MCHashEntry *> newSlots(i, nullptr);
    copyTo(newSlots);
    slots = std::move(newSlots);
    threshold = (int_t)((float)i * growFactor);
}

void MCHash::copyTo(std::vector<MCHashEntry *> &dest)
{
    int_t i = (int_t)dest.size();
    for (int_t j = 0; j < (int_t)slots.size(); j++)
    {
        MCHashEntry *e = slots[j];
        if (!e) continue;
        slots[j] = nullptr;
        do
        {
            MCHashEntry *next = e->nextEntry;
            int_t k = getSlotIndex(e->slotHash, i);
            e->nextEntry = dest[k];
            dest[k] = e;
            e = next;
        } while (e);
    }
}

void MCHash::insert(int_t slotHash, int_t key, void *obj, int_t k)
{
    slots[k] = new MCHashEntry(slotHash, key, obj, slots[k]);
    if (count++ >= threshold)
        grow(2 * (int_t)slots.size());
}
