#include "CompressedStreamTools.h"

#include "NBTBase.h"
#include "NBTTagCompound.h"
#include <sstream>
#include <vector>
#include <stdexcept>
#include <zlib.h>

static std::vector<char> gzipDecompress(const std::vector<char> &compressed)
{
    // Streaming inflate with a growable output buffer. The previous version
    // allocated a fixed 1 MB buffer and called inflate() once with Z_FINISH:
    // any stream that decompressed to more than 1 MB returned Z_BUF_ERROR, the
    // exception was swallowed by SaveHandler::loadWorldInfo(), and the world
    // was then treated as brand new with seed 0 (GuiSelectWorld passes 0LL).
    std::vector<char> out;
    out.resize(64 * 1024);

    z_stream zs{};
    if (inflateInit2(&zs, 16 + MAX_WBITS) != Z_OK) // gzip mode
        throw std::runtime_error("gzip decompress init failed");

    zs.next_in  = (Bytef*)compressed.data();
    zs.avail_in = (uInt)compressed.size();

    int ret = Z_OK;
    while (ret != Z_STREAM_END)
    {
        if ((size_t)zs.total_out == out.size())
            out.resize(out.size() * 2);

        zs.next_out  = (Bytef*)out.data() + zs.total_out;
        zs.avail_out = (uInt)(out.size() - (size_t)zs.total_out);

        ret = inflate(&zs, Z_NO_FLUSH);

        if (ret != Z_OK && ret != Z_STREAM_END)
        {
            inflateEnd(&zs);
            throw std::runtime_error("gzip decompress failed");
        }

        if (ret == Z_OK && zs.avail_in == 0 && zs.avail_out > 0)
        {
            // All input consumed without an end-of-stream marker: truncated.
            inflateEnd(&zs);
            throw std::runtime_error("gzip decompress truncated");
        }
    }

    uLong actual = zs.total_out;
    inflateEnd(&zs);
    out.resize(actual);
    return out;
}

static std::vector<char> gzipCompress(const std::vector<char> &raw)
{
    uLongf compLen = compressBound((uLong)raw.size()) + 64;
    std::vector<char> out(compLen);

    z_stream zs{};
    deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 16 + MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
    zs.next_in  = (Bytef*)raw.data();
    zs.avail_in = (uInt)raw.size();
    zs.next_out  = (Bytef*)out.data();
    zs.avail_out = (uInt)out.size();

    deflate(&zs, Z_FINISH);
    uLong actual = zs.total_out;
    deflateEnd(&zs);

    out.resize(actual);
    return out;
}

NBTTagCompound* CompressedStreamTools::readGzippedCompound(std::istream &is)
{
    // Read all bytes from stream
    std::vector<char> compressed((std::istreambuf_iterator<char>(is)), {});
    std::vector<char> raw = gzipDecompress(compressed);

    std::string s(raw.begin(), raw.end());
    std::istringstream rawStream(s, std::ios::binary);
    return readCompound(rawStream);
}

NBTTagCompound* CompressedStreamTools::readCompressed(std::istream &is)
{
    return readGzippedCompound(is);
}

void CompressedStreamTools::writeGzippedCompoundToOutputStream(NBTTagCompound *compound, std::ostream &os)
{
    std::ostringstream rawStream(std::ios::binary);
    writeCompound(compound, rawStream);

    std::string raw = rawStream.str();
    std::vector<char> rawVec(raw.begin(), raw.end());
    std::vector<char> compressed = gzipCompress(rawVec);

    os.write(compressed.data(), (std::streamsize)compressed.size());
}

NBTTagCompound* CompressedStreamTools::readCompound(std::istream &is)
{
    NBTBase *tag = NBTBase::readTag(is);
    NBTTagCompound *compound = dynamic_cast<NBTTagCompound*>(tag);
    if (compound == nullptr)
        throw std::runtime_error("Root tag must be a named compound tag");
    return compound;
}

void CompressedStreamTools::writeCompound(NBTTagCompound *compound, std::ostream &os)
{
    NBTBase::writeTag(compound, os);
}
