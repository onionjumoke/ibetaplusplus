#include "Packet51MapChunk.h"

#include <atomic>
#include <vector>

#include "NetHandler.h"
#include <zlib.h>

Packet51MapChunk::Packet51MapChunk() :
	xPosition(0),
	yPosition(0),
	zPosition(0),
	xSize(0),
	ySize(0),
	zSize(0),
	chunkSize(0)
{
	isChunkDataPacket = true;
}

void Packet51MapChunk::readPacketData(std::istream &is)
{
	// betamc/vanilla batched MapChunk:
	//   int x, short y, short z, <size fields>, int compressedSize, zlib...
	// The zlib header 0x78 0x9c marks the end of the header, and the 4 bytes
	// immediately before it are the big-endian compressed length. Reading
	// exactly that many bytes is essential: the old "window + keep feeding"
	// approach swallowed ~50 following packets (stream desync).
	auto be16 = [](const std::vector<unsigned char> &v, size_t o) {
		return (int)((v[o] << 8) | v[o + 1]);
	};
	auto be32 = [](const std::vector<unsigned char> &v, size_t o) {
		return (int)(((unsigned)v[o] << 24) | ((unsigned)v[o + 1] << 16) |
		             ((unsigned)v[o + 2] << 8) | (unsigned)v[o + 3]);
	};

	// 1) Read until the zlib magic (0x78 0x9c) is seen.
	std::vector<unsigned char> head;
	head.reserve(24);
	for (int k = 0; k < 24; ++k)
	{
		int b = is.get();
		if (b == EOF)
			return;
		head.push_back((unsigned char)b);
		size_t n = head.size();
		if (n >= 2 && head[n - 2] == 0x78 &&
		    (head[n - 1] == 0x9c || head[n - 1] == 0xda || head[n - 1] == 0x01))
			break;
	}
	if (head.size() < 8 || head[head.size() - 2] != 0x78)
	{
		fprintf(stderr, "Beta++: MapChunk no zlib magic (got %zu)\n", head.size());
		return;
	}
	size_t magicOff = head.size() - 2;
	// Observed batched layout: int x, short y, short reserved, short z,
	// byte xSize-1, byte ySize-1, byte zSize-1, int compressedSize, zlib.
	// (Previously z was read at offset 6, so every chunk landed at z=0 and
	// the player's area stayed empty -> constant falling.)
	xPosition = be32(head, 0);
	yPosition = be16(head, 4);
	zPosition = be16(head, 8);
	xSize = head[10] + 1;
	ySize = head[11] + 1;
	zSize = head[12] + 1;
	int csize = be32(head, magicOff - 4);

	if (csize < 2 || csize > (1 << 20))
	{
		fprintf(stderr, "Beta++: MapChunk bad csize=%d magicOff=%zu\n", csize, magicOff);
		return;
	}

	// 2) Exact-length read: 2 zlib bytes are already buffered.
	std::vector<unsigned char> comp;
	comp.assign(head.begin() + (long)magicOff, head.end());
	comp.resize((size_t)csize);
	for (size_t k = 2; k < (size_t)csize; ++k)
	{
		int b = is.get();
		if (b == EOF)
			return;
		comp[k] = (unsigned char)b;
	}

	// 3) Inflate exactly this stream.
	static std::vector<byte_t> outBuf;
	outBuf.assign(1 << 21, 0); // 2MB ceiling for a batched region
	z_stream zs{};
	inflateInit(&zs);
	zs.next_in = comp.data();
	zs.avail_in = (uInt)csize;
	zs.next_out = reinterpret_cast<Bytef *>(outBuf.data());
	zs.avail_out = (uInt)outBuf.size();
	int rc = inflate(&zs, Z_FINISH);
	size_t decomp = zs.total_out;
	inflateEnd(&zs);
	if (rc != Z_STREAM_END)
	{
		fprintf(stderr, "Beta++: MapChunk inflate rc=%d out=%zu csize=%d\n", rc, decomp, csize);
		return;
	}

	// 4) Derive dimensions: data = w*h*d*5/2 (blocks + 3 nibble arrays).
	// The header stores them as (short, byte, byte) possibly with -1 bias.
	// Validate the size fields against the decompressed length:
	// blocks + 3 nibble arrays = xSize*ySize*zSize*5/2.
	long expect = (long)xSize * ySize * zSize * 5 / 2;
	if (expect != (long)decomp)
	{
		fprintf(stderr, "Beta++: MapChunk size mismatch decomp=%zu expect=%ld hdr=%02x %02x %02x %02x %02x\n",
		        decomp, expect, head[8], head[9], head[10], head[11], head[12]);
		return;
	}
	chunk.assign(outBuf.data(), outBuf.data() + decomp);

	static std::atomic<uint32_t> ok{0};
	if (ok.fetch_add(1) < 40)
		fprintf(stderr, "Beta++: MapChunk x=%d y=%d z=%d %dx%dx%d decomp=%zu csize=%d\n",
		        xPosition, yPosition, zPosition, xSize, ySize, zSize, decomp, csize);

}

void Packet51MapChunk::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, xPosition);
	IOUtil::writeShort(os, yPosition);
	IOUtil::writeShort(os, zPosition);
	os.put(xSize - 1);
	os.put(ySize - 1);
	os.put(zSize - 1);
	IOUtil::writeInt(os, chunkSize);
	os.write(reinterpret_cast<const char*>(chunk.data()), chunkSize);
}

void Packet51MapChunk::processPacket(NetHandler &nethandler)
{
	nethandler.handleMapChunk(*this);
}

int_t Packet51MapChunk::getPacketSize()
{
	return 17 + chunkSize;
}
