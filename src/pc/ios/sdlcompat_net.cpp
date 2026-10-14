// sdlcompat_net.cpp — SDLNet_* over BSD sockets. Blocking TCP client only,
// exactly what java/JavaNetwork.cpp uses (multiplayer + the resource fetcher).
#include "sdlcompat/sdlnet.h"

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <errno.h>
#include <cstdio>
#include <cstring>
#include <string>

static char g_netError[256] = "";

static void setNetError(const std::string &msg)
{
	snprintf(g_netError, sizeof(g_netError), "%s: %s", msg.c_str(), strerror(errno));
}

extern "C" int SDLNet_Init(void)
{
	return 0;
}

extern "C" void SDLNet_Quit(void)
{
}

extern "C" const char *SDLNet_GetError(void)
{
	return g_netError;
}

extern "C" int SDLNet_ResolveHost(IPaddress *address, const char *host, Uint16 port)
{
	if (address == nullptr)
		return -1;
	memset(address, 0, sizeof(*address));

	// SDL_net semantics: NULL host binds INADDR_ANY.
	if (host == nullptr || host[0] == '\0')
	{
		address->host = INADDR_ANY;
		address->port = static_cast<Uint16>(((port & 0xFFu) << 8) | ((port >> 8) & 0xFFu));
		return 0;
	}

	addrinfo hints{};
	hints.ai_family = AF_INET; // iOS 5 era devices are IPv4-first
	hints.ai_socktype = SOCK_STREAM;

	addrinfo *result = nullptr;
	int rc = getaddrinfo(host, nullptr, &hints, &result);
	if (rc != 0)
	{
		snprintf(g_netError, sizeof(g_netError), "resolve %s: %s", host, gai_strerror(rc));
		return -1;
	}

	sockaddr_in *sa = reinterpret_cast<sockaddr_in *>(result->ai_addr);
	address->host = sa->sin_addr.s_addr;
	address->port = static_cast<Uint16>(((port & 0xFFu) << 8) | ((port >> 8) & 0xFFu));

	freeaddrinfo(result);
	return 0;
}

extern "C" TCPsocket SDLNet_TCP_Open(IPaddress *address)
{
	if (address == nullptr)
		return nullptr;

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
	{
		setNetError("socket");
		return nullptr;
	}

	sockaddr_in sa{};
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = address->host;
	sa.sin_port = address->port;

	(void)0;
	if (connect(fd, reinterpret_cast<sockaddr *>(&sa), sizeof(sa)) != 0)
	{
		(void)0;
		setNetError("connect");
		close(fd);
		return nullptr;
	}
	(void)0;

	// Blocking with a sane receive timeout so a dead peer cannot wedge the
	// net thread forever.
	timeval tv{30, 0};
	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	return reinterpret_cast<TCPsocket>(static_cast<intptr_t>(fd));
}

extern "C" int SDLNet_TCP_Send(TCPsocket sock, const void *data, int size)
{
	if (sock == nullptr || data == nullptr || size <= 0)
		return 0;
	int fd = static_cast<int>(reinterpret_cast<intptr_t>(sock));

	const char *p = static_cast<const char *>(data);
	int sent = 0;
	while (sent < size)
	{
		ssize_t n = send(fd, p + sent, static_cast<size_t>(size - sent), 0);
		if (n <= 0)
			return sent;
		sent += static_cast<int>(n);
	}
	return sent;
}

extern "C" int SDLNet_TCP_Recv(TCPsocket sock, void *data, int size)
{
	if (sock == nullptr || data == nullptr || size < 0)
		return -1;
	if (size == 0)
		return 0;
	int fd = static_cast<int>(reinterpret_cast<intptr_t>(sock));

	// A SO_RCVTIMEO expiry must NOT look like a clean close, or the game
	// reports "End of stream" and drops a perfectly healthy connection.
	// Retry while the socket would block; real errors/close still surface.
	for (;;)
	{
		ssize_t n = recv(fd, data, static_cast<size_t>(size), 0);
		if (n >= 0)
			return static_cast<int>(n); // >0 data, 0 clean close
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
			continue;
		return -1;
	}
}

extern "C" void SDLNet_TCP_Close(TCPsocket sock)
{
	if (sock == nullptr)
		return;
	close(static_cast<int>(reinterpret_cast<intptr_t>(sock)));
}
