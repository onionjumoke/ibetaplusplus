#include "SaveFormatOld.h"

#include "SaveFormatComparator.h"
#include "SaveHandler.h"
#include "CompressedStreamTools.h"
#include "NBTTagCompound.h"
#include "WorldInfo.h"
#include "java/File.h"
#include "java/String.h"

// Java: protected static void func_22179_a(File afile[])
static void func_22179_a(const std::vector<std::unique_ptr<File>> &afile)
{
    for (const auto &f : afile)
    {
        if (f->isDirectory())
            func_22179_a(f->listFiles());
        f->remove();
    }
}

SaveFormatOld::SaveFormatOld(const std::string &baseDir)
    : field_22180_a(File::open(String::fromUTF8(baseDir)))
    , baseDirectory(baseDir)
{
    if (!field_22180_a->exists())
        field_22180_a->mkdirs();
}

jstring SaveFormatOld::func_22178_a()
{
    return String::fromUTF8("Old Format");
}

// Java: public List func_22176_b()
std::vector<SaveFormatComparator *> SaveFormatOld::getSaveList()
{
    std::vector<SaveFormatComparator *> list;
    for (int i = 0; i < 5; i++)
    {
        std::string s = "World" + std::to_string(i + 1);
        WorldInfo *info = getWorldInfoUTF8(s);
        if (info)
            list.push_back(new SaveFormatComparator(s, "", info->getLastTimePlayed(), info->getSizeOnDisk(), false));
    }
    return list;
}

// Java: public void flushCache()
void SaveFormatOld::flushCache()
{
}

WorldInfo* SaveFormatOld::getWorldInfoUTF8(const std::string &s)
{
    File *file = File::open(*field_22180_a, String::fromUTF8(s));
    if (!file->exists())
        return nullptr;

    File *file1 = File::open(*file, u"level.dat");
    if (file1->exists())
    {
        try
        {
            std::istream *stream = file1->toStreamIn();
            NBTTagCompound *root = CompressedStreamTools::readGzippedCompound(*stream);
            delete stream;
            return new WorldInfo(root->getCompoundTag(u"Data"));
        }
        catch (...) {}
    }

    file1 = File::open(*file, u"level.dat_old");
    if (file1->exists())
    {
        try
        {
            std::istream *stream = file1->toStreamIn();
            NBTTagCompound *root = CompressedStreamTools::readGzippedCompound(*stream);
            delete stream;
            return new WorldInfo(root->getCompoundTag(u"Data"));
        }
        catch (...) {}
    }
    return nullptr;
}

WorldInfo* SaveFormatOld::getWorldInfo(const jstring &s)
{
    return getWorldInfoUTF8(String::toUTF8(s));
}

// Java: public void func_22170_a(String s, String s1)
void SaveFormatOld::renameWorld(const jstring &s, const jstring &s1)
{
    File *file = File::open(*field_22180_a, s);
    if (!file->exists())
        return;

    File *file1 = File::open(*file, u"level.dat");
    if (!file1->exists())
        return;

    try
    {
        std::istream *fin = file1->toStreamIn();
        NBTTagCompound *root = CompressedStreamTools::readGzippedCompound(*fin);
        delete fin;

        root->getCompoundTag(u"Data")->setString(u"LevelName", s1);

        std::ostream *fout = file1->toStreamOut();
        CompressedStreamTools::writeGzippedCompoundToOutputStream(root, *fout);
        delete fout;
    }
    catch (...) {}
}

// Java: public void func_22172_c(String s)
void SaveFormatOld::deleteWorldDirectory(const jstring &s)
{
    File *file = File::open(*field_22180_a, s);
    if (!file->exists())
        return;
    func_22179_a(file->listFiles());
    file->remove();
}

ISaveHandler* SaveFormatOld::getSaveLoader(const jstring &s, bool flag)
{
    return new SaveHandler(baseDirectory, String::toUTF8(s), flag);
}

bool SaveFormatOld::isOldMapFormat(const jstring &s)
{
    return false;
}

bool SaveFormatOld::convertMapFormat(const jstring &s, IProgressUpdate *iprogressupdate)
{
    return false;
}
