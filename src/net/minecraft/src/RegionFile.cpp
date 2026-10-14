#include "RegionFile.h"

#include <cstring>
#include <ctime>
#include <stdexcept>
#include <cstdio>
#include <zlib.h>

const byte_t RegionFile::emptySector[4096] = {};

static void writeInt32BE(std::fstream &f, int_t v)
{
    byte_t b[4];
    b[0] = (byte_t)(v >> 24);
    b[1] = (byte_t)(v >> 16);
    b[2] = (byte_t)(v >> 8);
    b[3] = (byte_t)(v);
    f.write((char*)b, 4);
}

static int_t readInt32BE(std::fstream &f)
{
    byte_t b[4];
    f.read((char*)b, 4);
    return ((int_t)(unsigned char)b[0] << 24) | ((int_t)(unsigned char)b[1] << 16)
         | ((int_t)(unsigned char)b[2] << 8)  | ((int_t)(unsigned char)b[3]);
}

static void writeByte(std::fstream &f, byte_t b)
{
    f.write((char*)&b, 1);
}

static byte_t readByte(std::fstream &f)
{
    byte_t b;
    f.read((char*)&b, 1);
    return b;
}

static long_t fileLength(std::fstream &f)
{
    auto pos = f.tellg();
    f.seekg(0, std::ios::end);
    long_t len = f.tellg();
    f.seekg(pos);
    return len;
}

