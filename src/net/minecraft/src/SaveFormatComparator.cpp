#include "SaveFormatComparator.h"

SaveFormatComparator::SaveFormatComparator(const std::string &fileName, const std::string &displayName,
                                           long_t lastPlayed, long_t sizeOnDisk, bool needsConversion) :
    fileName(fileName),
    displayName(displayName),
    lastTimePlayed(lastPlayed),
    sizeOnDisk_(sizeOnDisk),
    conversionRequired(needsConversion)
{
}

const std::string& SaveFormatComparator::getFileName() const { return fileName; }
const std::string& SaveFormatComparator::getDisplayName() const { return displayName; }
long_t SaveFormatComparator::getSizeOnDisk() const { return sizeOnDisk_; }
bool SaveFormatComparator::requiresConversion() const { return conversionRequired; }
long_t SaveFormatComparator::getLastTimePlayed() const { return lastTimePlayed; }

int SaveFormatComparator::compareTo(const SaveFormatComparator &other) const
{
    if (lastTimePlayed < other.lastTimePlayed) return 1;
    if (lastTimePlayed > other.lastTimePlayed) return -1;
    return fileName.compare(other.fileName);
}

bool SaveFormatComparator::operator<(const SaveFormatComparator &other) const
{
    return compareTo(other) < 0;
}
