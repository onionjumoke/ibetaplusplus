#pragma once

#include "TileEntity.h"
#include <string>

class TileEntitySign : public TileEntity {
public:
    TileEntitySign();

    void writeToNBT(NBTTagCompound* nbttagcompound) override;
    void readFromNBT(NBTTagCompound* nbttagcompound) override;

    std::string signText[4];
    int lineBeingEdited;
};
