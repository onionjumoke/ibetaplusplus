#include "java/JavaNetwork.h"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <mutex>
#include <streambuf>

#include <SDL_net.h>

namespace JavaNetwork
{

namespace
{

// SDL_net needs a one-time init before any socket call. SDL itself is already
// initialised by main(); SDLNet_Init() only sets up the networking subsystem.
void ensureNetInit()
{
	static std::once_flag flag;
	static bool ok = false;
	std::call_once(flag, []()
	{
		ok = SDLNet_Init() == 0;
		if (!ok)
			std::cerr << "SDLNet_Init failed: " << SDLNet_GetError() << std::endl;
	});
}

// Opens a blocking client TCP connection. Returns nullptr on failure.
TCPsocket openConnection(const std::string &host, int port)
{
	ensureNetInit();
	IPaddress address;
	if (SDLNet_ResolveHost(&address, host.c_str(), static_cast<Uint16>(port)) != 0)
		return nullptr;
	return SDLNet_TCP_Open(&address);
}

class SdlNetSocket : public Socket
{
public:
	~SdlNetSocket() override
	{
		close();
	}

	bool connect(const std::string &host, int port) override
	{
		remoteSocketAddress = host + ":" + std::to_string(port);
		socket = openConnection(host, port);
		return socket != nullptr;
	}

	// Blocking read. Returns >0 with data, or <=0 when the peer closed the
	// connection or on error, which the stream adapter maps to EOF.
	int read(char *buffer, int length) override
	{
		if (socket == nullptr || length <= 0)
			return -1;
		return SDLNet_TCP_Recv(socket, buffer, length);
	}

	bool write(const char *buffer, int length) override
	{
		if (socket == nullptr)
			return false;
		if (length <= 0)
			return true;
		return SDLNet_TCP_Send(socket, buffer, length) == length;
	}

	// SDL_net sends immediately; there is no userspace buffer to drain.
	bool flush() override
	{
		return socket != nullptr;
	}

	void close() override
	{
		if (socket != nullptr)
		{
			SDLNet_TCP_Close(socket);
			socket = nullptr;
		}
	}

	std::string getRemoteSocketAddress() const override
	{
		return remoteSocketAddress;
	}

private:
	TCPsocket socket = nullptr;
	std::string remoteSocketAddress;
};

class SocketInputBuffer : public std::streambuf
{
public:
	explicit SocketInputBuffer(Socket &socket)
		: socket(socket)
	{
		setg(buffer, buffer, buffer);
	}

protected:
	int_type underflow() override
	{
		if (gptr() < egptr())
			return traits_type::to_int_type(*gptr());

		int count = socket.read(buffer, sizeof(buffer));
		if (count <= 0)
			return traits_type::eof();

		setg(buffer, buffer, buffer + count);
		return traits_type::to_int_type(*gptr());
	}

private:
	Socket &socket;
	char buffer[512];
};

class SocketOutputBuffer : public std::streambuf
{
public:
	explicit SocketOutputBuffer(Socket &socket)
		: socket(socket)
	{
	}

	~SocketOutputBuffer() override
	{
		sync();
	}

protected:
	std::streamsize xsputn(const char *s, std::streamsize n) override
	{
		return socket.write(s, static_cast<int>(n)) ? n : 0;
	}

	int_type overflow(int_type ch) override
	{
		if (traits_type::eq_int_type(ch, traits_type::eof()))
			return traits_type::not_eof(ch);

		char c = traits_type::to_char_type(ch);
		return socket.write(&c, 1) ? ch : traits_type::eof();
	}

