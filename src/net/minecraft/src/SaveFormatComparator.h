#pragma once

#include <string>
#include "java/Type.h"

// net.minecraft.src.SaveFormatComparator
class SaveFormatComparator
{
public:
    SaveFormatComparator(const std::string &fileName, const std::string &displayName,
                         long_t lastPlayed, long_t sizeOnDisk, bool needsConversion);

    const std::string& getFileName() const;
    const std::string& getDisplayName() const;
    long_t getSizeOnDisk() const;
    bool requiresConversion() const;
    long_t getLastTimePlayed() const;

    int compareTo(const SaveFormatComparator &other) const;
    bool operator<(const SaveFormatComparator &other) const;

private:
    std::string fileName;
    std::string displayName;
    long_t lastTimePlayed;
    long_t sizeOnDisk_;
    bool conversionRequired;
};
