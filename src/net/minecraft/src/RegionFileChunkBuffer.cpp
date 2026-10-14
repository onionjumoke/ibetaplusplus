#include "RegionFileChunkBuffer.h"

#include "RegionFile.h"

RegionFileChunkBuffer::RegionFileChunkBuffer(RegionFile *regionfile, int_t x, int_t z) :
    regionFile(regionfile),
    chunkX(x),
    chunkZ(z)
{
    buf.reserve(8096);
}

void RegionFileChunkBuffer::write(byte_t b)
{
    buf.push_back(b);
}

void RegionFileChunkBuffer::write(const byte_t *data, int_t len)
{
    buf.insert(buf.end(), data, data + len);
}

void RegionFileChunkBuffer::close()
{
    regionFile->write(chunkX, chunkZ, buf.data(), (int_t)buf.size());
}
