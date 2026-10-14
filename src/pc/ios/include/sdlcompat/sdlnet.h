#pragma once

// Minimal SDL_net surface used by java/JavaNetwork.cpp — blocking TCP client
// over BSD sockets (see sdlcompat_net.cpp).

#include "sdlcompat/sdlcompat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IPaddress
{
	Uint32 host; // network byte order
	Uint16 port; // network byte order
} IPaddress;

typedef void *TCPsocket; // opaque socket handle; NULL on failure

int SDLNet_Init(void);
void SDLNet_Quit(void);
const char *SDLNet_GetError(void);

// Resolves hostname into address; port is in HOST byte order here
// (matches SDL_net semantics). Returns -1 on failure.
int SDLNet_ResolveHost(IPaddress *address, const char *host, Uint16 port);

TCPsocket SDLNet_TCP_Open(IPaddress *address);
int SDLNet_TCP_Send(TCPsocket sock, const void *data, int size);
int SDLNet_TCP_Recv(TCPsocket sock, void *data, int size); // 0 = clean close, <0 = error
void SDLNet_TCP_Close(TCPsocket sock);

#ifdef __cplusplus
} // extern "C"
#endif
