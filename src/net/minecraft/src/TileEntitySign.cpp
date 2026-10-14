#include "TileEntitySign.h"
#include "NBTTagCompound.h"

TileEntitySign::TileEntitySign() {
    signText[0] = "";
    signText[1] = "";
    signText[2] = "";
    signText[3] = "";
    lineBeingEdited = -1;
}

void TileEntitySign::writeToNBT(NBTTagCompound* nbttagcompound) {
    TileEntity::writeToNBT(nbttagcompound);
    nbttagcompound->setString("Text1", signText[0]);
    nbttagcompound->setString("Text2", signText[1]);
    nbttagcompound->setString("Text3", signText[2]);
    nbttagcompound->setString("Text4", signText[3]);
}

void TileEntitySign::readFromNBT(NBTTagCompound* nbttagcompound) {
    TileEntity::readFromNBT(nbttagcompound);
    signText[0] = nbttagcompound->getString("Text1");
    signText[1] = nbttagcompound->getString("Text2");
    signText[2] = nbttagcompound->getString("Text3");
    signText[3] = nbttagcompound->getString("Text4");
}