RegionFile::RegionFile(const std::string &path) :
    filePath(path),
    sizeDelta(0)
{
    memset(offsets, 0, sizeof(offsets));
    memset(timestamps, 0, sizeof(timestamps));

    try
    {
        dataFile.open(path, std::ios::in | std::ios::out | std::ios::binary);
        if (!dataFile.is_open())
        {
            dataFile.open(path, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
        }

        if (fileLength(dataFile) < 4096)
        {
            // Write two empty sector tables (header)
            for (int i = 0; i < 1024; i++)
                writeInt32BE(dataFile, 0);
            for (int i = 0; i < 1024; i++)
                writeInt32BE(dataFile, 0);
            sizeDelta += 8192;
        }

        // Pad to 4096-byte boundary
        long_t len = fileLength(dataFile);
        if ((len & 4095) != 0)
        {
            dataFile.seekp(0, std::ios::end);
            int pad = (int)(4096 - (len & 4095));
            for (int i = 0; i < pad; i++)
                writeByte(dataFile, 0);
        }

        int sectorCount = (int)(fileLength(dataFile) / 4096);
        sectorFree.assign(sectorCount, true);
        sectorFree[0] = false; // offset table sector
        sectorFree[1] = false; // timestamp table sector

        // Read offset table
        dataFile.seekg(0);
        for (int i = 0; i < 1024; i++)
        {
            int_t v = readInt32BE(dataFile);
            offsets[i] = v;
            if (v != 0)
            {
                int start  = v >> 8;
                int length = v & 0xff;
                if (start + length <= (int)sectorFree.size())
                {
                    for (int s = 0; s < length; s++)
                        sectorFree[start + s] = false;
                }
            }
        }

        // Read timestamp table
        for (int i = 0; i < 1024; i++)
            timestamps[i] = readInt32BE(dataFile);
    }
    catch (...)
    {
        // Silently continue (matching Java behavior)
    }
}

RegionFile::~RegionFile()
{
    close();
}

int_t RegionFile::getSizeDelta()
{
    std::lock_guard<std::mutex> lock(mtx);
    int_t d = sizeDelta;
    sizeDelta = 0;
    return d;
}

bool RegionFile::outOfBounds(int_t x, int_t z)
{
    return x < 0 || x >= 32 || z < 0 || z >= 32;
}

int_t RegionFile::getOffset(int_t x, int_t z)
{
    return offsets[x + z * 32];
}

void RegionFile::setOffset(int_t x, int_t z, int_t value)
{
    offsets[x + z * 32] = value;
    dataFile.seekp((x + z * 32) * 4);
    writeInt32BE(dataFile, value);
}

void RegionFile::setTimestamp(int_t x, int_t z, int_t value)
{
    timestamps[x + z * 32] = value;
    dataFile.seekp(4096 + (x + z * 32) * 4);
    writeInt32BE(dataFile, value);
}

bool RegionFile::hasChunk(int_t x, int_t z)
{
    std::lock_guard<std::mutex> lock(mtx);
    if (outOfBounds(x, z)) return false;
    return getOffset(x, z) != 0;
}

// Returns raw (inflated) chunk bytes, or empty on failure
std::vector<byte_t> RegionFile::getChunkData(int_t x, int_t z)
{
    std::lock_guard<std::mutex> lock(mtx);

    if (outOfBounds(x, z)) return {};

    int_t offset = getOffset(x, z);
    if (offset == 0) return {};

    int_t sectorStart  = offset >> 8;
    int_t sectorLength = offset & 0xff;

    if (sectorStart + sectorLength > (int_t)sectorFree.size()) return {};

    try
    {
        dataFile.seekg(sectorStart * 4096);
        int_t dataLength = readInt32BE(dataFile);
        if (dataLength > 4096 * sectorLength) return {};

        byte_t version = readByte(dataFile);

        std::vector<byte_t> compressed(dataLength - 1);
        dataFile.read((char*)compressed.data(), dataLength - 1);

        if (version == 1)
        {
            // GZIP — use zlib with gzip header
            uLongf destLen = 1024 * 1024;
            std::vector<byte_t> out(destLen);

            z_stream zs{};
            inflateInit2(&zs, 16 + MAX_WBITS); // gzip mode
            zs.next_in  = (Bytef*)compressed.data();
            zs.avail_in = (uInt)compressed.size();
            zs.next_out  = reinterpret_cast<Bytef *>(out.data());
            zs.avail_out = (uInt)out.size();
            inflate(&zs, Z_FINISH);
            uLong actual = zs.total_out;
            inflateEnd(&zs);
            out.resize(actual);
            return out;
        }
        else if (version == 2)
        {
            // Deflate (zlib)
            uLongf destLen = 1024 * 1024;
            std::vector<byte_t> out(destLen);
            uncompress(reinterpret_cast<Bytef *>(out.data()), &destLen,
                       reinterpret_cast<const Bytef *>(compressed.data()), (uLong)compressed.size());
            out.resize((size_t)destLen);
            return out;
        }
    }
    catch (...) {}
    return {};
}

void RegionFile::write(int_t x, int_t z, const byte_t *rawData, int_t rawLength)
{
    std::lock_guard<std::mutex> lock(mtx);

    try
    {
        // Compress with zlib deflate
        uLongf compLen = compressBound((uLong)rawLength);
        std::vector<byte_t> compressed(compLen);
        compress2(reinterpret_cast<Bytef *>(compressed.data()), &compLen,
                  reinterpret_cast<const Bytef *>(rawData), (uLong)rawLength, Z_DEFAULT_COMPRESSION);
        compressed.resize((size_t)compLen);

        int_t sectorNeeded = ((int_t)compLen + 5) / 4096 + 1;
        if (sectorNeeded >= 256) return;

        int_t existingOffset = getOffset(x, z);
        int_t existingSector = existingOffset >> 8;
        int_t existingLength = existingOffset & 0xff;

        if (existingSector != 0 && existingLength == sectorNeeded)
        {
            // Overwrite in place
            writeSector(existingSector, compressed.data(), (int_t)compressed.size());
        }
        else
        {
            // Free old sectors
            for (int i = 0; i < existingLength; i++)
            {
                int_t s = existingSector + i;
                if (s < (int_t)sectorFree.size())
                    sectorFree[s] = true;
            }

            // Find free run of sectorNeeded sectors
            int_t runStart = -1, runLen = 0;
            for (int i = 0; i < (int_t)sectorFree.size(); i++)
            {
                if (sectorFree[i])
                {
                    if (runLen == 0) runStart = i;
                    runLen++;
                    if (runLen >= sectorNeeded) break;
                }
                else
                {
                    runLen = 0;
                    runStart = -1;
                }
            }

            if (runLen >= sectorNeeded)
            {
                // Reuse existing free sectors
                for (int i = 0; i < sectorNeeded; i++)
                    sectorFree[runStart + i] = false;
                setOffset(x, z, (runStart << 8) | sectorNeeded);
                writeSector(runStart, compressed.data(), (int_t)compressed.size());
            }
            else
            {
                // Grow file
                dataFile.seekp(0, std::ios::end);
                int_t k1 = (int_t)sectorFree.size();
                for (int i = 0; i < sectorNeeded; i++)
                {
                    dataFile.write((char*)emptySector, 4096);
                    sectorFree.push_back(false);
                }
                sizeDelta += 4096 * sectorNeeded;
                setOffset(x, z, (k1 << 8) | sectorNeeded);
                writeSector(k1, compressed.data(), (int_t)compressed.size());
            }
        }

        setTimestamp(x, z, (int_t)(time(nullptr)));
    }
    catch (...) {}
}

void RegionFile::writeSector(int_t sectorNum, const byte_t *data, int_t length)
{
    dataFile.seekp(sectorNum * 4096);
    writeInt32BE(dataFile, length + 1);
    writeByte(dataFile, 2); // version 2 = zlib deflate
    dataFile.write((char*)data, length);
}

void RegionFile::close()
{
    if (dataFile.is_open())
        dataFile.close();
}
