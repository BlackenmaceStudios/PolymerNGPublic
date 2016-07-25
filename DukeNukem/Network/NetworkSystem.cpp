#include "pch.h"

#include "Build.h"
#include "BitMsg.h"
#include "Snapshot.h"
#include "NetworkSystem_local.h"
#include "mmulti.h"
#include "../Third-Party/zlib/zlib.h"

NetworkSystemLocal networkSystemLocal;
NetworkSystem *networkSystem = &networkSystemLocal;

NetworkSystemLocal::NetworkSystemLocal()
{
	client = NULL;
	server = NULL;
	peer = NULL;
}

void NetworkSystemLocal::StartServer(int port, int maxclients)
{
	initprintf("-------- NetworkSystem::StartServer ---------");
	initprintf("Init enet...\n");

	// Init enet library.
	if (enet_initialize() != 0)
	{
		NetError("StartServer: Failed to init enet\n");
		return;
	}

	this->maxClients = maxclients;

	initprintf("Starting Server...\n");

	// Start the server.
	ENetAddress address;
	/* Bind the server to the default localhost.     */
	/* A specific host address can be specified by   */
	/* enet_address_set_host (& address, "x.x.x.x"); */
	address.host = ENET_HOST_ANY;
	/* Bind the server to port 1234. */
	address.port = port;
	server = enet_host_create(&address /* the address to bind the server host to */,
							  maxclients      /* allow up to 32 clients and/or outgoing connections */,
							  2      /* allow up to 2 channels to be used, 0 and 1 */,
							  0      /* assume any amount of incoming bandwidth */,
							  0      /* assume any amount of outgoing bandwidth */);
	if (server == NULL)
	{
		NetError("StartServer: Failed to createserver\n");
		return;
	}

	numConnectedClients = 0;
}

bool NetworkSystemLocal::JoinServer(const char *ipaddress, int port)
{
	initprintf("-------- NetworkSystem::StartServer ---------\n");

	// Init enet library.
	initprintf("Init enet...\n");
	if (enet_initialize() != 0)
	{
		NetError("StartServer: Failed to init enet\n");
		return false;
	}

	if (client == NULL)
	{
		initprintf("Init client...\n");
		client = enet_host_create(NULL /* create a client host */,
			1 /* only allow 1 outgoing connection */,
			2 /* allow up 2 channels to be used, 0 and 1 */,
			0 /* 56K modem with 56 Kbps downstream bandwidth */,
			0 /* 56K modem with 14 Kbps upstream bandwidth */);

		if (client == NULL)
		{
			NetError("JoinServer: Failed to create client\n");
			return false;
		}
	}

	initprintf("Joining %s:%d..\n", ipaddress, port);

	/* Connect to some.server.net:1234. */
	ENetAddress address;
	enet_address_set_host(&address, ipaddress);
	address.port = port;

	peer = enet_host_connect(client, &address, 2, 0);
	if (peer == NULL)
	{
		NetError("JoinServer: Failed to create peer\n");
		return false;
	}

	ENetEvent event;
	if (!(enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT))
	{
		enet_peer_reset(peer);
		initprintf("Failed to connect to host\n");
		return false;
	}

	initprintf("Connected to host\n");

	enet_host_service(client, &event, 5000);
	//NetPackets::SendInitialConnectionHandshake();

	return true;
}

bool NetworkSystemLocal::GetNextPacket(BitMsg &msg)
{
	static ENetEvent event;
	int ret = 0;

	enet_host_service(peer->host, NULL, 0);
	
	ret = enet_host_check_events(peer->host, &event);

	if (ret > 0)
	{
		if (event.type == ENET_EVENT_TYPE_RECEIVE)
		{
			static int packet_length = 0;

			static char *packet_data = NULL;
			if (packet_data == NULL)
			{
				packet_data = new char[sizeof(Snapshot) * 2];
			}

			//initprintf("event.packet->dataLength=%d\n", event.packet->dataLength);
			//if (event.packet->dataLength == 134683)
			//{
			//	initprintf("recieved world packet\n");
			//}

			z_stream infstream;
			infstream.zalloc = Z_NULL;
			infstream.zfree = Z_NULL;
			infstream.opaque = Z_NULL;
			// setup "b" as the input and "c" as the compressed output
			infstream.avail_in = (uInt)(event.packet->dataLength); // size of input
			infstream.next_in = (Bytef *)event.packet->data; // input char array
			infstream.avail_out = (uInt)sizeof(Snapshot) * 2; // size of output
			infstream.next_out = (Bytef *)packet_data; // output char array
															  // the actual DE-compression work.
			inflateInit(&infstream);
			inflate(&infstream, Z_FINISH);
			inflateEnd(&infstream);
			

			msg.SetData((byte *)&packet_data[0], infstream.total_out);
			enet_packet_destroy(event.packet);
			return true;
		}
	}

	return false;
}

void NetworkSystemLocal::SendPacket(byte *buffer, int length, bool isReliablePacket)
{
	static byte *compressedBuffer = NULL;
	
	if (compressedBuffer == NULL)
	{
		compressedBuffer = new byte[sizeof(Snapshot) * 2];
	}

	z_stream defstream;
	defstream.zalloc = Z_NULL;
	defstream.zfree = Z_NULL;
	defstream.opaque = Z_NULL;

	defstream.avail_in = length; // size of input, string + terminator
	defstream.next_in = buffer; // input char array
	defstream.avail_out = sizeof(Snapshot) * 2; // size of output
	defstream.next_out = compressedBuffer; // output char array

	deflateInit(&defstream, Z_BEST_SPEED);
	deflate(&defstream, Z_FINISH);
	deflateEnd(&defstream);

	//enet_uint32 packetFlag = ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT;
	//if (isReliablePacket)
	//	packetFlag = ENET_PACKET_FLAG_RELIABLE;

	ENetPacket * packet = enet_packet_create(compressedBuffer, defstream.total_out, ENET_PACKET_FLAG_RELIABLE);

	enet_host_broadcast(peer->host, 1, packet);
	//enet_host_flush(peer->host);

	if (packet->referenceCount == 0)
		enet_packet_destroy(packet);
}

void NetworkSystemLocal::SendPacket(BitMsg *msg)
{
	ENetPacket * packet = enet_packet_create(msg->GetBuffer(), msg->GetLength(), ENET_PACKET_FLAG_RELIABLE);

	enet_host_broadcast(peer->host, 1, packet);
//	enet_host_flush(peer->host);
	if (packet->referenceCount == 0)
		enet_packet_destroy(packet);
}

bool NetworkSystemLocal::IsWaitingForClients()
{
	ENetEvent event;
	
	while (enet_host_service(server, &event, 1000) > 0)
	{
		
		switch (event.type)
		{
			case ENET_EVENT_TYPE_CONNECT:
				initprintf("Client Connection Event Received\n");
				peer = event.peer;
				numConnectedClients++;
				break;
		}
	}

	return numConnectedClients < maxClients - 1;
}

int NetworkSystemLocal::GetNumConnectedPlayers()
{
	return numConnectedClients;
}