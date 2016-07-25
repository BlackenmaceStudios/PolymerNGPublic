// NetworkSystem.h
//

#include "BitMsg.h"

#pragma once

#define NET_PORT			   31150

//
// NetworkSystem
//
class NetworkSystem
{
public:
	virtual void				StartServer(int port, int maxclients) = 0;
	virtual bool				JoinServer(const char *address, int port) = 0;
	virtual bool				IsWaitingForClients() = 0;
	virtual int					GetNumConnectedPlayers() = 0;

	virtual void				SendPacket(byte *buffer, int length, bool isReliablePacket = false) = 0;
	virtual bool				GetNextPacket(BitMsg &msg) = 0;
};

extern NetworkSystem *networkSystem;