	int sync() override
	{
		return socket.flush() ? 0 : -1;
	}

private:
	Socket &socket;
};

class SocketInputStream : public std::istream
{
public:
	explicit SocketInputStream(Socket &socket)
		: std::istream(&buffer)
		, buffer(socket)
	{
	}

private:
	SocketInputBuffer buffer;
};

class SocketOutputStream : public std::ostream
{
public:
	explicit SocketOutputStream(Socket &socket)
		: std::ostream(&buffer)
		, buffer(socket)
	{
	}

private:
	SocketOutputBuffer buffer;
};

}

std::unique_ptr<Socket> createSocket()
{
	return std::make_unique<SdlNetSocket>();
}

std::unique_ptr<std::istream> createInputStream(Socket &socket)
{
	return std::make_unique<SocketInputStream>(socket);
}

std::unique_ptr<std::ostream> createOutputStream(Socket &socket)
{
	return std::make_unique<SocketOutputStream>(socket);
}

namespace
{

// Minimal HTTP/1.0 GET over SDL_net. Returns the HTTP status code (or -1 on a
// connection/parse failure) and fills body with the response payload.
// Only plain http:// is supported — SDL_net has no TLS, so https is rejected.
int httpGet(const std::string &url, std::vector<unsigned char> &body)
{
	const std::string scheme = "http://";
	if (url.rfind(scheme, 0) != 0)
		return -1;

	std::string rest = url.substr(scheme.size());
	std::string::size_type pathStart = rest.find('/');
	std::string hostPort = pathStart == std::string::npos ? rest : rest.substr(0, pathStart);
	std::string path = pathStart == std::string::npos ? "/" : rest.substr(pathStart);

	std::string host = hostPort;
	int port = 80;
	std::string::size_type colon = hostPort.find(':');
	if (colon != std::string::npos)
	{
		host = hostPort.substr(0, colon);
		port = std::atoi(hostPort.c_str() + colon + 1);
	}

	TCPsocket socket = openConnection(host, port);
	if (socket == nullptr)
		return -1;

	std::string request =
		"GET " + path + " HTTP/1.0\r\n" +
		"Host: " + host + "\r\n" +
		"User-Agent: Minecraft\r\n" +
		"Connection: close\r\n\r\n";
	if (SDLNet_TCP_Send(socket, request.data(), (int)request.size()) != (int)request.size())
	{
		SDLNet_TCP_Close(socket);
		return -1;
	}

	std::string raw;
	char chunk[2048];
	for (;;)
	{
		int count = SDLNet_TCP_Recv(socket, chunk, sizeof(chunk));
		if (count <= 0)
			break;
		raw.append(chunk, count);
	}
	SDLNet_TCP_Close(socket);

	if (raw.empty())
		return -1;

	int status = -1;
	std::string::size_type sp = raw.find(' ');
	if (sp != std::string::npos)
		status = std::atoi(raw.c_str() + sp + 1);

	std::string::size_type headerEnd = raw.find("\r\n\r\n");
	if (headerEnd != std::string::npos)
	{
		const char *bodyStart = raw.data() + headerEnd + 4;
		std::size_t bodyLen = raw.size() - (headerEnd + 4);
		body.assign(bodyStart, bodyStart + bodyLen);
	}
	return status;
}

}

bool readUrl(const std::string &url, std::vector<unsigned char> &data)
{
	if (url.rfind("http://", 0) == 0)
	{
		int status = httpGet(url, data);
		return status >= 200 && status < 300;
	}

	std::string file = url;
	if (file.rfind("file://", 0) == 0)
		file = file.substr(7);

	if (file.find("://") != std::string::npos)
		return false;

	std::ifstream input(file, std::ios::binary);
	if (!input)
		return false;

	input.seekg(0, std::ios::end);
	std::streamoff size = input.tellg();
	input.seekg(0, std::ios::beg);
	if (size < 0)
		return false;

	data.resize((std::size_t)size);
	if (!data.empty())
		input.read(reinterpret_cast<char *>(data.data()), size);
	return true;
}

int getResponseCode(const std::string &url)
{
	if (url.rfind("http://", 0) == 0)
	{
		std::vector<unsigned char> body;
		return httpGet(url, body);
	}

	std::vector<unsigned char> data;
	return readUrl(url, data) ? 200 : -1;
}

}
