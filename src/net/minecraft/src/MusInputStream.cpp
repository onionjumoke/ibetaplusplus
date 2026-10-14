#include "MusInputStream.h"

#include <algorithm>
#include <iostream>

MusInputStream::MusInputStream(CodecMus *codecmus, const std::string &urlPath, std::istream *inputstream)
    : std::istream(&streamBuf),
      codec(codecmus),
      hash(0),
      inputStream(inputstream),
      streamBuf(this)
{
    // Derive hash from filename component of the URL path (like Java's url.getPath())
    std::string s = urlPath;
    size_t slash = s.rfind('/');
    if (slash != std::string::npos)
        s = s.substr(slash + 1);

    // Java: hash = s.hashCode()
    int h = 0;
    for (unsigned char c : s)
        h = 31 * h + c;
    hash = h;
}

int MusInputStream::read()
{
    std::vector<byte_t> tmp(1);
    int n = read(tmp, 0, 1);
    if (n < 0)
        return n;
    return (unsigned char)tmp[0];
}

int MusInputStream::read(std::vector<byte_t> &abyte0, int_t i, int_t j)
{
    // Read j bytes from the underlying stream
    inputStream->read(reinterpret_cast<char *>(abyte0.data() + i), j);
    j = (int_t)inputStream->gcount();

    if (j <= 0)
        return -1;

    // XOR each byte with the rolling hash key
    for (int_t k = 0; k < j; k++)
    {
        byte_t &b = abyte0[i + k];
        b ^= (byte_t)(hash >> 8);
        // Java: hash = hash * 0x1dba038f + 0x14ee3 * b
        hash = hash * 0x1dba038f + 0x14ee3 * (int_t)(unsigned char)b;
    }

    return j;
}

std::streambuf::int_type MusInputStream::MusStreamBuf::underflow()
{
    int c = owner_->read();
    if (c < 0)
        return traits_type::eof();
    buf_[0] = (char)c;
    setg(buf_, buf_, buf_ + 1);
    return traits_type::to_int_type(buf_[0]);
}
