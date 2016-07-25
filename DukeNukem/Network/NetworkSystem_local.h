// NetworkSystem_local.h
//

#pragma once

#include <enet/enet.h>

#include "NetworkSystem.h"


extern "C" void initprintf(const char *f, ...);

__forceinline void NetError(const char *f, ...)
{
	va_list va;
	char buf[2048];

	va_start(va, f);
	Bvsnprintf(buf, sizeof(buf), f, va);
	va_end(va);

	initprintf(buf);
	exit(0);
}

//
// NetworkSystem
//
class NetworkSystemLocal : public NetworkSystem
{
public:
	NetworkSystemLocal();

	virtual void				StartServer(int port, int maxclients);
	virtual bool				JoinServer(const char *address, int port);
	virtual bool				IsWaitingForClients();
	virtual int					GetNumConnectedPlayers();

	virtual void				SendPacket(byte *buffer, int length, bool isReliablePacket);
	virtual void				SendPacket(BitMsg *msg);
	virtual bool				GetNextPacket(BitMsg &msg);
private:
	ENetHost * client;
	ENetHost * server;
	ENetPeer * peer;

	int maxClients;
	int numConnectedClients;
};

extern NetworkSystemLocal networkSystemLocal;